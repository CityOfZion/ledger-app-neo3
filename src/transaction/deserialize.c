/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include "deserialize.h"
#include "types.h"
#include "../common/buffer.h"
#include "stdlib.h"

parser_status_e transaction_deserialize(buffer_t *buf, transaction_t *tx) {
    if (buf->size > MAX_TX_LEN) {
        return INVALID_LENGTH_ERROR;
    }

    // This can actually never fail because 'buf' would contain the tx data send in the 3rd apdu.
    // If the 3rd apdu has no data, it will fail in the dispatcher.
    // Leaving it just incase code might change in the future. Better safe than sorrow
    if (!buffer_read_u8(buf, &tx->version)) {
        return VERSION_PARSING_ERROR;
    }

    if (tx->version > 0) {
        return VERSION_VALUE_ERROR;
    }

    if (!buffer_read_u32(buf, &tx->nonce, LE)) {
        return NONCE_PARSING_ERROR;
    }

    if (!buffer_read_s64(buf, &tx->system_fee, LE)) {
        return SYSTEM_FEE_PARSING_ERROR;
    }
    if (tx->system_fee < 0) {
        return SYSTEM_FEE_VALUE_ERROR;
    }

    if (!buffer_read_s64(buf, &tx->network_fee, LE)) {
        return NETWORK_FEE_PARSING_ERROR;
    }

    if (tx->network_fee < 0) {
        return NETWORK_FEE_VALUE_ERROR;
    }

    if (!buffer_read_u32(buf, &tx->valid_until_block, LE)) {
        return VALID_UNTIL_BLOCK_PARSING_ERROR;
    }

    // Parse (Co)Signers
    uint64_t signer_length;
    if (!buffer_read_varint(buf, &signer_length)) {
        return SIGNER_LENGTH_PARSING_ERROR;
    }
    if (signer_length < MIN_TX_SIGNERS || signer_length > MAX_TX_SIGNERS) {
        return SIGNER_LENGTH_VALUE_ERROR;
    }

    tx->signers_size = (uint8_t) signer_length;
    tx->signers[0].account = (uint8_t *) (buf->ptr + buf->offset);
    if (!buffer_seek_cur(buf, UINT160_LEN)) {
        return SIGNER_ACCOUNT_PARSING_ERROR;
    }

    uint8_t value;
    if (!buffer_read_u8(buf, &value)) {
        return SIGNER_SCOPE_PARSING_ERROR;
    }
    tx->signers[0].scope = (witness_scope_e) value;

    // Scope GLOBAL is not allowed to have other flags
    if ((((witness_scope_e) value & GLOBAL) == GLOBAL) && ((witness_scope_e) value != GLOBAL)) {
        return SIGNER_SCOPE_VALUE_ERROR_GLOBAL_FLAG;
    }

    if (((witness_scope_e) value & CUSTOM_CONTRACTS) == CUSTOM_CONTRACTS) {
        return SIGNER_SCOPE_CONTRACTS_NOT_ALLOWED_ERROR;
    }

    if (((witness_scope_e) value & CUSTOM_GROUPS) == CUSTOM_GROUPS) {
        return SIGNER_SCOPE_GROUPS_NOT_ALLOWED_ERROR;
    }

    // Parse transaction attributes
    uint64_t attributes_length;
    if (!buffer_read_varint(buf, &attributes_length)) {
        return ATTRIBUTES_LENGTH_PARSING_ERROR;
    }

    if (attributes_length > MAX_ATTRIBUTES) {
        return ATTRIBUTES_LENGTH_VALUE_ERROR;
    }
    tx->attributes_size = (uint8_t) attributes_length;

    if (tx->attributes_size > 0) {
        for (int i = 0; i < tx->attributes_size; i++) {
            uint8_t attribute_type;
            if (!buffer_read_u8(buf, &attribute_type) || attribute_type != HIGH_PRIORITY) {
                return ATTRIBUTES_UNSUPPORTED_TYPE;
            }
            // check for duplicates
            for (int j = 0; j < tx->attributes_size; j++) {
                if (tx->attributes[j].type == attribute_type) {
                    return ATTRIBUTES_DUPLICATE_TYPE;
                }
            }
            tx->attributes[i] = (attribute_t){.type = attribute_type};
        }
    }

    // Parse out script
    uint64_t script_length;
    if (!buffer_read_varint(buf, &script_length)) {
        return SCRIPT_LENGTH_PARSING_ERROR;
    }

    tx->script = (uint8_t *) (buf->ptr + buf->offset);
    if (script_length > 0xFFFF || script_length == 0 || !buffer_seek_cur(buf, script_length)) {
        return SCRIPT_LENGTH_VALUE_ERROR;
    }
    tx->script_size = (uint16_t) script_length;

    return (buf->offset == buf->size) ? PARSING_OK : INVALID_LENGTH_ERROR;
}
