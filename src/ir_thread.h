#ifndef IR_THREAD_H__
#define IR_THREAD_H__

#include <stdint.h>

#include "ir_command.h"

/**
 * @brief Initialize IR thread
 *
 * @return int error code
 */
int ir_thread_init(void);

/**
 * @brief Push command to IR thread queue
 *
 * @param cmd command to push
 * @return int error code
 */
int ir_thread_push(ir_command_t cmd);

#endif /* IR_THREAD_H__ */
