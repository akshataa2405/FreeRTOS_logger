#include "unity.h"
#include "block_builder.h"
#include "crc32.h"
#include <string.h>

void test_block_builder_full_block(void) {
    BlockBuilder_t builder;
    block_builder_init(&builder, 0x10203040U);

    DataRecord_t rec;
    memset(&rec, 0, sizeof(rec));

    /* Add 16 records - should not be ready yet */
    for (uint32_t i = 0; i < 16; i++) {
        rec.seq = 100 + i;
        bool ready = block_builder_add_record(&builder, &rec);
        TEST_ASSERT_FALSE(ready);
    }

    /* 17th record completes the block */
    rec.seq = 116;
    bool ready = block_builder_add_record(&builder, &rec);
    TEST_ASSERT_TRUE(ready);
    TEST_ASSERT_TRUE(builder.block_ready);

    LogBlock_t *blk = &builder.current_block;
    TEST_ASSERT_EQUAL_UINT32(BLOCK_MAGIC, blk->magic);
    TEST_ASSERT_EQUAL_INT(BLOCK_VERSION, blk->version);
    TEST_ASSERT_EQUAL_INT(BLOCK_FLAG_COMPLETE, blk->flags);
    TEST_ASSERT_EQUAL_INT(17, blk->record_count);
    TEST_ASSERT_EQUAL_UINT32(100, blk->first_seq);
    TEST_ASSERT_EQUAL_UINT32(0x10203040U, blk->session_id);

    /* Verify CRC */
    size_t crc_len = offsetof(LogBlock_t, crc32);
    uint32_t calculated_crc = crc32_compute((const uint8_t *)blk, crc_len);
    TEST_ASSERT_EQUAL_UINT32(calculated_crc, blk->crc32);
}

void test_block_builder_flush_partial(void) {
    BlockBuilder_t builder;
    block_builder_init(&builder, 0x99887766U);

    DataRecord_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.seq = 500;
    block_builder_add_record(&builder, &rec);
    rec.seq = 501;
    block_builder_add_record(&builder, &rec);

    bool flushed = block_builder_flush(&builder);
    TEST_ASSERT_TRUE(flushed);
    TEST_ASSERT_TRUE(builder.block_ready);

    LogBlock_t *blk = &builder.current_block;
    TEST_ASSERT_EQUAL_INT(BLOCK_FLAG_INCOMPLETE, blk->flags);
    TEST_ASSERT_EQUAL_INT(2, blk->record_count);
    TEST_ASSERT_EQUAL_UINT32(500, blk->first_seq);

    /* Ensure padding and unused slots are zeroed out and CRC matches */
    size_t crc_len = offsetof(LogBlock_t, crc32);
    uint32_t calculated_crc = crc32_compute((const uint8_t *)blk, crc_len);
    TEST_ASSERT_EQUAL_UINT32(calculated_crc, blk->crc32);
}
