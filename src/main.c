#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "ir_thread.h"
#include "ir_command.h"

typedef enum {
  BENQ_KEY_POWER_OFF,
  BENQ_KEY_POWER_ON,
} benq_key_t;

static ir_command_t benq_keys[] = { (ir_command_t) {}, (ir_command_t) {} };
static void prepare_keys(void);

void main(void)
{
	printk("Start main thread\n");

	prepare_keys();
	ir_thread_init();

	while (1) {
		ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);
		ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);
		ir_thread_push(benq_keys[BENQ_KEY_POWER_OFF]);

		k_sleep(K_SECONDS(2U));
	}
}

static void prepare_keys(void) {
  const uint16_t address = 0x3000;

  // power off
  benq_keys[BENQ_KEY_POWER_OFF] = (ir_command_t) {
    .address = address,
    .command = 0x4E,
  };

  // power on
  benq_keys[BENQ_KEY_POWER_ON] = (ir_command_t) {
    .address = address,
    .command = 0x4F,
  };
}
