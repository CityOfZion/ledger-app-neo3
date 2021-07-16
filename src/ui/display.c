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

#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"  // snprintf
#pragma GCC diagnostic ignored "-Wformat-extra-args"         // snprintf

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "display.h"
#include "constants.h"
#include "../globals.h"
#include "../io.h"
#include "../sw.h"
#include "../address.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../common/bip32.h"
#include "../common/format.h"

static action_validate_cb g_validate_callback;
//static char g_bip32_path[60];
static char g_address[43];
static char g_system_fee[30];
static char g_network_fee[30];
static char g_network[11]; // Target network the tx in tended for
                           // ("MainNet", "TestNet" or uint32 network number)
static char g_valid_until_block[11]; // uint32 (=max 10 chars) + \0

//// Step with icon and text
//UX_STEP_NOCB(ux_display_confirm_addr_step, pn, {&C_icon_eye, "Confirm Address"});
//// Step with title/text for BIP32 path
//UX_STEP_NOCB(ux_display_path_step,
//             bnnn_paging,
//             {
//                 .title = "Path",
//                 .text = g_bip32_path,
//             });
//// Step with title/text for address
//UX_STEP_NOCB(ux_display_address_step,
//             bnnn_paging,
//             {
//                 .title = "Address",
//                 .text = g_address,
//             });
// Step with approve button
UX_STEP_CB(ux_display_approve_step,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });
// Step with reject button
UX_STEP_CB(ux_display_reject_step,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

//// FLOW to display address and BIP32 path:
//// #1 screen: eye icon + "Confirm Address"
//// #2 screen: display BIP32 Path
//// #3 screen: display address
//// #4 screen: approve button
//// #5 screen: reject button
//UX_FLOW(ux_display_pubkey_flow,
//        &ux_display_confirm_addr_step,
//        &ux_display_path_step,
//        &ux_display_address_step,
//        &ux_display_approve_step,
//        &ux_display_reject_step);

int ui_display_address() {
//    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
//        G_context.state = STATE_NONE;
//        return io_send_sw(SW_BAD_STATE);
//    }
//
//    memset(g_bip32_path, 0, sizeof(g_bip32_path));
//    if (!bip32_path_format(G_context.bip44_path,
//                           BIP44_PATH_LEN,
//                           g_bip32_path,
//                           sizeof(g_bip32_path))) {
//        return io_send_sw(SW_DISPLAY_BIP32_PATH_FAIL);
//    }
//
//    memset(g_address, 0, sizeof(g_address));
//    uint8_t address[ADDRESS_LEN] = {0};
//    if (!address_from_pubkey(G_context.raw_public_key, address, sizeof(address))) {
//        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
//    }
//    snprintf(g_address, sizeof(g_address), "0x%.*H", sizeof(address), address);
//
//    g_validate_callback = &ui_action_validate_pubkey;
//
//    ux_flow_init(0, ux_display_pubkey_flow, NULL);

    return 0;
}

// Step with icon and text
UX_STEP_NOCB(ux_display_review_step,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 "Transaction",
             });

UX_STEP_NOCB(ux_display_systemfee_step,
            bnnn_paging,
            {
                .title = "System fee",
                .text = g_system_fee,
            });

UX_STEP_NOCB(ux_display_network_step,
            bnnn_paging,
            {
                .title = "Target network",
                .text = g_network,
            });

UX_STEP_NOCB(ux_display_networkfee_step,
            bnnn_paging,
            {
                .title = "Network fee",
                .text = g_network_fee,
            });

UX_STEP_NOCB(ux_display_validuntilblock_step,
            bnnn_paging,
            {
                .title = "Valid until height",
                .text = g_valid_until_block,
            });

// FLOW to display transaction information:
// #1 screen : eye icon + "Review Transaction"
// #2 screen : display target network
// #3 screen : display system fee
// #4 screen : display network fee
// #5 screen : display max block height validity
// #6 screen : approve button
// #7 screen : reject button
UX_FLOW(ux_display_transaction_flow,
        &ux_display_review_step,
        &ux_display_network_step,
        &ux_display_systemfee_step,
        &ux_display_networkfee_step,
        &ux_display_validuntilblock_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

int ui_display_transaction() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // We'll try to give more user friendly names for known networks
    if (G_context.network_magic == NETWORK_MAINNET) {
        snprintf(g_network, sizeof(g_network), "%s", "MainNet");
    } else if (G_context.network_magic == NETWORK_TESTNET) {
        snprintf(g_network, sizeof(g_network), "%s", "TestNet");
    } else {
        snprintf(g_network, sizeof(g_network), "%d", G_context.network_magic);
    }
    PRINTF("Target network: %s\n", g_network);

    // System fee is a value multiplied by 100_000_000 to create 8 decimals stored in an int.
    // It is not allowed to be negative so we can safely cast it to uint64_t
    memset(g_system_fee, 0, sizeof(g_system_fee));
    char system_fee[30] = {0};
    if (!format_fpu64(system_fee, sizeof(system_fee), (uint64_t)G_context.tx_info.transaction.system_fee, 8)) {
        return io_send_sw(SW_DISPLAY_SYSTEM_FEE_FAIL);
    }
    snprintf(g_system_fee, sizeof(g_system_fee), "GAS %.*s", sizeof(system_fee), system_fee);
    PRINTF("System fee: %s GAS\n", system_fee);

    // Network fee is stored in a similar fashion as system fee above
    memset(g_network_fee, 0, sizeof(g_network_fee));
    char network_fee[30] = {0};
    if (!format_fpu64(network_fee, sizeof(network_fee), (uint64_t)G_context.tx_info.transaction.network_fee, 8)) {
        return io_send_sw(SW_DISPLAY_NETWORK_FEE_FAIL);
    }
    snprintf(g_network_fee, sizeof(g_network_fee), "GAS %.*s", sizeof(network_fee), network_fee);
    PRINTF("Network fee: %s GAS\n", network_fee);

    snprintf(g_valid_until_block, sizeof(g_valid_until_block), "%d", G_context.tx_info.transaction.valid_until_block);
    PRINTF("Valid until: %s\n", g_valid_until_block);

    g_validate_callback = &ui_action_validate_transaction;
    ux_flow_init(0, ux_display_transaction_flow, NULL);

    return 0;
}
