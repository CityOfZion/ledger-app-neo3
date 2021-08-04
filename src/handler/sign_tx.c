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

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"

#include "sign_tx.h"
#include "../sw.h"
#include "../globals.h"
#include "../crypto.h"
#include "../ui/display.h"
#include "../common/buffer.h"
#include "../common/bip44.h"
#include "../transaction/types.h"
#include "../transaction/deserialize.h"

int handler_sign_tx(buffer_t *cdata, uint8_t chunk, bool more) {
    if (chunk == 0) {  // First APDU, parse BIP44 path
        explicit_bzero(&G_context, sizeof(G_context));
        G_context.req_type = CONFIRM_TRANSACTION;
        G_context.state = STATE_NONE;

        uint16_t status;
        if (!buffer_read_and_validate_bip44(cdata, G_context.bip44_path, &status)) {
            return io_send_sw(status);
        }

        G_context.state = STATE_BIP44_OK;
        return io_send_sw(SW_OK);
    } else if (chunk == 1) {
        if (G_context.req_type != CONFIRM_TRANSACTION && G_context.state != STATE_BIP44_OK) {
            return io_send_sw(SW_BAD_STATE);
        }

        if (!buffer_read_u32(cdata, &G_context.network_magic, LE)) {
            return io_send_sw(SW_MAGIC_PARSING_FAIL);
        }
        G_context.state = STATE_MAGIC_OK;
        return io_send_sw(SW_OK);
    } else {  // Receive transaction
        if (G_context.req_type != CONFIRM_TRANSACTION && G_context.state != STATE_MAGIC_OK) {
            return io_send_sw(SW_BAD_STATE);
        }

        if (more) {  // APDU with another transaction part
            if (G_context.tx_info.raw_tx_len + cdata->size > MAX_TRANSACTION_LEN ||
                !buffer_move(cdata, G_context.tx_info.raw_tx + G_context.tx_info.raw_tx_len, cdata->size)) {
                return io_send_sw(SW_WRONG_TX_LENGTH);
            }

            G_context.tx_info.raw_tx_len += cdata->size;

            return io_send_sw(SW_OK);
        } else {  // Last APDU, let's parse and sign
            if (G_context.tx_info.raw_tx_len + cdata->size > MAX_TRANSACTION_LEN ||
                !buffer_move(cdata, G_context.tx_info.raw_tx + G_context.tx_info.raw_tx_len, cdata->size)) {
                return io_send_sw(SW_WRONG_TX_LENGTH);
            }

            G_context.tx_info.raw_tx_len += cdata->size;

            buffer_t buf = {.ptr = G_context.tx_info.raw_tx, .size = G_context.tx_info.raw_tx_len, .offset = 0};

            parser_status_e status = transaction_deserialize(&buf, &G_context.tx_info.transaction);
            PRINTF("Parsing status: %d.\n", status);
            if (status != PARSING_OK) {
                char status_char[1] = {(uint8_t) status};
                return io_send_response(&(const buffer_t){.ptr = (unsigned char *) status_char, .size = 1, .offset = 0},
                                        SW_TX_PARSING_FAIL);
            }

            G_context.state = STATE_PARSED;

            /**
             * Here we hash the signed part of the transaction. This is _not_ the final hash used as input for ecdsa
             * (see crypto_sign_tx()) The final hash is: sha256(network magic + sha256(signed part of tx data)), but we
             * don't hash this until we've approved among others the network magic
             */

            cx_sha256_t tx_hash;
            cx_sha256_init(&tx_hash);
            cx_hash((cx_hash_t *) &tx_hash,
                    CX_LAST /*mode*/,
                    G_context.tx_info.raw_tx /* data in */,
                    G_context.tx_info.raw_tx_len /* data in len */,
                    G_context.tx_info.hash /* hash out*/,
                    sizeof(G_context.tx_info.hash) /* hash out len */);

            PRINTF("Hash: %.*H\n", sizeof(G_context.tx_info.hash), G_context.tx_info.hash);

            return ui_display_transaction();
        }
    }

    return 0;
}
