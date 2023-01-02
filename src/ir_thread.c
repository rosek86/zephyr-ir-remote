#include <zephyr/kernel.h>
#include <zephyr/kernel/thread.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#include "ir_remote.h"
#include "ir_protocol.h"
#include "ir_thread.h"

#define IR_REMOTE_THREAD_STACK_SIZE 1024
#define IR_REMOTE_THREAD_PRIORITY 5

static const struct pwm_dt_spec pwm_ir = PWM_DT_SPEC_GET(DT_ALIAS(pwm_ir));

K_MSGQ_DEFINE(my_msgq, sizeof(benq_key_t), 10, 4);

static uint32_t benq_keys[] = { 0, 0 };
static void prepare_keys(void);

static inline int benq_send_power_on(void);
static inline int benq_send_power_off(void);

static void ir_thread(void *a, void *b, void *c);

K_THREAD_DEFINE(ir_remote_id,
  IR_REMOTE_THREAD_STACK_SIZE, ir_thread, NULL, NULL, NULL,
  IR_REMOTE_THREAD_PRIORITY, 0, 0);

int ir_thread_push(benq_key_t key) {
  return k_msgq_put(&my_msgq, &key, K_NO_WAIT);
}

static void ir_thread(void *a, void *b, void *c) {
  int ret;

  if (!device_is_ready(pwm_ir.dev)) {
    printk("Error: PWM device %s is not ready\n",
           pwm_ir.dev->name);
    return;
  }

  prepare_keys();

  printk("Start\n");

  ret = ir_remote_init((ir_remote_t) {
    .pwm              = &pwm_ir,
    .burst_freq       = 38000,
    .payload_size     = 32,
    .head_mark_time   = K_USEC(9000),
    .head_space_time  = K_USEC(4500),
    .one_mark_time    = K_NSEC(562500),
    .one_space_time   = K_NSEC(1687500),
    .zero_mark_time   = K_NSEC(562500),
    .zero_space_time  = K_NSEC(562500),
  });
  if (ret) {
    printk("Init error: %d\n", ret);
  }

  int attempt = 0;
  while (1) {
    // pop from queue

    benq_key_t payload;
    ret = k_msgq_get(&my_msgq, &payload, K_FOREVER);
    if (ret) {
      printk("Error: %d\n", ret);
      continue;
    }

    printk("Attempt %d\n", ++attempt);

    switch (payload) {
      case BENQ_KEY_POWER_OFF:
        ret = benq_send_power_off();
        break;
      case BENQ_KEY_POWER_ON:
        ret = benq_send_power_on();
        break;
      default:
        printk("Unknown payload: %d\n", payload);
    }

    if (ret) {
      if (ir_remote_busy()) {
        printk("Error: busy\n");
      }
      if (ir_remote_last_error()) {
        printk("Last error: %d\n", ir_remote_last_error());
      }
    }

    k_msleep(100U);
  }
}

static void prepare_keys(void) {
  const uint16_t address = 0x3000;

  uint32_t ir_payload;

  ir_protocol_t ir_protocol = {
    .type = IR_PROTOCOL_TYPE_NEC,
    .info = { .nec = { .address_size = 16 } },
  };

  // power off
  ir_protocol_build(&ir_protocol, (ir_command_t) {
    .address = address,
    .command = 0x4E,
  }, &ir_payload);
  benq_keys[BENQ_KEY_POWER_OFF] = ir_payload;

  // power on
  ir_protocol_build(&ir_protocol, (ir_command_t) {
    .address = address,
    .command = 0x4F,
  }, &ir_payload);
  benq_keys[BENQ_KEY_POWER_ON] = ir_payload;
}

static inline int benq_send_power_on(void) {
  return ir_remote_send(benq_keys[BENQ_KEY_POWER_ON]);
}

static inline int benq_send_power_off(void) {
  return ir_remote_send(benq_keys[BENQ_KEY_POWER_OFF]);
}
