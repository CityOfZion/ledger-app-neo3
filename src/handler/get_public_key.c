/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *   (c) 2021 COZ Inc.
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

#include "get_public_key.h"
#include "../globals.h"
#include "../types.h"
#include "../io.h"
#include "../sw.h"
#include "../crypto.h"
#include "../common/buffer.h"
#include "../common/bip44.h"
#include "../ui/display.h"

int handler_get_public_key(buffer_t *cdata) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.state = STATE_NONE;

    uint16_t status;
    if (!buffer_read_and_validate_bip44(cdata, G_context.bip44_path, &status)) return io_send_sw(status);

    cx_ecfp_private_key_t private_key = {0};
    cx_ecfp_public_key_t public_key = {0};

    // Derive private key according to BIP44 path
    crypto_derive_private_key(&private_key, G_context.bip44_path, BIP44_PATH_LEN);
    // Generate corresponding public key
    crypto_init_public_key(&private_key, &public_key, G_context.raw_public_key);
    // Clear private key
    explicit_bzero(&private_key, sizeof(private_key));

    return io_send_response(&(const buffer_t){.ptr = public_key.W, .size = 65, .offset = 0}, SW_OK);
}
