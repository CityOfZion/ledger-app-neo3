#pragma once

#include <stdint.h>  // uint*_t
#include <stdbool.h>

#include "os.h"
#include "cx.h"

bool address_from_pubkey(uint8_t public_key[static 64], char* out, size_t out_len);

void script_hash_to_address(char* out, size_t out_len, const unsigned char* script_hash);