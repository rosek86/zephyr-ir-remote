#ifndef IR_REMOTE_H__
#define IR_REMOTE_H__

#include <stdint.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>

typedef struct {
  const struct pwm_dt_spec *pwm;
  uint32_t burst_freq;
  uint32_t payload_size;
  k_timeout_t head_mark_time;
  k_timeout_t head_space_time;
  k_timeout_t one_mark_time;
  k_timeout_t one_space_time;
  k_timeout_t zero_mark_time;
  k_timeout_t zero_space_time;
} ir_remote_t;

int ir_remote_init(ir_remote_t init);
int ir_remote_send(uint32_t payload);
bool ir_remote_busy(void);
int ir_remote_last_error(void);

#endif // IR_REMOTE_H__
