#pragma once

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include "buffer.h"

/**
 * @brief Parse BIP44 path from buffer and perform basic validations
 *
 * Validations include: ensuring COIN_TYPE is NEO, account is hardened, prevent ridiculous large address index
 *
 * @param in
 * @param bip44path_out array where the BIP44 path numbers will be stored
 * @param status_out a status word indicating the failure reason
 * @return true if a BIP44 path is successfully parsed and passes all validations
 * @return false if failed to parse a BIP44 path or any validation fails.
 */

bool buffer_read_and_validate_bip44(buffer_t *in, uint32_t *bip44path_out, uint16_t *status_out);