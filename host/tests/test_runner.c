#include "unity.h"

/* Test function prototypes */
void test_crc32_known_vector(void);
void test_crc32_empty_string(void);
void test_crc32_incremental(void);

void test_struct_sizes(void);
void test_record_packing_roundtrip(void);

void test_block_builder_full_block(void);
void test_block_builder_flush_partial(void);

void test_shell_parser_basic(void);
void test_shell_parser_empty_and_unknown(void);

int main(void) {
    unity_begin("STM32 FreeRTOS Data Logger Host Tests");

    /* CRC32 suite */
    RUN_TEST(test_crc32_known_vector);
    RUN_TEST(test_crc32_empty_string);
    RUN_TEST(test_crc32_incremental);

    /* Record Format suite */
    RUN_TEST(test_struct_sizes);
    RUN_TEST(test_record_packing_roundtrip);

    /* Block Builder suite */
    RUN_TEST(test_block_builder_full_block);
    RUN_TEST(test_block_builder_flush_partial);

    /* Shell Parser suite */
    RUN_TEST(test_shell_parser_basic);
    RUN_TEST(test_shell_parser_empty_and_unknown);

    return unity_end();
}
