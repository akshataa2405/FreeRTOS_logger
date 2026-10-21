#include "unity.h"
#include "crc32.h"

void test_crc32_known_vector(void) {
    const char *test_str = "123456789";
    uint32_t crc = crc32_compute((const uint8_t *)test_str, 9);
    TEST_ASSERT_EQUAL_UINT32(0xCBF43926U, crc);
}

void test_crc32_empty_string(void) {
    uint32_t crc = crc32_compute((const uint8_t *)"", 0);
    TEST_ASSERT_EQUAL_UINT32(0x00000000U, crc);
}

void test_crc32_incremental(void) {
    const char *part1 = "1234";
    const char *part2 = "56789";

    uint32_t full_crc = crc32_compute((const uint8_t *)"123456789", 9);

    uint32_t step = crc32_update(0xFFFFFFFFU, (const uint8_t *)part1, 4);
    step = crc32_update(step, (const uint8_t *)part2, 5);
    uint32_t incremental_crc = step ^ 0xFFFFFFFFU;

    TEST_ASSERT_EQUAL_UINT32(full_crc, incremental_crc);
}
