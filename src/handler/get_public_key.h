#pragma once

#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <stdint.h>   // uint*_t

#include "../types.h"
#include "../common/buffer.h"

/**
 * Handler for GET_PUBLIC_KEY command. If the BIP44 path is parsed successfully
 * derive the public key and send APDU response.
 *
 * @see G_context.bip44_path and G_context.raw_public_key
 *
 * @param[in,out] cdata
 *   Command data with BIP44 path.
 * @param[in]     display
 *   Whether to display address on screen or not.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_get_public_key(buffer_t *cdata);
