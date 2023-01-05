#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>

#include "ir_thread.h"
#include "ir_command.h"

typedef enum {
  BENQ_KEY_POWER_OFF,
  BENQ_KEY_POWER_ON,
} benq_key_t;

static ir_command_t benq_keys[] = {
  IR_COMMAND_BUILD(0x3000, 0x4E), // Power off
  IR_COMMAND_BUILD(0x3000, 0x4F), // Power on
};

const struct screen_gpios {
  struct gpio_dt_spec down;
  struct gpio_dt_spec up;
} screen_gpios = {
  .down = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), screen_down_gpios),
  .up   = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), screen_up_gpios),
};

static int screen_init(void);
static int screen_down(void);
static int screen_up(void);

static int projector_init(void);
static int projector_power_on(void);
static int projector_power_off(void);

void main(void)
{
  int ret;

  printk("Start main thread\n");

  ret = projector_init();
  if (ret != 0) {
    printk("Error: projector_init() failed with %d\n", ret);
  }

  ret = screen_init();
  if (ret != 0) {
    printk("Error: screen_init() failed with %d\n", ret);
  }

  while (1) {
    projector_power_on();
    projector_power_off();

    screen_down();
    screen_up();

    k_sleep(K_SECONDS(2U));
  }
}

static int screen_init(void) {
  int ret;

  // Configure screen GPIOs
  ret = gpio_pin_configure_dt(&screen_gpios.down, GPIO_OUTPUT_ACTIVE);
  if (ret != 0) { return ret; }

  ret = gpio_pin_configure_dt(&screen_gpios.up, GPIO_OUTPUT_ACTIVE);
  if (ret != 0) { return ret; }

  return 0;
}

static int screen_down(void) {
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

static int screen_up(void) {
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

static int projector_init(void) {
  return ir_thread_init();
}

static int projector_power_on(void) {
  int ret;

  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_ON]);
  if (ret != 0) { return ret; }

  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_ON]);
  if (ret != 0) { return ret; }

  ret = ir_thread_push(benq_keys[BENQ_KEY_POWER_ON]);
  if (ret != 0) { return ret; }

  return 0;
}

static int projector_power_off(void) {
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
