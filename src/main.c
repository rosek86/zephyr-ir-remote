#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "screen_thread.h"
#include "projector_thread.h"

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
    // projector_power_on();
    // projector_power_off();

    ret = screen_down();
    if (ret != 0) {
      printk("Error: screen_down() failed with %d\n", ret);
    }

    ret = screen_up();
    if (ret != 0) {
      printk("Error: screen_up() failed with %d\n", ret);
    }

    k_sleep(K_SECONDS(2U));
  }
}
