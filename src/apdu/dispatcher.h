#pragma once

#include "../types.h"

/**
 * Parameter 2 for last APDU to receive.
 */
#define P2_LAST 0x00
/**
 * Parameter 2 for more APDU to receive.
 */
#define P2_MORE 0x80
/**
 * Parameter 1 for first APDU number.
 */
#define P1_START 0x00

/**
 * Parameter 1 for maximum APDU number.
 * First apdu must always be the BIP44 path (P1 chunk 0)
 * Second apdu must always be the network magic, (P1 chunk 1)
 * The maximum APDU length is 255 bytes. Subtracting the 5 bytes header leaves 250 bytes per APDU of actual data.
 * With MAX_TRANSACTION_LEN set to 1024 we should at most need 5 APDU's to transmit the transaction part (P1 chunk 2..6)
 */
#define P1_MAX 0x06

/**
 * Dispatch APDU command received to the right handler.
 *
 * @param[in] cmd
 *   Structured APDU command (CLA, INS, P1, P2, Lc, Command data).
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int apdu_dispatcher(const command_t *cmd);
