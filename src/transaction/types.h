#pragma once

#include <stddef.h>   // size_t
#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

#define ADDRESS_LEN 34  // base58 encoded address size
#define UINT160_LEN 20
#define ECPOINT_LEN 33

/**
 * Maximum signer_t count in a transaction.
 * Actual network value is 16, we limit it because we run out of SRAM
 * The individual signers must be unique as compared by the account field.
 */
#define MAX_TX_SIGNERS 2
/**
 * The minimum number of signers. First signer is always the sender of the tx
 */
#define MIN_TX_SIGNERS 1
/**
 * Limits the maximum 'allowed_groups' of a signer_t
 * Actual network value is 16, we limit it because we run out of SRAM
 */
#define MAX_SIGNER_ALLOWED_GROUPS 2

/**
 * Limits the maximum 'allowed_contracts' of a signer_t
 *
 */
#define MAX_SIGNER_ALLOWED_CONTRACTS 16

/**
 * The NEO network actually limits the attributes to (16 - signers count).
 * However, there currently only exist 2 attribute types, both can only be attached once
 * thus we limit the size to 2.
 *
 * The 16 magic is also reduced to 8 (see @MAX_TX_SIGNERS) due to SRAM limitation being reached.
 */
#define MAX_ATTRIBUTES 2

/**
 * Transaction parsing codes
 */
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
    SIGNER_ACCOUNT_PARSING_ERROR = -12,  // not enough data to get account
    SIGNER_ACCOUNT_DUPLICATE_ERROR = -13,
    SIGNER_SCOPE_PARSING_ERROR = -14,
    SIGNER_SCOPE_VALUE_ERROR_GLOBAL_FLAG = -15,  // scope GLOBAL is not allowed to have other flags
    SIGNER_ALLOWED_CONTRACTS_LENGTH_PARSING_ERROR = -16,
    SIGNER_ALLOWED_CONTRACTS_LENGTH_VALUE_ERROR = -17,
    SIGNER_ALLOWED_CONTRACT_PARSING_ERROR = -18,
    SIGNER_ALLOWED_GROUPS_LENGTH_PARSING_ERROR = -19,
    SIGNER_ALLOWED_GROUPS_LENGTH_VALUE_ERROR = -20,
    SIGNER_ALLOWED_GROUPS_PARSING_ERROR = -21,
    ATTRIBUTES_LENGTH_PARSING_ERROR = -22,
    ATTRIBUTES_LENGTH_VALUE_ERROR = -23,  // exceeding count limits
    ATTRIBUTES_UNSUPPORTED_TYPE = -24,
    ATTRIBUTES_DUPLICATE_TYPE = -25,
    SCRIPT_LENGTH_PARSING_ERROR = -26,
    SCRIPT_LENGTH_VALUE_ERROR = -27  // requesting more data than available
} parser_status_e;

typedef enum {
    NONE = 0,
    CALLED_BY_ENTRY = 0x1,
    CUSTOM_CONTRACTS = 0x10,
    CUSTOM_GROUPS = 0x20,
    GLOBAL = 0x80
} witness_scope_e;

typedef struct {
    uint8_t *account;  // UInt160, 20 bytes
    witness_scope_e scope;
    uint8_t *allowed_contracts[MAX_SIGNER_ALLOWED_CONTRACTS];  // array of UInt160s
    uint8_t allowed_contracts_size;
    uint8_t *allowed_groups[MAX_SIGNER_ALLOWED_GROUPS];  // array of ECPoints in compressed format, 33 bytes
    uint8_t allowed_groups_size;
} signer_t;

typedef enum {
    HIGH_PRIORITY = 0x1,
    ORACLE_RESPONSE = 0x11  // do not support signing this
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
    uint8_t signers_size;  // the actual signers count after parsing
    attribute_t attributes[MAX_ATTRIBUTES];
    uint8_t attributes_size;  // the actual attributes count after parsing
    uint8_t *script;          // VM opcodes
    uint16_t script_size;
    bool is_system_asset_transfer;  // indicates if the instructions in `script` match a standard GAS or NEO transfer
    bool is_neo;                    // indicates if 'transfer' is called on the NEO contract. False means GAS contract
    int64_t amount;                 // transfer amount
    uint8_t dst_address[ADDRESS_LEN];
    bool is_vote_script;
    bool is_remove_vote;
    uint8_t vote_to[ECPOINT_LEN];
} transaction_t;
