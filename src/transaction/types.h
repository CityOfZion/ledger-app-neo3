#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#define MAX_TX_LEN   510
#define ADDRESS_LEN  20
#define ACCOUNT_LEN  20
#define MAX_MEMO_LEN 465  // 510 - ADDRESS_LEN - 2*SIZE(U64) - SIZE(MAX_VARINT)

/**
 * Maximum signer count, the individual signers must by unique as compared by the account field
 */
#define MAX_TX_SIGNERS 16

/**
 * The NEO network actually limits the attributes to (16 - signers count).
 * However, there currently only exist 2 attribute types, both can only be attached once
 * (allow_multiple = False), so we can limit the size to 2.
 */
#define MAX_ATTRIBUTES 2

typedef enum {
    PARSING_OK = 1,
    INVALID_LENGTH_ERROR = -1,
    VERSION_PARSING_ERROR = -2,
    VERSION_VALUE_ERROR = -3,
    NONCE_PARSING_ERROR = -4,
    SYSTEM_FEE_PARSING_ERROR = -5,
    SYSTEM_FEE_VALUE_ERROR = -6,
    NETWORK_FEE_PARSING_ERROR = -7,
    NETWORK_FEE_VALUE_ERROR = -8,
    VALID_UNTIL_BLOCK_PARSING_ERROR = -9,
    SIGNER_LENGTH_PARSING_ERROR = -10,
    SIGNER_LENGTH_VALUE_ERROR = -11,
    SIGNER_ACCOUNT_PARSING_ERROR = -12, // not enough data to get account
    SIGNER_SCOPE_PARSING_ERROR = -13,
    SIGNER_SCOPE_VALUE_ERROR_GLOBAL_FLAG = -14, // scope GLOBAL is not allowed to have other flags
    SIGNER_ALLOWED_CONTRACTS_LENGTH_PARSING_ERROR = -14,
    SIGNER_ALLOWED_CONTRACTS_LENGTH_VALUE_ERROR = -15,
    SIGNER_ALLOWED_GROUPS_LENGTH_PARSING_ERROR = -16,
    SIGNER_ALLOWED_GROUPS_LENGTH_VALUE_ERROR = -17,
    ATTRIBUTES_LENGTH_PARSING_ERROR = -18,
    ATTRIBUTES_LENGTH_VALUE_ERROR = -19, // exceeding count limits
    ATTRIBUTES_UNSUPPORTED_TYPE = -20,
    ATTRIBUTES_DUPLICATE_TYPE = -21,
    SCRIPT_LENGTH_PARSING_ERROR = -22,
    SCRIPT_LENGTH_VALUE_ERROR = -23 // requesting more data than available
} parser_status_e;

typedef enum {
    NONE = 0,
    CALLED_BY_ENTRY = 0x1,
    CUSTOM_CONTRACTS = 0x10,
    CUSTOM_GROUPS = 0x20,
    GLOBAL = 0x80
} witness_scope_e;


typedef struct {
    uint8_t *account; // UInt160, 20 bytes
    witness_scope_e scope;
    uint8_t *allowed_contracts; // array of UInt160
    uint8_t allowed_contracts_size;
    uint8_t *allowed_groups; // array of ECPoint in compressed format, 33 bytes
    uint8_t allowed_groups_size;
} signer_t;

typedef enum {
    HIGH_PRIORITY = 0x1,
    ORACLE_RESPONSE = 0x11 // do not support signing this
} tx_attribute_type_e;

typedef struct {
    tx_attribute_type_e type;
    // might expand this later if new attributes are introduced to have data beyond a type
} attribute_t;



typedef struct {
    uint8_t version;
    uint32_t nonce;
    int64_t system_fee;
    int64_t network_fee;
    uint32_t valid_until_block;
    signer_t signers[MAX_TX_SIGNERS];
    uint8_t signers_size; // the actual count
    attribute_t attributes[MAX_ATTRIBUTES];
    uint8_t attributes_size; // the actual count
    uint8_t *script;
    uint16_t script_size;
} transaction_t;
