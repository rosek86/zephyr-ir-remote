#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "ir_thread.h"

void main(void)
{
	printk("Start main thread\n");

	ir_thread_init();

	while (1) {
		ir_thread_push(BENQ_KEY_POWER_OFF);
		ir_thread_push(BENQ_KEY_POWER_OFF);
		ir_thread_push(BENQ_KEY_POWER_OFF);

		k_sleep(K_SECONDS(2U));
	}
}
