#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Sampling & Timing Configurations
 * ========================================================================= */
#define CFG_ADC_SAMPLE_RATE_HZ          1000U   /* Timer trigger rate for ADC */
#define CFG_SENSOR_RATE_HZ              100U    /* MPU6050 sampling task rate (10ms) */
#define CFG_DMA_HALF_BUF_SAMPLES        10U     /* Circular buffer = 20 samples -> half = 10ms */

/* =========================================================================
 * Storage & Record Packaging
 * ========================================================================= */
#define CFG_RECORD_QUEUE_DEPTH          64U     /* Depth of queue between RecordTask & LoggerTask */
#define CFG_RECORDS_PER_BLOCK           17U     /* 17 records x 28 bytes = 476 bytes */
#define CFG_SD_BLOCK_BYTES              512U    /* 1 sector = 512 bytes */
#define CFG_LOG_MAX_SIZE_MB             16U     /* Pre-allocate max size per file */

/* =========================================================================
 * Watchdog & System Monitor
 * ========================================================================= */
#define CFG_WDG_TIMEOUT_MS              2000U   /* Hardware independent watchdog window */
#define CFG_MONITOR_PERIOD_MS           500U    /* MonitorTask check-in cadence */

/* Event group check-in bits for MonitorTask */
#define MONITOR_BIT_ADC                 (1U << 0)
#define MONITOR_BIT_RECORD              (1U << 1)
#define MONITOR_BIT_LOGGER              (1U << 2)
#define MONITOR_BIT_SHELL               (1U << 3)
#define MONITOR_ALL_TASKS_MASK          (MONITOR_BIT_ADC | MONITOR_BIT_RECORD | MONITOR_BIT_LOGGER | MONITOR_BIT_SHELL)

/* =========================================================================
 * FreeRTOS Task Priorities (Higher number = higher priority in FreeRTOS)
 * ========================================================================= */
#define PRI_ADC_TASK                    5U
#define PRI_RECORD_TASK                 4U
#define PRI_LOAD_TASK                   3U      /* Inversion experiment only */
#define PRI_LOGGER_TASK                 2U
#define PRI_SHELL_TASK                  1U
#define PRI_MONITOR_TASK                1U      /* Lowest application priority so starvation is caught */

/* =========================================================================
 * FreeRTOS Task Stack Sizes (in 32-bit words)
 * ========================================================================= */
#define STACK_ADC_TASK                  256U
#define STACK_RECORD_TASK               512U
#define STACK_LOGGER_TASK               1024U   /* Extra room for FatFs work buffers */
#define STACK_SHELL_TASK                512U
#define STACK_MONITOR_TASK              256U
#define STACK_LOAD_TASK                 256U

/* =========================================================================
 * Priority Inversion Experiment Modes
 * Mode 0: Binary semaphore without inheritance (problem demonstrator)
 * Mode 1: FreeRTOS Mutex with priority inheritance
 * Mode 2: Short critical section (copy only under lock, slow I/O outside)
 * ========================================================================= */
#ifndef CFG_LOCK_MODE
#define CFG_LOCK_MODE                   1U      /* Default to safe Mode 1 */
#endif

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */
