#pragma once

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include "buffer.h"

bool buffer_read_and_validate_bip44(buffer_t *in, uint32_t *bip44path_out, uint16_t *status_out);