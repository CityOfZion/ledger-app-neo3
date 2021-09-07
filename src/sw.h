#pragma once

/**
 * Status word for success.
 */
#define SW_OK 0x9000
/**
 * Status word for denied by user.
 */
#define SW_DENY 0x6985
/**
 * Status word for incorrect P1 or P2.
 */
#define SW_WRONG_P1P2 0x6A86
/**
 * Status word for either wrong Lc or length of APDU command less than 5.
 */
#define SW_WRONG_DATA_LENGTH 0x6A87
/**
 * Status word for unknown command with this INS.
 */
#define SW_INS_NOT_SUPPORTED 0x6D00
/**
 * Status word for instruction class is different than CLA.
 */
#define SW_CLA_NOT_SUPPORTED 0x6E00
/**
 * Status word for wrong response length (buffer too small or too big).
 */
#define SW_WRONG_RESPONSE_LENGTH 0xB000
/**
 * Status word for wrong transaction length.
 */
#define SW_WRONG_TX_LENGTH 0xB001
/**
 * Status word for fail of transaction parsing.
 */
#define SW_TX_PARSING_FAIL 0xB002
/**
 * Status word for rejecting tx signing by user
 */
#define SW_TX_USER_CONFIRMATION_FAIL 0xB003
/**
 * Status word for bad signing state.
 */
#define SW_BAD_STATE 0xB004
/**
 * Status word for signing failure.
 */
#define SW_SIGN_FAIL 0xB005
/**
 * Status word for invalid BIP44 purpose field
 */
#define SW_BIP44_BAD_PURPOSE 0xB100
/**
 * Status word for BIP44 coin type not matching NEO
 */
#define SW_BIP44_BAD_COIN_TYPE 0xB101
/**
 * Status word for BIP44 account not hardened
 */
#define SW_BIP44_ACCOUNT_NOT_HARDENED 0xB102
/**
 * Status word for BIP44 account value > 10
 */
#define SW_BIP44_BAD_ACCOUNT 0xB103
/**
 * Status word for BIP44 change field not Internal or External
 */
#define SW_BIP44_BAD_CHANGE 0xB104
/**
 * Status word for BIP44 address > 5000
 */
#define SW_BIP44_BAD_ADDRESS 0xB105
/**
 * Status word for failing to parse network magic
 */
#define SW_MAGIC_PARSING_FAIL 0xB106
/**
 * Status word for failing to parse the system fee into a format that
 * can be displayed on the device
 */
#define SW_DISPLAY_SYSTEM_FEE_FAIL 0xb107
/**
 * Status word for failing to parse the network fee into a format that
 * can be displayed on the device
 */
#define SW_DISPLAY_NETWORK_FEE_FAIL 0xb108

/**
 * Status word for failing to parse the total fees (network + system) 
 * into a format that can be displayed on the device
 */
#define SW_DISPLAY_TOTAL_FEE_FAIL 0xb109

/**
 * Status word for failing to convert public key to NEO address
 */
#define SW_CONVERT_TO_ADDRESS_FAIL 0xb200