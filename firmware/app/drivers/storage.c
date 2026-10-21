#include "storage.h"
#include "crc32.h"
#include <stdio.h>
#include <string.h>

#if defined(USE_FATFS) || defined(FATFS)
#include "ff.h"
static FATFS   s_fatfs;
static FIL     s_current_file;
static bool    s_mounted = false;
static bool    s_file_open = false;
static uint16_t s_current_file_idx = 0;

StorageStatus_t storage_init(void) {
    FRESULT res = f_mount(&s_fatfs, "", 1);
    if (res != FR_OK) {
        s_mounted = false;
        return STORAGE_ERR_NOT_MOUNTED;
    }
    s_mounted = true;
    return STORAGE_OK;
}

StorageStatus_t storage_open_new_log(uint16_t *file_num_out) {
    if (!s_mounted) return STORAGE_ERR_NOT_MOUNTED;

    /* Scan existing log_XXXX.bin to find the next unused index */
    char filename[32];
    for (uint16_t idx = 1; idx < 9999; idx++) {
        snprintf(filename, sizeof(filename), "log_%04u.bin", idx);
        FILINFO fno;
        if (f_stat(filename, &fno) != FR_OK) {
            s_current_file_idx = idx;
            break;
        }
    }

    FRESULT res = f_open(&s_current_file, filename, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if (res != FR_OK) {
        return STORAGE_ERR_FILE_CREATE;
    }

    /* Pre-allocate 16MB file space to protect against power-cut FAT size desync */
    #if FF_USE_EXPAND
    f_expand(&s_current_file, 16UL * 1024UL * 1024UL, 1);
    #endif

    s_file_open = true;
    if (file_num_out) *file_num_out = s_current_file_idx;
    return STORAGE_OK;
}

StorageStatus_t storage_write_block(const LogBlock_t *block) {
    if (!s_file_open) return STORAGE_ERR_NOT_MOUNTED;

    UINT bytes_written = 0;
    FRESULT res = f_write(&s_current_file, block, sizeof(LogBlock_t), &bytes_written);
    if (res != FR_OK || bytes_written != sizeof(LogBlock_t)) {
        return STORAGE_ERR_WRITE_FAIL;
    }

    return STORAGE_OK;
}

StorageStatus_t storage_flush(void) {
    if (!s_file_open) return STORAGE_ERR_NOT_MOUNTED;
    return (f_sync(&s_current_file) == FR_OK) ? STORAGE_OK : STORAGE_ERR_WRITE_FAIL;
}

StorageStatus_t storage_close(void) {
    if (!s_file_open) return STORAGE_OK;
    f_close(&s_current_file);
    s_file_open = false;
    return STORAGE_OK;
}

StorageStatus_t storage_scan_last_valid(uint32_t *recovered_records_out) {
    if (!s_mounted) return STORAGE_ERR_NOT_MOUNTED;
    if (s_current_file_idx == 0) return STORAGE_OK;

    char filename[32];
    snprintf(filename, sizeof(filename), "log_%04u.bin", s_current_file_idx);

    FIL f;
    if (f_open(&f, filename, FA_READ) != FR_OK) {
        return STORAGE_OK;
    }

    uint32_t count = 0;
    LogBlock_t blk;
    UINT bytes_read = 0;

    while (f_read(&f, &blk, sizeof(LogBlock_t), &bytes_read) == FR_OK && bytes_read == sizeof(LogBlock_t)) {
        if (blk.magic != BLOCK_MAGIC) break;
        size_t crc_payload_len = offsetof(LogBlock_t, crc32);
        uint32_t crc = crc32_compute((const uint8_t *)&blk, crc_payload_len);
        if (crc != blk.crc32) break;

        count += blk.record_count;
    }

    f_close(&f);
    if (recovered_records_out) *recovered_records_out = count;
    return STORAGE_OK;
}

bool storage_is_ready(void) {
    return s_mounted && s_file_open;
}

#else

/* Simulation / fallback storage stub */
static bool s_mock_open = true;

StorageStatus_t storage_init(void) { return STORAGE_OK; }
StorageStatus_t storage_open_new_log(uint16_t *file_num_out) {
    if (file_num_out) *file_num_out = 1;
    s_mock_open = true;
    return STORAGE_OK;
}
StorageStatus_t storage_write_block(const LogBlock_t *block) { (void)block; return STORAGE_OK; }
StorageStatus_t storage_flush(void) { return STORAGE_OK; }
StorageStatus_t storage_close(void) { s_mock_open = false; return STORAGE_OK; }
StorageStatus_t storage_scan_last_valid(uint32_t *recovered_records_out) {
    if (recovered_records_out) *recovered_records_out = 0;
    return STORAGE_OK;
}
bool storage_is_ready(void) { return s_mock_open; }

#endif
