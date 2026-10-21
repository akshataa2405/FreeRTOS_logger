#include "monitor_task.h"
#include "hardfault.h"
#include <stdio.h>

#if defined(FREERTOS) || defined(INC_FREERTOS_H)
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

static TaskHandle_t        s_monitor_task_handle = NULL;
static StackType_t         s_monitor_stack[STACK_MONITOR_TASK];
static StaticTask_t        s_monitor_tcb;

static EventGroupHandle_t  s_monitor_events = NULL;
static StaticEventGroup_t  s_monitor_event_struct;
#endif

#if defined(__arm__) || defined(__thumb__)
#include "stm32f4xx_hal.h"
extern IWDG_HandleTypeDef hiwdg;
#endif

static volatile bool s_force_hang = false;

void monitor_task_force_hang(void) {
    s_force_hang = true;
}

void monitor_task_checkin(uint32_t bit) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    if (s_monitor_events) {
        xEventGroupSetBits(s_monitor_events, bit);
    }
#else
    (void)bit;
#endif
}

void monitor_task_checkin_from_isr(uint32_t bit) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (s_monitor_events) {
        xEventGroupSetBitsFromISR(s_monitor_events, bit, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
#else
    (void)bit;
#endif
}

void monitor_task_code(void *params) {
    (void)params;

    for (;;) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
        vTaskDelay(pdMS_TO_TICKS(CFG_MONITOR_PERIOD_MS));

        if (s_force_hang) {
            /* Deliberately stop feeding watchdog to test reset */
            printf("[MONITOR] Deliberate hang induced: watchdog will fire in ~2s\n");
            vTaskDelay(portMAX_DELAY);
        }

        /* Check which tasks have checked in */
        EventBits_t bits = xEventGroupWaitBits(
            s_monitor_events,
            MONITOR_ALL_TASKS_MASK,
            pdTRUE,             /* Clear on exit */
            pdFALSE,            /* Don't wait for all; check whatever arrived */
            0                   /* Non-blocking query */
        );

        if ((bits & MONITOR_ALL_TASKS_MASK) == MONITOR_ALL_TASKS_MASK) {
            /* All tasks healthy: refresh hardware watchdog */
#if defined(__arm__) && defined(hiwdg)
            HAL_IWDG_Refresh(&hiwdg);
#endif
        } else {
            /* Missing task detected! */
            uint32_t missing = MONITOR_ALL_TASKS_MASK & (~bits);
            printf("[MONITOR WARNING] Task check-in missed! Missing mask: 0x%02lX\n", (unsigned long)missing);

            /* Save missing tasks to noinit memory for post-reset diagnostic */
            fault_set_missing_tasks(missing);

            /* DO NOT refresh watchdog -> hardware reset occurs automatically */
        }
#else
        break;
#endif
    }
}

void monitor_task_start(void) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    s_monitor_events = xEventGroupCreateStatic(&s_monitor_event_struct);

    s_monitor_task_handle = xTaskCreateStatic(
        monitor_task_code,
        "MonitorTask",
        STACK_MONITOR_TASK,
        NULL,
        PRI_MONITOR_TASK,
        s_monitor_stack,
        &s_monitor_tcb
    );
#endif
}
