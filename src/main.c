#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

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

void main(void)
{
	printk("Start main thread\n");

	ir_thread_init();

	while (1) {
		ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);
		ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);
		ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);

		k_sleep(K_SECONDS(2U));
	}
}
