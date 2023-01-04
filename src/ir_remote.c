#include "ir_remote.h"

#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>

#define ROUND_DIV(dividend, divisor) (((dividend) + ((divisor) - 1)) / (divisor))

static ir_remote_t conf;
static uint32_t frame_stop_wait_one;
static uint32_t frame_stop_wait_zero;

static struct k_timer frame_start_timer;
static struct k_timer frame_timer;

static struct {
  bool busy;
  int last_error;
  uint32_t start_state;
  uint32_t payload;
  uint32_t payload_iter; // bits
  uint32_t stop_wait;
} state;

static void frame_start_handler(struct k_timer *timer_id);
static void frame_handler(struct k_timer *timer_id);

static uint32_t burst_period = 0;
static inline int burst_on(void) { return pwm_set_dt(conf.pwm, burst_period, burst_period / 2U); }
static inline int burst_off(void) { return pwm_set_dt(conf.pwm, burst_period, 0); }

static int frame_send_next_bit(void);
static int frame_send_end(bool *done);
static void frame_send_error(int err);

int ir_remote_init(ir_remote_t init) {
  conf = init;

  if (!device_is_ready(conf.pwm->dev)) {
    return -ENODEV;
  }

  if (conf.one_mark_time.ticks != conf.zero_mark_time.ticks) {
    printk("Error: one_mark_time != zero_mark_time\n");
    return -EINVAL;
  }

  burst_period = ROUND_DIV((uint32_t)1e9, conf.burst_freq);
  frame_stop_wait_one = ROUND_DIV(conf.one_space_time.ticks, conf.one_mark_time.ticks);
  frame_stop_wait_zero = ROUND_DIV(conf.zero_space_time.ticks, conf.zero_mark_time.ticks);

  // initialize timers
  k_timer_init(&frame_start_timer, frame_start_handler, NULL);
  k_timer_init(&frame_timer, frame_handler, NULL);

  memset(&state, 0, sizeof(state));

  return 0;
}

int ir_remote_send(uint32_t payload) {
  int ret = 0;

  if (state.busy) {
    return -EBUSY;
  }

  state.busy          = true;
  state.start_state   = 0;
  state.payload       = payload;
  state.payload_iter  = 0; // bits
  state.stop_wait     = 0;
  state.last_error    = 0;

  if ((ret = burst_on()) != 0) {
    goto exception;
  }

  k_timer_start(&frame_start_timer, conf.head_mark_time, conf.head_space_time);

  return 0;

exception:
  state.busy = false;
  state.last_error = ret;
  return ret;
}

bool ir_remote_busy(void) {
  return state.busy;
}

int ir_remote_last_error_get(void) {
  return state.last_error;
}

void ir_remote_last_error_clear(void) {
  state.last_error = 0;
}

static void frame_start_handler(struct k_timer *timer_id) {
  int ret = 0;

  if (state.start_state == 0) {
    if ((ret = burst_off()) != 0) {
      goto exception;
    }

    state.start_state = 1;
  } else {
    k_timer_stop(&frame_start_timer);

    if ((ret = frame_send_next_bit()) != 0) {
      goto exception;
    }

    k_timer_start(&frame_timer, conf.one_mark_time, conf.one_mark_time);
  }

  return;

exception:
  frame_send_error(ret);
}

static void frame_handler(struct k_timer *timer_id) {
  int ret;

  if (state.stop_wait > 0) {
    if ((ret = burst_off()) != 0) {
      goto exception;
    }

    --state.stop_wait;
    return;
  }

  if (state.payload_iter >= conf.payload_size) {
    bool done = false;

    if ((ret = frame_send_end(&done)) != 0) {
      goto exception;
    }

    if (done) {
      k_timer_stop(&frame_timer);
      state.busy = false;
    }

    return;
  }

  if ((ret = frame_send_next_bit()) != 0) {
    goto exception;
  }

  return;

exception:
  frame_send_error(ret);
}

static int frame_send_next_bit(void) {
  int ret;

  if ((ret = burst_on()) != 0) {
    return ret;
  }

  uint32_t bit = (state.payload >> state.payload_iter) & 1;

  state.stop_wait = bit ? frame_stop_wait_one : frame_stop_wait_zero;
  state.payload_iter++;

  return 0;
}

static int frame_send_end(bool *done) {
  int ret;

  if (state.payload_iter == conf.payload_size) {
    // start end of frame burst
    if ((ret = burst_on()) != 0) {
      return ret;
    }
    state.payload_iter++;
  } else {
    // end of frame
    if ((ret = burst_off()) != 0) {
      return ret;
    }

    *done = true;
  }

  return 0;
}

static void frame_send_error(int err) {
  k_timer_stop(&frame_start_timer);
  k_timer_stop(&frame_timer);
  burst_off();
  state.busy = false;
  state.last_error = err;
}
