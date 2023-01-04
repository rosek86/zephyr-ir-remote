#ifndef IR_THREAD_H__
#define IR_THREAD_H__

#include <stdint.h>

#include "ir_command.h"

int ir_thread_init(void);
int ir_thread_push(ir_command_t cmd);

#endif /* IR_THREAD_H__ */
