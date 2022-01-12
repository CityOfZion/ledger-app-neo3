#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <cmocka.h>

#include "common/read.h"

static void test_read(void **state) {
    (void) state;

    int64_t i64;
    uint64_t u64;
    const uint8_t u32_buf1[4] = {0x11, 0x22, 0x33, 0x44};
    int32_t res = read_s32_be(u32_buf1, 0);
    assert_int_equal(res, 0x11223344);

    const uint8_t u32_buf2[4] = {0xf1, 0x22, 0x33, 0x44};
    int32_t res2 = read_s32_be(u32_buf2, 0);
    assert_int_equal(res2, -249416892);
    uint32_t u32 = read_u32_be(u32_buf2, 0);
    assert_int_equal(u32, 0xf1223344);

    const uint8_t u64_buf1[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    i64 = read_s64_be(u64_buf1, 0);
    assert_int_equal(i64, 0x1122334455667788);
    i64 = read_s64_le(u64_buf1, 0);
    assert_int_equal(i64, -8613303245920329199);

    u64 = read_u64_be(u64_buf1, 0);
    assert_int_equal(u64, 0x1122334455667788);
    u64 = read_u64_le(u64_buf1, 0);
    assert_int_equal(u64, 0x8877665544332211);
}

int main() {
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_read)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
