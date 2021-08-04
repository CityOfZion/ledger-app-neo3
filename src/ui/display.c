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
#include "action/validate.h"
#include "../transaction/types.h"
#include "../common/format.h"

static action_validate_cb g_validate_callback;
static char g_system_fee[30];
static char g_network_fee[30];
static char g_network[11];            // Target network the tx in tended for
                                      // ("MainNet", "TestNet" or uint32 network number for private nets)
static char g_valid_until_block[11];  // uint32 (=max 10 chars) + \0
static char g_account[41];            // UInt160 + \0
static char g_scope[28];              // Longest combination is: "By Entry, Contracts, Groups" (27) + \0

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

UX_STEP_NOCB(ux_display_sender_step, nn, {"Review", "Sender"});

UX_STEP_NOCB(ux_display_senderaccount_step,
             bnnn_paging,
             {
                 .title = "Account",
                 .text = g_account,
             });

UX_STEP_NOCB(ux_display_senderscope_step,
             bnnn_paging,
             {
                 .title = "Scope",
                 .text = g_scope,
             });

// FLOW to display transaction information:
// #1 screen : eye icon + "Review Transaction"
// #2 screen : display target network
// #3 screen : display system fee
// #4 screen : display network fee
// #5 screen : display max block height validity
// #n screen : display properties of attached signers
// #n+1 screen : approve button
// #n+2 screen : reject button
UX_FLOW(ux_display_transaction_flow,
        &ux_display_review_step,
        &ux_display_network_step,
        &ux_display_systemfee_step,
        &ux_display_networkfee_step,
        &ux_display_validuntilblock_step,
        &ux_display_sender_step,
        &ux_display_senderaccount_step,
        &ux_display_senderscope_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

void parse_scope_name(witness_scope_e scope) {
    size_t len = 0;
    if (scope == NONE) {
        snprintf(g_scope, sizeof(g_scope), "%s", "None");
    }

    if (scope == GLOBAL) {
        snprintf(g_scope, sizeof(g_scope), "%s", "Global");
    }

    if (scope & CALLED_BY_ENTRY) {
        len += snprintf(&g_scope[len], sizeof(g_scope), "%s", "By Entry,");
    };

    if (scope & CUSTOM_CONTRACTS) {
        len += snprintf(&g_scope[len], sizeof(g_scope), "%s", "Contracts,");
    };

    if (scope & CUSTOM_GROUPS) {
        len += snprintf(&g_scope[len], sizeof(g_scope), "%s", "Groups,");
    };
    g_scope[len - 1] = '\0';  // take of the comma
}

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
    if (!format_fpu64(system_fee, sizeof(system_fee), (uint64_t) G_context.tx_info.transaction.system_fee, 8)) {
        return io_send_sw(SW_DISPLAY_SYSTEM_FEE_FAIL);
    }
    snprintf(g_system_fee, sizeof(g_system_fee), "GAS %.*s", sizeof(system_fee), system_fee);
    PRINTF("System fee: %s GAS\n", system_fee);

    // Network fee is stored in a similar fashion as system fee above
    memset(g_network_fee, 0, sizeof(g_network_fee));
    char network_fee[30] = {0};
    if (!format_fpu64(network_fee, sizeof(network_fee), (uint64_t) G_context.tx_info.transaction.network_fee, 8)) {
        return io_send_sw(SW_DISPLAY_NETWORK_FEE_FAIL);
    }
    snprintf(g_network_fee, sizeof(g_network_fee), "GAS %.*s", sizeof(network_fee), network_fee);
    PRINTF("Network fee: %s GAS\n", network_fee);

    snprintf(g_valid_until_block, sizeof(g_valid_until_block), "%d", G_context.tx_info.transaction.valid_until_block);
    PRINTF("Valid until: %s\n", g_valid_until_block);

    signer_t sender = G_context.tx_info.transaction.signers[0];

    memset(g_account, 0, sizeof(g_account));
    snprintf(g_account, sizeof(g_account), "%.*H", 20, sender.account);
    PRINTF("Account: %s\n", g_account);

    memset(g_scope, 0, sizeof(g_scope));
    parse_scope_name(sender.scope);
    PRINTF("Scope: %s\n", g_scope);

    g_validate_callback = &ui_action_validate_transaction;
    ux_flow_init(0, ux_display_transaction_flow, NULL);

    return 0;
}