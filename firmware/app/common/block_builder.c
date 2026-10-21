#include "block_builder.h"
#include "crc32.h"
#include <string.h>

static void seal_block(BlockBuilder_t *builder, uint8_t flag) {
    builder->current_block.magic = BLOCK_MAGIC;
    builder->current_block.version = BLOCK_VERSION;
    builder->current_block.flags = flag;
    builder->current_block.record_count = builder->records_buffered;
    builder->current_block.session_id = builder->session_id;

    if (builder->records_buffered > 0) {
        builder->current_block.first_seq = builder->current_block.records[0].seq;
    } else {
        builder->current_block.first_seq = 0;
    }

    /* Zero pad any unused records in the 17-record array */
    if (builder->records_buffered < RECORDS_PER_BLOCK) {
        size_t unused_count = RECORDS_PER_BLOCK - builder->records_buffered;
        memset(&builder->current_block.records[builder->records_buffered], 0, unused_count * sizeof(DataRecord_t));
    }

    /* Zero pad the 16 trailing padding bytes */
    memset(builder->current_block.padding, 0, sizeof(builder->current_block.padding));

    /* Compute CRC-32 over bytes 0 .. 507 */
    size_t crc_payload_len = offsetof(LogBlock_t, crc32);
    builder->current_block.crc32 = crc32_compute((const uint8_t *)&builder->current_block, crc_payload_len);

    builder->block_ready = true;
}

void block_builder_init(BlockBuilder_t *builder, uint32_t session_id) {
    if (!builder) return;
    memset(builder, 0, sizeof(BlockBuilder_t));
    builder->session_id = session_id;
    builder->records_buffered = 0;
    builder->block_ready = false;
}

bool block_builder_add_record(BlockBuilder_t *builder, const DataRecord_t *record) {
    if (!builder || !record) return false;

    /* Copy record into slot */
    memcpy(&builder->current_block.records[builder->records_buffered], record, sizeof(DataRecord_t));
    builder->records_buffered++;

    if (builder->records_buffered == RECORDS_PER_BLOCK) {
        seal_block(builder, BLOCK_FLAG_COMPLETE);
        return true;
    }

    return false;
}

bool block_builder_flush(BlockBuilder_t *builder) {
    if (!builder || builder->records_buffered == 0) return false;

    seal_block(builder, BLOCK_FLAG_INCOMPLETE);
    return true;
}

void block_builder_reset_ready(BlockBuilder_t *builder) {
    if (!builder) return;
    builder->records_buffered = 0;
    builder->block_ready = false;
    memset(&builder->current_block, 0, sizeof(LogBlock_t));
}
