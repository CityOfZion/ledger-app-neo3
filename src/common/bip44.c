#include "buffer.h"
#include "../sw.h" // status words
#include "../constants.h" // BIP44 constants

bool buffer_read_and_validate_bip44(buffer_t *in,
                                    uint32_t *bip44path_out,
                                    uint16_t *status_out) {

    if (in->size < BIP44_BYTE_LENGTH) {
        *status_out = SW_WRONG_DATA_LENGTH;
        return false;
    }

    // temp var
    uint32_t bip_level;

    // check purpose
    buffer_read_u32(in, &bip_level, BE);
    if (bip_level != BIP44_PURPOSE) {
        *status_out = SW_BIP44_BAD_PURPOSE;
        return false;
    }
    bip44path_out[0] = bip_level;

    // check coin type
    buffer_read_u32(in, &bip_level, BE);
    if (bip_level != BIP44_COIN_TYPE_NEO) {
        *status_out = SW_BIP44_BAD_COIN_TYPE;
        return false;
    }
    bip44path_out[1] = bip_level;

    // check account is within a sane range
    buffer_read_u32(in, &bip_level, BE);
    if (bip_level < 0x80000000) {
        *status_out = SW_BIP44_ACCOUNT_NOT_HARDENED;
        return false;
    }

    if (bip_level > 0x80000010) {
        *status_out = SW_BIP44_BAD_ACCOUNT;
        return false;
    }
    bip44path_out[2] = bip_level;

    // make sure Change is either external or internal
    buffer_read_u32(in, &bip_level, BE);
    if (bip_level != 0x0 && bip_level != 0x1) {
        *status_out = SW_BIP44_BAD_CHANGE;
        return false;
    }
    bip44path_out[3] = bip_level;

    // check address is within a sane range
    buffer_read_u32(in, &bip_level, BE);
    if (bip_level >= 5000) {
        *status_out = SW_BIP44_BAD_ADDRESS;
        return false;
    }
    bip44path_out[4] = bip_level;

    return true;
}