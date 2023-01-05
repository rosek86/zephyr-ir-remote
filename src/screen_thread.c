#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include "screen_thread.h"

const struct screen_gpios {
  struct gpio_dt_spec down;
  struct gpio_dt_spec up;
} screen_gpios = {
  .down = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), screen_down_gpios),
  .up   = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), screen_up_gpios),
};

int screen_init(void) {
  int ret;

  // Configure screen GPIOs
  ret = gpio_pin_configure_dt(&screen_gpios.down, GPIO_OUTPUT_ACTIVE);
  if (ret != 0) { return ret; }

  ret = gpio_pin_configure_dt(&screen_gpios.up, GPIO_OUTPUT_ACTIVE);
  if (ret != 0) { return ret; }

  return 0;
}

int screen_down(void) {
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

int screen_up(void) {
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
