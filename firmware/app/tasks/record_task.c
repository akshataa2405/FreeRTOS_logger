#include "record_task.h"
#include "adc_task.h"
#include "staging_lock.h"
#include "debug_gpio.h"
#include "monitor_task.h"
#include "mpu6050.h"
#include <string.h>

#if defined(FREERTOS) || defined(INC_FREERTOS_H)
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

static TaskHandle_t s_record_task_handle = NULL;
static StackType_t  s_record_stack[STACK_RECORD_TASK];
static StaticTask_t s_record_tcb;

QueueHandle_t       g_record_queue = NULL;
static uint8_t      s_record_queue_storage[CFG_RECORD_QUEUE_DEPTH * sizeof(DataRecord_t)];
static StaticQueue_t s_record_queue_struct;
#endif

static volatile uint32_t s_sample_counter = 0;
static volatile uint32_t s_dropped_samples = 0;
static volatile uint32_t s_current_rate_hz = CFG_SENSOR_RATE_HZ;

uint32_t record_task_get_sample_count(void) { return s_sample_counter; }
uint32_t record_task_get_drop_count(void) { return s_dropped_samples; }
void     record_task_set_rate_hz(uint32_t rate_hz) {
    if (rate_hz >= 10 && rate_hz <= 500) {
        s_current_rate_hz = rate_hz;
    }
}
uint32_t record_task_get_rate_hz(void) { return s_current_rate_hz; }

void record_task_code(void *params) {
    (void)params;

#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        TickType_t period_ticks = pdMS_TO_TICKS(1000U / s_current_rate_hz);
        if (period_ticks == 0) period_ticks = 1;
        vTaskDelayUntil(&last_wake_time, period_ticks);

        DataRecord_t rec;
        memset(&rec, 0, sizeof(rec));

        /* 1. Acquire staging lock and read latest ADC block */
        dbg_waiting_set();
        staging_lock_acquire();
        dbg_waiting_clr();

        rec.adc_mean = g_latest_adc.mean;
        rec.adc_min  = g_latest_adc.min;
        rec.adc_max  = g_latest_adc.max;

        staging_lock_release();

        /* 2. Read I2C sensor (MPU6050) */
        dbg_sensor_set();
#if defined(hi2c1)
        extern I2C_HandleTypeDef hi2c1;
        mpu6050_read_all(&hi2c1, &rec.ax, &rec.ay, &rec.az, &rec.gx, &rec.gy, &rec.gz);
#else
        mpu6050_read_all(NULL, &rec.ax, &rec.ay, &rec.az, &rec.gx, &rec.gy, &rec.gz);
#endif
        dbg_sensor_clr();

        /* 3. Stamp sequence and microsecond timestamp */
        rec.seq = s_sample_counter++;
#if defined(TIM3)
        rec.timestamp_us = TIM3->CNT;
#else
        rec.timestamp_us = (uint32_t)(xTaskGetTickCount() * 1000U);
#endif
        rec.flags = RECORD_FLAG_NORMAL;

        /* 4. Non-blocking queue send to LoggerTask */
        if (g_record_queue != NULL) {
            if (xQueueSend(g_record_queue, &rec, 0) != pdPASS) {
                /* Queue overrun: drop newest and increment drop counter */
                s_dropped_samples++;
                rec.flags |= RECORD_FLAG_OVERRUN;
            }
        }

        /* 5. Check in with watchdog monitor */
        monitor_task_checkin(MONITOR_BIT_RECORD);
    }
#endif
}

void record_task_start(void) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    g_record_queue = xQueueCreateStatic(
        CFG_RECORD_QUEUE_DEPTH,
        sizeof(DataRecord_t),
        s_record_queue_storage,
        &s_record_queue_struct
    );

    s_record_task_handle = xTaskCreateStatic(
        record_task_code,
        "RecordTask",
        STACK_RECORD_TASK,
        NULL,
        PRI_RECORD_TASK,
        s_record_stack,
        &s_record_tcb
    );
#endif
}
