#ifndef STORAGE_H
#define STORAGE_H

#include "record_format.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STORAGE_OK = 0,
    STORAGE_ERR_NOT_MOUNTED,
    STORAGE_ERR_FILE_CREATE,
    STORAGE_ERR_WRITE_FAIL,
    STORAGE_ERR_FULL,
    STORAGE_ERR_CORRUPT
} StorageStatus_t;

/**
 * @brief Mounts the storage media (FatFs SD card / SPI Flash).
 */
StorageStatus_t storage_init(void);

/**
 * @brief Opens a new sequenced log file (e.g. "log_0001.bin").
 * Pre-allocates file space to guarantee power-loss resilience.
 *
 * @param file_num_out Output parameter populated with allocated log index
 */
StorageStatus_t storage_open_new_log(uint16_t *file_num_out);

/**
 * @brief Writes a 512-byte block to storage.
 */
StorageStatus_t storage_write_block(const LogBlock_t *block);

/**
 * @brief Flushes any cached writes to physical storage media.
 */
StorageStatus_t storage_flush(void);

/**
 * @brief Closes the currently active log file.
 */
StorageStatus_t storage_close(void);

/**
 * @brief Scans the newest file on boot to count all valid blocks prior to power loss.
 *
 * @param recovered_records_out Output count of valid recovered records
 */
StorageStatus_t storage_scan_last_valid(uint32_t *recovered_records_out);

/**
 * @brief Checks if storage media is currently mounted and operational.
 */
bool storage_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif /* STORAGE_H */
