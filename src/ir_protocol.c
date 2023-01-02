#include <stdint.h>
#include <errno.h>

#include "ir_protocol.h"

static int build_payload_nec(ir_protocol_t const * const protocol, ir_command_t cmd, uint32_t *payload);

int ir_protocol_build(ir_protocol_t const * const conf, ir_command_t cmd, uint32_t *payload) {
  if (conf->type == IR_PROTOCOL_TYPE_NEC) {
    return build_payload_nec(conf, cmd, payload);
  }

  return -EINVAL; // unknown protocol
}

static int build_payload_nec(ir_protocol_t const * const conf, ir_command_t cmd, uint32_t *payload) {
  uint32_t p = 0;

  if (conf->info.nec.address_size == 16) {
    p |= (( cmd.address & 0xFFFF) << 0);
  } else if (conf->info.nec.address_size == 8) {
    p |= (( cmd.address & 0xFF) << 0);
    p |= ((~cmd.address & 0xFF) << 8);
  } else {
    return -EINVAL; // unknown address size
  }

  p |= (( cmd.command & 0xFF) << 16);
  p |= ((~cmd.command & 0xFF) << 24);

  *payload = p;

  return 0;
}
