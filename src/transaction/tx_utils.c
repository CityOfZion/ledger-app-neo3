#include "tx_utils.h"
#include "../ui/utils.h"

#include <string.h>

typedef union {
    uint8_t u8;
    int8_t s8;
    uint16_t u16;
    int16_t s16;
    int32_t s32;
    int64_t s64;
} data;
data s;

void try_parse_transfer_script(buffer_t *script, transaction_t *tx) {
    if (!buffer_read_u8(script, &s.u8)) return;

    // first byte should be 0xb (OpCode.PUSHNULL), indicating no data for the Nep17.transfer() 'data' argument
    if (s.u8 != 0xB) return;

    if (!buffer_read_u8(script, &s.u8)) return;

    // OpCode.PUSH0 - OpCode.PUSH16
    if (s.u8 >= 0x10 && s.u8 <= 0x20) {
        tx->amount = (int64_t) s.u8 - 0x10;
    } else if (s.u8 == 0x00) {  // OpCode.PUSHINT8
        if (!buffer_read_s8(script, &s.s8)) return;
        tx->amount = (int64_t) s.u8;
    } else if (s.u8 == 0x01) {  // OpCode.PUSHINT16
        if (!buffer_read_s16(script, &s.s16, LE)) return;
        tx->amount = (int64_t) s.s16;
    } else if (s.u8 == 0x2) {  // OpCode.PUSHINT32
        if (!buffer_read_s32(script, &s.s32, LE)) return;
        tx->amount = (int64_t) s.s32;
    } else if (s.u8 == 0x3) {  // OpCode.PUSHINT64
        if (!buffer_read_s64(script, &s.s64, LE)) return;
        tx->amount = s.s64;
    } else {  // we do not support INT128 and INT256 values on Ledger
        return;
    }

    // check for destination script hash
    if (!buffer_read_u16(script, &s.u16, BE)) return;
    if (s.u16 != 0x0C14) return;  // PUSHDATA1 , 20 bytes length for destination script hash

    uint8_t *script_hash;
    script_hash = (uint8_t *) (script->ptr + script->offset);
    if (!buffer_seek_cur(script, UINT160_LEN)) return;

    // parse and store destination address
    script_hash_to_address((char *) tx->dst_address, sizeof(tx->dst_address), script_hash);

    // check for source script hash
    if (!buffer_read_u16(script, &s.u16, BE)) return;
    if (s.u16 != 0x0C14) return;  // PUSHDATA1 , 20 bytes length for destination script hash
    if (!buffer_seek_cur(script, UINT160_LEN)) return;

    // clang-format off
    uint8_t sequence[] = {
        0x14,  // OpCode.PUSH4
        0xC0,  // OpCode.PACK - we pack the 4 arguments to the transfer() method
        0x1F,  // OpCode.PUSH15 - CallFlags
        0x0C, 0x08,  // OpCode.PUSHDATA1, length 8 - contract method name
        0x74, 0x72, 0x61, 0x6e, 0x73, 0x66, 0x65, 0x72,  // 'transfer'
        0x0C, 0x14,  // OpCode.PUSHDATA1, length 20 - contract script hash
    };

    // we expect the above fixed sequence next
    if (!buffer_can_read(script, sizeof(sequence))) return;
    if (memcmp(script->ptr + script->offset, sequence, sizeof(sequence))) return;
    buffer_seek_cur(script, sizeof(sequence));

    // read contract script hash
    // script_hash = (uint8_t *) (script->ptr + script->offset);
    uint8_t neo_script_hash[] = {0xf5, 0x63, 0xea, 0x40, 0xbc, 0x28, 0x3d, 0x4d, 0x0e, 0x05,
                                 0xc4, 0x8e, 0xa3, 0x05, 0xb3, 0xf2, 0xa0, 0x73, 0x40, 0xef};
    uint8_t gas_script_hash[] = {0xcf, 0x76, 0xe2, 0x8b, 0xd0, 0x06, 0x2c, 0x4a, 0x47, 0x8e,
                                 0xe3, 0x55, 0x61, 0x01, 0x13, 0x19, 0xf3, 0xcf, 0xa4, 0xd2};

    if (!buffer_can_read(script, UINT160_LEN)) return;
    if (!memcmp(script->ptr + script->offset, neo_script_hash, UINT160_LEN)) {
        tx->is_neo = true;
    } else if (memcmp(script->ptr + script->offset, gas_script_hash, UINT160_LEN)) {
        // neither NEO or GAS, abort
        return;
    }
    buffer_seek_cur(script, UINT160_LEN);

    // finally make sure we end with a contract syscall

    // clang-format off
    uint8_t sequence2[] = {
        0x41,  // OpCode.SYSCALL
        0x62, 0x7d, 0x5b, 0x52  // id 'System.Contract.Call'
    };

    if (!buffer_can_read(script, sizeof(sequence2))) return;
    if (memcmp(script->ptr + script->offset, sequence2, sizeof(sequence2)) != 0) return;
    buffer_seek_cur(script, sizeof(sequence2));

    // make sure there is no extra code after the transfer script
    if (script->offset != tx->script_size) return;

    // everything looks like a standard contract transfer for NEO/GAS and we were able to parse the amount + dst address
    tx->is_system_asset_transfer = true;
}