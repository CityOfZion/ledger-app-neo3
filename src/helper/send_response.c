#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t
#include <string.h>  // memmove

#include "send_response.h"
#include "../constants.h"
#include "../globals.h"
#include "../sw.h"
#include "common/buffer.h"

int helper_send_response_pubkey() {
    uint8_t resp[1 + PUBKEY_LEN] = {0};
    size_t offset = 1;

    resp[0] = 0x04;
    memmove(resp + offset, G_context.raw_public_key, PUBKEY_LEN);
    offset += PUBKEY_LEN;

    return io_send_response(&(const buffer_t){.ptr = resp, .size = offset, .offset = 0}, SW_OK);
}