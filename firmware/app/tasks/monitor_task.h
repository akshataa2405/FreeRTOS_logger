#ifndef MONITOR_TASK_H
#define MONITOR_TASK_H

#include <stdint.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

void monitor_task_start(void);
void monitor_task_code(void *params);

/**
 * @brief Sets a task's check-in bit to notify the monitor of active liveness.
 * Safe to call from tasks.
 *
 * @param bit One of MONITOR_BIT_ADC, MONITOR_BIT_RECORD, etc.
 */
void monitor_task_checkin(uint32_t bit);

/**
 * @brief Sets check-in bit from an ISR.
 */
void monitor_task_checkin_from_isr(uint32_t bit);

/**
 * @brief Triggers a deliberate watchdog hang for testing.
 */
void monitor_task_force_hang(void);

#ifdef __cplusplus
}
#endif

#endif /* MONITOR_TASK_H */
