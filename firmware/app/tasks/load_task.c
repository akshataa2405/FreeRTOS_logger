#include "load_task.h"
#include "config.h"

#if defined(FREERTOS) || defined(INC_FREERTOS_H)
#include "FreeRTOS.h"
#include "task.h"

static TaskHandle_t s_load_task_handle = NULL;
static StackType_t  s_load_stack[STACK_LOAD_TASK];
static StaticTask_t s_load_tcb;
#endif

static volatile bool s_load_enabled = true;

void load_task_enable(bool enable) { s_load_enabled = enable; }
bool load_task_is_enabled(void)    { return s_load_enabled; }

void load_task_code(void *params) {
    (void)params;

#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    TickType_t last_wake = xTaskGetTickCount();

    for (;;) {
        /* Wakes every 50 ms */
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(50));

        if (s_load_enabled) {
            /* Burns ~5 ms CPU cycles without yielding */
            /* ARTIFICIAL DELAY - experiment only */
            volatile uint32_t count = 0;
            const uint32_t busy_limit = 100000; /* Approximate ~5ms at 180 MHz core clock */
            while (count < busy_limit) {
                count++;
            }
        }
    }
#endif
}

void load_task_start(void) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    s_load_task_handle = xTaskCreateStatic(
        load_task_code,
        "LoadTask",
        STACK_LOAD_TASK,
        NULL,
        PRI_LOAD_TASK,
        s_load_stack,
        &s_load_tcb
    );
#endif
}
