#ifndef RECORD_FORMAT_H
#define RECORD_FORMAT_H

#include <stdint.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_MAGIC                 0xDA7A1068UL   /* "DATA LOG" */
#define BLOCK_VERSION               0x01U
#define RECORDS_PER_BLOCK           17U

/* Record status flags */
#define RECORD_FLAG_NORMAL          0x0000U
#define RECORD_FLAG_OVERRUN         0x0001U        /* Queue was full or DMA buffer overrun occurred */
#define RECORD_FLAG_FAULT_EVENT     0x0002U        /* Record logs a captured fault recovery event */
#define RECORD_FLAG_RATE_CHANGED    0x0004U        /* Log rate changed on the fly */

/* Block status flags */
#define BLOCK_FLAG_COMPLETE         0x01U
#define BLOCK_FLAG_INCOMPLETE       0x02U

#pragma pack(push, 1)

/**
 * @brief Individual sample record produced every sensor tick (100 Hz = 10 ms).
 * Exactly 28 bytes.
 */
typedef struct {
    uint32_t seq;           /* Monotonic sequence number (detects missing data) */
    uint32_t timestamp_us;  /* Monotonic microsecond timestamp from 32-bit hardware timer */
    int16_t  adc_mean;      /* ADC channel mean over window */
    int16_t  adc_min;       /* ADC channel min over window */
    int16_t  adc_max;       /* ADC channel max over window */
    int16_t  ax;            /* MPU6050 Accelerometer X */
    int16_t  ay;            /* MPU6050 Accelerometer Y */
    int16_t  az;            /* MPU6050 Accelerometer Z */
    int16_t  gx;            /* MPU6050 Gyroscope X */
    int16_t  gy;            /* MPU6050 Gyroscope Y */
    int16_t  gz;            /* MPU6050 Gyroscope Z */
    uint16_t flags;         /* Bitfield of status/warning flags */
} DataRecord_t;

/**
 * @brief SD Card / Flash storage block structure.
 * Exactly 512 bytes (matches hardware storage sector size).
 * 16 bytes header + 17 * 28 bytes records (476) + 16 bytes pad + 4 bytes CRC-32 = 512 bytes.
 */
typedef struct {
    uint32_t     magic;                     /* 0xDA7A1068UL */
    uint8_t      version;                   /* 0x01 */
    uint8_t      flags;                     /* Block flags */
    uint16_t     record_count;              /* Count of valid records in this block (1..17) */
    uint32_t     first_seq;                 /* Sequence number of records[0] */
    uint32_t     session_id;                /* Session ID (unique per boot/start) */
    DataRecord_t records[RECORDS_PER_BLOCK];/* 17 * 28 = 476 bytes */
    uint8_t      padding[16];               /* Zero padding to align to 508 bytes */
    uint32_t     crc32;                     /* CRC-32 over bytes [0 .. 507] */
} LogBlock_t;

#pragma pack(pop)

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert(sizeof(DataRecord_t) == 28, "DataRecord_t must be exactly 28 bytes");
_Static_assert(sizeof(LogBlock_t) == 512, "LogBlock_t must be exactly 512 bytes");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RECORD_FORMAT_H */
