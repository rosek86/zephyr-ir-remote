#ifndef IR_COMMAND_H__
#define IR_COMMAND_H__

#include <stdint.h>

typedef struct {
  uint16_t address;
  uint8_t command;
} ir_command_t;

#define IR_COMMAND_BUILD(_address, _command)  \
  (ir_command_t) {                            \
    .address = _address,                      \
    .command = _command,                      \
  }

#endif // IR_COMMAND_H__
