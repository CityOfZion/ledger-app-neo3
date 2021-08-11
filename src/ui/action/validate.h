#pragma once

#include <stdbool.h>  // bool

/**
 * Action for public key validation and export.
 *
 * @param[in] approved
 *   User approved or rejected.
 *
 */
void ui_action_validate_pubkey(bool approved);

/**
 * Action for transaction information validation.
 *
 * @param[in] approved
 *   User approved or rejected.
 *
 */
void ui_action_validate_transaction(bool approved);
