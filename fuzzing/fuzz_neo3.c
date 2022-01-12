#include "transaction/deserialize.h"

#include <stddef.h>
#include <stdint.h>

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    buffer_t buf = {.offset = 0, .ptr = Data, .size = Size};
    transaction_t tx;

    transaction_deserialize(&buf, &tx);
    return 0;
}
