#ifndef IR_PROTOCOL_H__
#define IR_PROTOCOL_H__

#include <stdint.h>
#include "ir_command.h"

typedef enum {
  IR_PROTOCOL_TYPE_NEC,
} ir_protocol_type_t;

typedef struct {
  ir_protocol_type_t type;

  union {
    struct {
      int address_size;
    } nec;
  } info;
} ir_protocol_t;

int ir_protocol_build(ir_protocol_t const * const conf, ir_command_t cmd, uint32_t *payload);

#endif // IR_PROTOCOL_H__
