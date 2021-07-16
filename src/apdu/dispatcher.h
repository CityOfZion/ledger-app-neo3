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
 * First apdu must always be the BIP44 path,
 * Second apdu must always be the network magic,
 * therefore we have 2 chunks of 510 bytes left to fit our TX in
 */
#define P1_MAX 0x03

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
