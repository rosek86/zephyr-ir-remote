#include <zephyr/kernel.h>

#include "ir_command.h"
#include "ir_thread.h"

typedef enum {
  BENQ_KEY_POWER_OFF,
  BENQ_KEY_POWER_ON,
} benq_key_t;

static ir_command_t benq_keys[] = {
  IR_COMMAND_BUILD(0x3000, 0x4E), // Power off
  IR_COMMAND_BUILD(0x3000, 0x4F), // Power on
};

int projector_init(void) {
  return ir_thread_init();
}

int projector_power_on(void) {
  int ret;

  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_ON]);
  if (ret != 0) { return ret; }

  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_ON]);
  if (ret != 0) { return ret; }

  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_ON]);
  if (ret != 0) { return ret; }

  return 0;
}

int projector_power_off(void) {
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
