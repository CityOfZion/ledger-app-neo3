#pragma once

/**
 * Instruction class of the Boilerplate application.
 */
#define CLA 0x80

/**
 * Length of APPNAME variable in the Makefile.
 */
#define APPNAME_LEN (sizeof(APPNAME) - 1)

/**
 * Maximum length of MAJOR_VERSION || MINOR_VERSION || PATCH_VERSION.
 */
#define APPVERSION_LEN 3

/**
 * Maximum length of application name.
 */
#define MAX_APPNAME_LEN 64

/**
 * Maximum transaction length (bytes).
 */
#define MAX_TRANSACTION_LEN 1024  // TODO: look into what we should say is the  max tx length

/**
 * Maximum signature length (bytes).
 */
#define MAX_DER_SIG_LEN 72

/**
 * Exponent used to convert mBOL to BOL unit (N BOL = N * 10^3 mBOL).
 */
#define EXPONENT_SMALLEST_UNIT 3

/**
 * Length of BIP44 path
 * m / purpose' / coin_type' / account' / change / address_index
 * */
#define BIP44_PATH_LEN 5

/** Length of BIP44 path, in bytes */
#define BIP44_BYTE_LENGTH (BIP44_PATH_LEN * sizeof(unsigned int))

/**
 * Coin type 888 as described in
 * https://github.com/satoshilabs/slips/blob/master/slip-0044.md
 * */
#define BIP44_COIN_TYPE_NEO 0x80000378

/** BIP44 purpose 44' */
#define BIP44_PURPOSE 0x8000002C

/** NEO Main network magic */
#define NETWORK_MAINNET 5195086

/** NEO Test network magic */
#define NETWORK_TESTNET 1951352142