// parts of code gracefully copied from app-neo (https://github.com/LedgerHQ/app-neo/blob/master/src/neo.c)

#include "utils.h"
#include "types.h"
#include "../common/base58.h"

#include <string.h>

/** the length of a SHA256 hash */
#define SHA256_HASH_LEN 32

/** the current version of the address field */
#define ADDRESS_VERSION 0x35

/** length of tx.output.script_hash */
#define SCRIPT_HASH_LEN 20

/** length of the checksum used to convert a script_hash into an Address. */
#define SCRIPT_HASH_CHECKSUM_LEN 4

/** length of a tx.output Address, after Base58 encoding. */
#define ADDRESS_BASE58_LEN 34

/** length of a Address before encoding, which is the length of <address_version>+<script_hash>+<checksum> */
#define ADDRESS_LEN_PRE (1 + SCRIPT_HASH_LEN + SCRIPT_HASH_CHECKSUM_LEN)

/**
 * Length of a standard single account verification script
 * 1 byte OpCode.PUSHDATA1 + 1 byte size + 33 bytes public key + 1 byte OpCode.SYSCALL + 4 bytes syscall id
 */
#define VERIFICATION_SCRIPT_LENGTH 40

bool create_signature_redeem_script(const uint8_t* public_key, uint8_t* out, size_t out_len) {
    if (out_len != VERIFICATION_SCRIPT_LENGTH) {
        return false;
    }

    // we first have to compress the public key
    uint8_t compressed_key[33];
    compressed_key[0] = ((public_key[63] & 1) ? 0x03 : 0x02);
    memcpy(&compressed_key[1], public_key, 32);

    out[0] = 0xc;   // OpCode.PUSHDATA1;
    out[1] = 0x21;  // data size, 33 bytes for compressed public key
    memcpy(&out[2], compressed_key, sizeof(compressed_key));

    out[35] = 0x41;                  // OpCode.SYSCALL
    uint32_t checksig = 0x27B3E756;  // Syscall "System.Crypto.CheckSig"
    memcpy(&out[36], &checksig, 4);

    return true;
}

void public_key_hash160(const unsigned char* in, unsigned short inlen, unsigned char* out) {
    union {
        cx_sha256_t shasha;
        cx_ripemd160_t riprip;
    } u;
    unsigned char buffer[32];

    cx_sha256_init(&u.shasha);
    cx_hash(&u.shasha.header, CX_LAST, in, inlen, buffer, 32);
    cx_ripemd160_init(&u.riprip);
    cx_hash(&u.riprip.header, CX_LAST, buffer, 32, out, 20);
}

void script_hash_to_address(char* out, size_t out_len, const unsigned char* script_hash) {
    static cx_sha256_t data_hash;
    unsigned char data_hash_1[SHA256_HASH_LEN];
    unsigned char data_hash_2[SHA256_HASH_LEN];
    unsigned char address[ADDRESS_LEN_PRE];

    address[0] = ADDRESS_VERSION;
    memcpy(&address[1], script_hash, UINT160_LEN);

    // do a sha256 hash of the address twice.
    cx_sha256_init(&data_hash);
    cx_hash(&data_hash.header, CX_LAST, address, UINT160_LEN + 1, data_hash_1, 32);
    cx_sha256_init(&data_hash);
    cx_hash(&data_hash.header, CX_LAST, data_hash_1, SHA256_HASH_LEN, data_hash_2, 32);

    // the first 4 bytes of the final hash is the checksum for base58check encode
    // append to the end of the data
    memcpy(&address[1 + UINT160_LEN], data_hash_2, SCRIPT_HASH_CHECKSUM_LEN);

    base58_encode(address, sizeof(address), out, out_len);
}

bool address_from_pubkey(const uint8_t public_key[static 64], char* out, size_t out_len) {
    // we need to go through 3 steps
    // 1. create a verification script with the public key
    // 2. create a script hash of the verification script (using sha256 + ripemd160)
    // 3. base58check encode the NEO account version + script hash to get the address
    unsigned char verification_script[VERIFICATION_SCRIPT_LENGTH];
    unsigned char script_hash[UINT160_LEN];

    // step 1
    if (!create_signature_redeem_script(public_key, verification_script, sizeof(verification_script))) {
        return false;
    }
    // step 2
    public_key_hash160(verification_script, sizeof(verification_script), script_hash);
    // step 3
    script_hash_to_address(out, out_len, script_hash);
    return true;
}