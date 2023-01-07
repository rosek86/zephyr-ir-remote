#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include "screen_thread.h"
#include "utils.h"

#define SCREEN_MESSAGE_QUEUE_SIZE  10
#define SCREEN_THREAD_STACK_SIZE   1024
#define SCREEN_THREAD_PRIORITY     5

typedef enum {
  SCREEN_CMD_DOWN,
  SCREEN_CMD_UP,
} screen_cmd_t;

const struct screen_gpios {
  struct gpio_dt_spec down;
  struct gpio_dt_spec up;
} screen_gpios = {
  .down = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), screen_down_gpios),
  .up   = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), screen_up_gpios),
};

K_MSGQ_DEFINE_STATIC(msgq, sizeof(uint32_t), SCREEN_MESSAGE_QUEUE_SIZE, 4);
static K_THREAD_STACK_DEFINE(thread_stack, SCREEN_THREAD_STACK_SIZE);
static struct k_thread thread_data;
static k_tid_t thread_id;

static int init_gpio(void);
static void thread(void *a, void *b, void *c);

int screen_init(void) {
  int ret;

  ret = init_gpio();
  if (ret != 0) { return ret; }

  thread_id = k_thread_create(
    &thread_data, thread_stack,
    K_THREAD_STACK_SIZEOF(thread_stack),
    thread, NULL, NULL, NULL,
    SCREEN_THREAD_PRIORITY, 0, K_NO_WAIT
  );

  return 0;
}

static int init_gpio(void) {
  int ret;

  // Configure screen GPIOs
  ret = gpio_pin_configure_dt(&screen_gpios.down, GPIO_OUTPUT_ACTIVE);
  if (ret != 0) { return ret; }

  ret = gpio_pin_configure_dt(&screen_gpios.up, GPIO_OUTPUT_ACTIVE);
  if (ret != 0) { return ret; }

  return 0;
}

int screen_down(void) {
  uint32_t cmd = SCREEN_CMD_DOWN;
  return k_msgq_put(&msgq, &cmd, K_NO_WAIT);
}

int screen_up(void) {
  uint32_t cmd = SCREEN_CMD_UP;
  return k_msgq_put(&msgq, &cmd, K_NO_WAIT);
}

static int down(void);
static int up(void);

static void thread(void *a, void *b, void *c) {
  while (1) {
    uint32_t cmd;
    k_msgq_get(&msgq, &cmd, K_FOREVER);

    switch (cmd) {
      case SCREEN_CMD_DOWN: down(); break;
      case SCREEN_CMD_UP:   up();   break;
    }
  }
}

static int down(void) {
  int ret;

  if ((ret = gpio_pin_set_dt(&screen_gpios.down, 0)) != 0) {
    return ret;
  }

  k_sleep(K_MSEC(100));

  if ((ret = gpio_pin_set_dt(&screen_gpios.down, 1)) != 0) {
    return ret;
  }

  return 0;
}

static int up(void) {
  int ret;

  if ((ret = gpio_pin_set_dt(&screen_gpios.up, 0)) != 0) {
    return ret;
  }

  k_sleep(K_MSEC(100));

  if ((ret = gpio_pin_set_dt(&screen_gpios.up, 1)) != 0) {
    return ret;
  }

  return 0;
}
