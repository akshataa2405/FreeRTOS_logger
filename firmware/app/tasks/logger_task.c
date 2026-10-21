#include "logger_task.h"
#include "block_builder.h"
#include "storage.h"
#include "staging_lock.h"
#include "debug_gpio.h"
#include "monitor_task.h"
#include "config.h"

#if defined(FREERTOS) || defined(INC_FREERTOS_H)
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

extern QueueHandle_t g_record_queue;
static TaskHandle_t  s_logger_task_handle = NULL;
static StackType_t   s_logger_stack[STACK_LOGGER_TASK];
static StaticTask_t  s_logger_tcb;
#endif

static volatile bool     s_is_logging = true;
static volatile uint32_t s_blocks_written = 0;
static uint32_t          s_session_counter = 0x20261021U;

void logger_task_request_start(void) {
    if (!s_is_logging) {
        uint16_t file_idx = 0;
        storage_open_new_log(&file_idx);
        s_session_counter++;
        s_is_logging = true;
    }
}

void logger_task_request_stop(void) {
    if (s_is_logging) {
        storage_flush();
        storage_close();
        s_is_logging = false;
    }
}

bool     logger_task_is_logging(void) { return s_is_logging; }
uint32_t logger_task_get_blocks_written(void) { return s_blocks_written; }

void logger_task_code(void *params) {
    (void)params;

    BlockBuilder_t builder;
    block_builder_init(&builder, s_session_counter);

    DataRecord_t record;

    for (;;) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
        /* Wait for records arriving from RecordTask */
        if (g_record_queue != NULL && xQueueReceive(g_record_queue, &record, pdMS_TO_TICKS(100)) == pdPASS) {
            if (s_is_logging) {
                bool block_full = block_builder_add_record(&builder, &record);

                if (block_full) {
                    dbg_logger_set();

#if CFG_LOCK_MODE == 0 || CFG_LOCK_MODE == 1
                    /* Mode A & B: Deliberate lock hold during write to demonstrate priority inversion */
                    dbg_lock_set();
                    staging_lock_acquire();

                    /* Artificial busy-wait to make inversion clearly measurable on logic analyzer */
                    /* ARTIFICIAL DELAY - experiment only */
                    vTaskDelay(pdMS_TO_TICKS(4));

                    storage_write_block(&builder.current_block);

                    staging_lock_release();
                    dbg_lock_clr();
#elif CFG_LOCK_MODE == 2
                    /* Mode C (Best Practice): Fast copy only under lock, slow storage write outside lock */
                    LogBlock_t local_block_copy;
                    staging_lock_acquire();
                    local_block_copy = builder.current_block;
                    staging_lock_release();

                    storage_write_block(&local_block_copy);
#endif

                    s_blocks_written++;
                    block_builder_reset_ready(&builder);
                    dbg_logger_clr();
                }
            }
        }

        /* Periodic flush & watchdog check-in */
        monitor_task_checkin(MONITOR_BIT_LOGGER);
#else
        break;
#endif
    }
}

void logger_task_start(void) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    s_logger_task_handle = xTaskCreateStatic(
        logger_task_code,
        "LoggerTask",
        STACK_LOGGER_TASK,
        NULL,
        PRI_LOGGER_TASK,
        s_logger_stack,
        &s_logger_tcb
    );
#endif
}
