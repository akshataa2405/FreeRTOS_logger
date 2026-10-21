#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Computes standard IEEE 802.3 CRC-32 (polynomial 0xEDB88320).
 * Initial value 0xFFFFFFFF, inverted at end.
 * Compatible with Python's zlib.crc32 / binascii.crc32.
 *
 * @param data Pointer to input buffer
 * @param length Number of bytes to process
 * @return 32-bit CRC checksum
 */
uint32_t crc32_compute(const uint8_t *data, size_t length);

/**
 * @brief Continues an incremental CRC-32 computation.
 *
 * @param current_crc Current intermediate CRC value
 * @param data Pointer to new data
 * @param length Number of bytes
 * @return Updated CRC checksum
 */
uint32_t crc32_update(uint32_t current_crc, const uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* CRC32_H */
