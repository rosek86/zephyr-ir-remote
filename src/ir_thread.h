#ifndef IR_THREAD_H__
#define IR_THREAD_H__

#include <stdint.h>

typedef enum {
  BENQ_KEY_POWER_OFF,
  BENQ_KEY_POWER_ON,
} benq_key_t;

int ir_thread_push(benq_key_t key);

#endif /* IR_THREAD_H__ */
