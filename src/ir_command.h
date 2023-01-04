#ifndef IR_COMMAND_H__
#define IR_COMMAND_H__

#include <stdint.h>

typedef struct {
  uint16_t address;
  uint8_t command;
} ir_command_t;

#endif // IR_COMMAND_H__
