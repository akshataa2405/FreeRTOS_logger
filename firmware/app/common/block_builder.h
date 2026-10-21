#ifndef BLOCK_BUILDER_H
#define BLOCK_BUILDER_H

#include "record_format.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    LogBlock_t current_block;
    uint32_t   session_id;
    uint16_t   records_buffered;
    bool       block_ready;
} BlockBuilder_t;

/**
 * @brief Initializes a BlockBuilder context with a unique session ID.
 */
void block_builder_init(BlockBuilder_t *builder, uint32_t session_id);

/**
 * @brief Appends a single record into the current block.
 *
 * @param builder Pointer to block builder
 * @param record Pointer to incoming record
 * @return true if the block is now full and sealed (ready to write), false otherwise.
 */
bool block_builder_add_record(BlockBuilder_t *builder, const DataRecord_t *record);

/**
 * @brief Manually seals the block (e.g., at shutdown or file boundary), even if < 17 records.
 *
 * @param builder Pointer to block builder
 * @return true if a non-empty block was sealed, false if no records were buffered.
 */
bool block_builder_flush(BlockBuilder_t *builder);

/**
 * @brief Clears the ready flag to begin filling the next block.
 */
void block_builder_reset_ready(BlockBuilder_t *builder);

#ifdef __cplusplus
}
#endif

#endif /* BLOCK_BUILDER_H */
