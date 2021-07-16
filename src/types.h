#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "constants.h"
#include "transaction/types.h"
#include "common/bip32.h"

/**
 * Enumeration for the status of IO.
 */
typedef enum {
    READY,     /// ready for new event
    RECEIVED,  /// data received
    WAITING    /// waiting
} io_state_e;

/**
 * Enumeration with expected INS of APDU commands.
 */
typedef enum {
    GET_APP_NAME = 0x0,    /// name of the application
    GET_VERSION = 0x01,    /// version of the application
    SIGN_TX = 0x02,        /// sign transaction with BIP44 path and return signature
    GET_PUBLIC_KEY = 0x04  /// public key of corresponding BIP44 path and return uncompressed public key
} command_e;

/**
 * Structure with fields of APDU command.
 */
typedef struct {
    uint8_t cla;    /// Instruction class
    command_e ins;  /// Instruction code
    uint8_t p1;     /// Instruction parameter 1
    uint8_t p2;     /// Instruction parameter 2
    uint8_t lc;     /// Length of command data
    uint8_t *data;  /// Command data
} command_t;

/**
 * Enumeration with parsing state.
 */
typedef enum {
    STATE_NONE,     /// No state
    STATE_BIP44_OK, /// BIP44 path parsed
    STATE_MAGIC_OK, /// Network magic parsed
    STATE_PARSED,   /// Transaction data parsed
    STATE_APPROVED  /// Transaction data approved
} state_e;

/**
 * Enumeration with user request type.
 */
typedef enum {
    CONFIRM_ADDRESS,     /// Confirm address derived from public key
    CONFIRM_TRANSACTION  /// Confirm transaction information
} request_type_e;

/**
 * Structure for transaction information context.
 */
typedef struct {
    uint8_t raw_tx[MAX_TRANSACTION_LEN];  /// Raw transaction serialized
    size_t raw_tx_len;                    /// Length of raw transaction
    transaction_t transaction;            /// Structured transaction

                                          /// Transaction hash digest
                                          /// This is just the hash of the tx signed data portion
                                          /// this is not the actual hash going used for signing
    uint8_t hash[32];                     /// as that also includes the network magic
    uint8_t signature[MAX_DER_SIG_LEN];   /// Transaction signature encoded in ASN1.DER
    uint8_t signature_len;                /// Length of transaction signature
} transaction_ctx_t;

/**
 * Structure for global context.
 */
typedef struct {
    state_e state;  /// State of the context
    union {
        uint8_t raw_public_key[64];  /// x-coordinate (32), y-coodinate (32)
        transaction_ctx_t tx_info;  /// Transaction context
    };
    uint32_t network_magic;
    request_type_e req_type;              /// User request
    uint32_t bip44_path[BIP44_PATH_LEN];  /// BIP44 path
} global_ctx_t;
