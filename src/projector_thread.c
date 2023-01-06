#include <zephyr/kernel.h>
#include <zephyr/kernel/thread.h>

#include "ir_command.h"
#include "ir_thread.h"

#define PROJECTOR_MESSAGE_QUEUE_SIZE  10
#define PROJECTOR_THREAD_STACK_SIZE   1024
#define PROJECTOR_THREAD_PRIORITY     5

typedef enum {
  BENQ_KEY_POWER_OFF,
  BENQ_KEY_POWER_ON,
} benq_key_t;

static ir_command_t benq_keys[] = {
  IR_COMMAND_BUILD(0x3000, 0x4E), // Power off
  IR_COMMAND_BUILD(0x3000, 0x4F), // Power on
};

K_MSGQ_DEFINE(projector_msgq, sizeof(uint32_t), PROJECTOR_MESSAGE_QUEUE_SIZE, 4);
static K_THREAD_STACK_DEFINE(thread_stack, PROJECTOR_THREAD_STACK_SIZE);
static struct k_thread thread_data;
static k_tid_t thread_id;

static void thread(void *a, void *b, void *c);

int projector_init(void) {
  int ret;

  ret = ir_thread_init();
  if (ret != 0) { return ret; }

  thread_id = k_thread_create(
    &thread_data, thread_stack,
    K_THREAD_STACK_SIZEOF(thread_stack),
    thread, NULL, NULL, NULL,
    PROJECTOR_THREAD_PRIORITY, 0, K_NO_WAIT
  );

  return 0;
}

int projector_power_on(void) {
  uint32_t cmd = BENQ_KEY_POWER_ON;
  return k_msgq_put(&projector_msgq, &cmd, K_NO_WAIT);
}

int projector_power_off(void) {
  uint32_t cmd = BENQ_KEY_POWER_OFF;
  return k_msgq_put(&projector_msgq, &cmd, K_NO_WAIT);
}

static int power_off(void);
static int power_on(void);

static void thread(void *a, void *b, void *c) {
  while (1) {
    uint32_t cmd;
    k_msgq_get(&projector_msgq, &cmd, K_FOREVER);

    switch (cmd) {
      case BENQ_KEY_POWER_ON:
        power_on();
        break;
      case BENQ_KEY_POWER_OFF:
        power_off();
        break;
    }
  }
}

static int power_on(void) {
  int ret;

  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_ON]);
  if (ret != 0) { return ret; }

  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_ON]);
  if (ret != 0) { return ret; }

  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_ON]);
  if (ret != 0) { return ret; }

  return 0;
}

static int power_off(void) {
  int ret;

  // request to turn off the projector
  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);
  if (ret != 0) { return ret; }
  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);
  if (ret != 0) { return ret; }
  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);
  if (ret != 0) { return ret; }

  k_sleep(K_SECONDS(2U));

  // confirm power off
  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);
  if (ret != 0) { return ret; }
  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);
  if (ret != 0) { return ret; }
  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);
  if (ret != 0) { return ret; }

  return 0;
}
