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
static char g_system_fee[30];
static char g_network_fee[30];
static char g_network[11]; // Target network the tx in tended for
                           // ("MainNet", "TestNet" or uint32 network number)
static char g_valid_until_block[11]; // uint32 (=max 10 chars) + \0
static char g_title[64]; // generic step title
static char g_text[64]; // generic step text

static char g_signer_account[8+UINT160_LEN+1]; // "Account " + UInt160 len + \0
static char g_signer_scope[9+1]; // None, ByEntry, Contracts, Groups, Global + \0
static char g_signer_custom_group[11+ECPOINT_LEN+1]; // "Group (n/m)" + point len + \0
static char g_signer_custom_contract[14+UINT160_LEN+1]; // "Contract (n/m)"

struct display_ctx_t {
// An instance of our new enum
   enum e_state current_state; // screen state
   int8_t s_index; // track which signer is to be displayed
   enum e_signer_state s_next_prop; // track which property is to be displayed next
   uint8_t prop_index;
   int8_t c_index; // track which contract is to be displayed
   int8_t g_index; // track which group is to be displayed
   bool start;
   bool end; // last signer is shown

} display_ctx;

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

UX_STEP_INIT(
   ux_upper_delimiter,
   NULL,
   NULL,
   {
      // This function will be detailed later on.
      display_next_state(true);
   }
);

UX_STEP_NOCB(
    ux_display_signer,
    bn,
    {"Signer", "1 of 8"});  // make dynamic

UX_STEP_NOCB(
    ux_display_signer_account,
    bnnn_paging,
    {
        .title = "Account",
        .text = g_signer_account,
    });

UX_STEP_NOCB(
   ux_display_generic,
   bnnn_paging,
   {
      .title = g_title,
      .text = g_text,
   }
);

// Note we're using UX_STEP_INIT because this step won't display anything.
UX_STEP_INIT(
   ux_lower_delimiter,
   NULL,
   NULL,
   {
      // This function will be detailed later on.
      display_next_state(false);
   }
);

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
        &ux_upper_delimiter, // special step that won't be shown, but used for runtime displaying dynamics screens when applicable
        &ux_display_generic, // will be used to dynamically display Signers
        &ux_lower_delimiter, // special step that won't be shown, but used for runtime displaying dynamics screens when applicable
        &ux_display_approve_step,
        &ux_display_reject_step);

void reset_signer_display_state() {
    display_ctx.current_state = STATIC_SCREEN;
    display_ctx.start = true;
    display_ctx.end = false;
    display_ctx.s_next_prop = 0;
    display_ctx.s_index = 0;
    display_ctx.g_index = -1;
    display_ctx.c_index = -1;
    display_ctx.prop_index = 0;
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
    reset_signer_display_state();
    ux_flow_init(0, ux_display_transaction_flow, NULL);

    return 0;
}

const char* get_scope_name(witness_scope_e scope) 
{
   switch (scope) 
   {
      case NONE: return "None";
      case CALLED_BY_ENTRY: return "By entry";
      case CUSTOM_CONTRACTS: return "Contracts";
      case CUSTOM_GROUPS: return "Groups";
      case GLOBAL: return "Global";
      default: return "Unknown";
   }
}

// This is a special function you must call for bnnn_paging to work properly in an edgecase.
// It does some weird stuff with the `G_ux` global which is defined by the SDK.
// No need to dig deeper into the code, a simple copy paste will do.
void bnnn_paging_edgecase() {
    G_ux.flow_stack[G_ux.stack_count - 1].prev_index = G_ux.flow_stack[G_ux.stack_count - 1].index - 2;
    G_ux.flow_stack[G_ux.stack_count - 1].index--;
    ux_flow_relayout();
}

static enum e_signer_state signer_property[7] = {START, INDEX, ACCOUNT, SCOPE, CONTRACTS, GROUPS, END};


// FINISHED and WORKING!
void next_prop() {
    uint8_t *idx = &display_ctx.prop_index;
    signer_t signer = G_context.tx_info.transaction.signers[display_ctx.s_index];

    if (*idx < (uint8_t)CONTRACTS)
        (*idx)++;

    if (signer_property[*idx] == CONTRACTS) {
        // we start at -1            
        if (display_ctx.c_index + 1 < signer.allowed_contracts_size) {
            display_ctx.c_index++;
            return; // let it display the contract
        }
        (*idx)++; // advance state to GROUPS
    }
    if (signer_property[*idx] == GROUPS) {
        // we start at -1
        if (display_ctx.g_index + 1 < signer.allowed_groups_size) {
            display_ctx.g_index++;
            return; // let it display the group
        }
        (*idx)++; // advance state to END
    }

    // set flag that the next screen is no longer the first DYNAMIC screen
    // if (signer_property[*idx] == INDEX) {
        // display_ctx.start = false;
    // }

    // if we displayed all properties of the current signer    
    if (signer_property[*idx] == END) {
        // are there more signers?
        if (display_ctx.s_index + 1 == G_context.tx_info.transaction.signers_size) {
            display_ctx.end = true; // we've displayed all data for all signers
            return;
        } else {
            // yes, advance signer index and reset some properties
            display_ctx.s_index++;
            display_ctx.c_index = -1;
            display_ctx.g_index = -1;
            *idx = (uint8_t)START;
            next_prop();
        }
    }
}

bool get_next_data(bool is_forward) {
    if (is_forward) {
        next_prop();
    } else {
        reset_signer_display_state();
        return false; 
    }

    signer_t s = G_context.tx_info.transaction.signers[display_ctx.s_index];
    enum e_signer_state display = signer_property[display_ctx.prop_index];

    if (display_ctx.end) { // we reached the last signer and displayed all its data
        return false;
    }

    switch (display) {
        case START: {
            return false;
        }
        case INDEX: {
            snprintf(g_title, sizeof(g_title), "Signer");
            snprintf(g_text, sizeof(g_text), "%d of %d", display_ctx.s_index + 1, G_context.tx_info.transaction.signers_size);
            return true;
        }
        case ACCOUNT: {
            snprintf(g_title, sizeof(g_title), "Account");
            snprintf(g_text, sizeof(g_text), "%.*H", 20, s.account);
            return true;
        }
        case SCOPE: {
            snprintf(g_title, sizeof(g_title), "Scope");
            const char *name = get_scope_name(s.scope);
            snprintf(g_text, sizeof(g_text), "%s", name);
            return true;
        }
        case CONTRACTS: {
            s = G_context.tx_info.transaction.signers[display_ctx.s_index];
            snprintf(g_title, sizeof(g_title), "Contract %d of %d", display_ctx.c_index + 1, s.allowed_contracts_size);
            snprintf(g_text, sizeof(g_text), "%.*H", UINT160_LEN, s.allowed_contracts[display_ctx.c_index]);
            return true;
        } 
        case GROUPS: {
            s = G_context.tx_info.transaction.signers[display_ctx.s_index];
            snprintf(g_title, sizeof(g_title), "Group %d of %d", display_ctx.g_index + 1, s.allowed_groups_size);
            snprintf(g_text, sizeof(g_text), "%.*H", ECPOINT_LEN, s.allowed_groups[display_ctx.g_index]);
            return true;
        }
        case END: {
            return false;
        }
    }
}

// Taken from Ledger's advanced display management docs
void display_next_state(bool is_upper_delimiter) {
    if (is_upper_delimiter) { // We're called from the upper delimiter.
        if (display_ctx.current_state == STATIC_SCREEN) {
            // Fetch new data.
            bool dynamic_data = get_next_data(true);
            if (dynamic_data) {
                // We found some data to display so we now enter in dynamic mode.
                display_ctx.current_state = DYNAMIC_SCREEN;
            }

            // Move to the next step, which will display the screen.
            ux_flow_next();
        } else {
            // The previous screen was NOT a static screen, so we were already in a dynamic screen.

            // Fetch new data.
            bool dynamic_data = get_next_data(false);
            if (dynamic_data) {
                // We found some data so simply display it.
                ux_flow_next();
            } else {
                // There's no more dynamic data to display, so
                // update the current state accordingly.
                display_ctx.current_state = STATIC_SCREEN;
                
                // Display the previous screen which should be a static one.
                ux_flow_prev();
            }
        } 
    } else {
        // We're called from the lower delimiter.
        if (display_ctx.current_state == STATIC_SCREEN) {
            // Fetch new data.
            bool dynamic_data = get_next_data(false);
            if (dynamic_data) {
                // We found some data to display so enter in dynamic mode.
                display_ctx.current_state = DYNAMIC_SCREEN;
            }
                
            // Display the data.
            ux_flow_prev();
        } else {
            // We're being called from a dynamic screen, so the user was already browsing the array.

            // Fetch new data.
            bool dynamic_data = get_next_data(true);
            if (dynamic_data) {
                // We found some data, so display it.
                // Similar to `ux_flow_prev()` but updates layout to account for `bnnn_paging`'s weird behaviour.
                bnnn_paging_edgecase();
            } else {
                // We found no data so make sure we update the state accordingly.
                display_ctx.current_state = STATIC_SCREEN;
                // Display the next screen
                ux_flow_next();
            }
        }
    }
}