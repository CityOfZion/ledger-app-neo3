#pragma once

#include "os.h"

#include "../common/macros.h"

/**
 * Length of public key.
 */
#define PUBKEY_LEN 64

int helper_send_response_pubkey(void);