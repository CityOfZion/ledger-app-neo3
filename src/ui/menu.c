/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *       2021 COZ Inc.
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

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "../globals.h"
#include "menu.h"
#include "../shared_context.h"

void ui_menu_main();
void display_settings(const ux_flow_step_t* const start_step);
void switch_settings_contract_scripts(void);

UX_STEP_NOCB(ux_menu_ready_step, pn, {&C_badge_neo, "Wake up NEO.."});
UX_STEP_NOCB(ux_menu_version_step, bn, {"Version", APPVERSION});
UX_STEP_CB(ux_menu_settings_step, pb, display_settings(NULL), {&C_icon_eye, "Settings"});
UX_STEP_CB(ux_menu_about_step, pb, ui_menu_about(), {&C_icon_certificate, "About"});
UX_STEP_VALID(ux_menu_exit_step, pb, os_sched_exit(-1), {&C_icon_dashboard_x, "Quit"});

// FLOW for the main menu:
// #1 screen: ready
// #2 screen: version of the app
// #3 screen: settings
// #4 screen: about submenu
// #5 screen: quit
UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step,
        &ux_menu_version_step,
        &ux_menu_settings_step,
        &ux_menu_about_step,
        &ux_menu_exit_step,
        FLOW_LOOP);

#if defined(TARGET_NANOS)
// clang-format off
UX_STEP_CB(
    ux_settings_contract_scripts,
    bnnn_paging,
    switch_settings_contract_scripts(),
    {
        .title = "Contract data",
        .text = strings.scriptsAllowed
    });

#else
UX_STEP_CB(
    ux_settings_contract_scripts,
    bnnn,
    switch_settings_contract_scripts(),
    {
        "Contract scripts",
        "Allow contract scripts",
        "in transactions",
        strings.scriptsAllowed
    });
#endif

UX_STEP_CB(
    ux_settings_back_step,
    pb,
    ui_menu_main(),
    {
      &C_icon_back_x,
      "Back",
    });

// clang-format on
UX_FLOW(ux_settings_flow, &ux_settings_contract_scripts, &ux_settings_back_step);

void ui_menu_main() {
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_menu_main_flow, NULL);
}

void display_settings(const ux_flow_step_t* const start_step) {
    strlcpy(strings.scriptsAllowed, (N_storage.scriptsAllowed ? "Allowed" : "NOT Allowed"), 12);
    ux_flow_init(0, ux_settings_flow, start_step);
}

void switch_settings_contract_scripts() {
    uint8_t value = (N_storage.scriptsAllowed ? 0 : 1);
    nvm_write((void*) &N_storage.scriptsAllowed, (void*) &value, sizeof(uint8_t));
    display_settings(&ux_settings_contract_scripts);
}

UX_STEP_NOCB(ux_menu_info_step, bn, {"NEO N3 App", "(c) 2021 COZ Inc"});
UX_STEP_CB(ux_menu_back_step, pb, ui_menu_main(), {&C_icon_back, "Back"});

// FLOW for the about submenu:
// #1 screen: app info
// #2 screen: back button to main menu
UX_FLOW(ux_menu_about_flow, &ux_menu_info_step, &ux_menu_back_step, FLOW_LOOP);

void ui_menu_about() {
    ux_flow_init(0, ux_menu_about_flow, NULL);
}
