#include <zephyr/kernel.h>
#include <zephyr/kernel/thread.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#include "ir_command.h"
#include "ir_remote.h"
#include "ir_protocol.h"
#include "ir_thread.h"

#define IR_REMOTE_MESSAGE_QUEUE_SIZE  10
#define IR_REMOTE_THREAD_STACK_SIZE   1024
#define IR_REMOTE_THREAD_PRIORITY     5

static const struct pwm_dt_spec pwm_ir = PWM_DT_SPEC_GET(DT_ALIAS(pwm_ir));

K_MSGQ_DEFINE(my_msgq, sizeof(ir_command_t), IR_REMOTE_MESSAGE_QUEUE_SIZE, 4);
K_THREAD_STACK_DEFINE(my_stack_area, IR_REMOTE_THREAD_STACK_SIZE);

static ir_protocol_t ir_protocol;
static struct k_thread ir_thread_data;
static k_tid_t ir_thread_id;

static void ir_thread(void *a, void *b, void *c);

int ir_thread_init(void) {
  int ret;

  if (!device_is_ready(pwm_ir.dev)) {
    printk("Error: PWM device %s is not ready\n",
           pwm_ir.dev->name);
    return -ENODEV;
  }

  ir_protocol = (ir_protocol_t) {
    .type = IR_PROTOCOL_TYPE_NEC,
    .info = { .nec = { .address_size = 16 } },
  };

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

  ir_thread_id = k_thread_create(
    &ir_thread_data, my_stack_area,
    K_THREAD_STACK_SIZEOF(my_stack_area),
    ir_thread, NULL, NULL, NULL,
    IR_REMOTE_THREAD_PRIORITY, 0, K_NO_WAIT
  );

  return ret;
}

int ir_thread_push(ir_command_t cmd) {
  return k_msgq_put(&my_msgq, &cmd, K_NO_WAIT);
}

static void ir_thread(void *a, void *b, void *c) {
  int ret;
  ir_command_t command;
  uint32_t payload;

  while (1) {
    // pop from queue
    ret = k_msgq_get(&my_msgq, &command, K_FOREVER);
    if (ret) {
      printk("Cannot pop from queue, reason: %d\n", ret);
      continue;
    }

    // prepare payload
    ret = ir_protocol_build(&ir_protocol, command, &payload);
    if (ret) {
      printk("Cannot build command, reason: %d\n", ret);
      continue;
    }

    // send payload
    ret = ir_remote_send(payload);
    if (ret) {
      printk("Cannot send command, reason: %d\n", ret);
      continue;
    }

    k_msleep(100U); // 100ms delay between commands in NEC protocol

    if (ir_remote_last_error_get()) {
      printk("Cannot transfer command, reason: %d\n", ir_remote_last_error_get());
      ir_remote_last_error_clear();
    }
  }
}
