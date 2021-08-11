#pragma once

#include <stdint.h>  // uint*_t

#include "os.h"
#include "cx.h"

bool address_from_pubkey(const uint8_t public_key[static 64], uint8_t* out, size_t out_len);
