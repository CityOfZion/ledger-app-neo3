#pragma once

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

#include "../common/buffer.h"

/**
 * Handler for SIGN_TX command. If the BIP44 path is parsed successfully
 * sign the transaction and send the signature in the APDU response.
 *
 * @see G_context.bip44_path, G_context.tx_info.raw_transaction,
 * G_context.tx_info.signature.
 *
 * @param[in,out] cdata
 *   Command data with BIP44 path and raw transaction serialized.
 * @param[in]     chunk
 *   Index number of the APDU chunk.
 * @param[in]       more
 *   Whether more chunks are expected to be received or not.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_sign_tx(buffer_t *cdata, uint8_t chunk, bool more);
