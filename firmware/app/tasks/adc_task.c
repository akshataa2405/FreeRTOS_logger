#include "adc_task.h"
#include "staging_lock.h"
#include "debug_gpio.h"
#include "monitor_task.h"
#include <string.h>

#if defined(FREERTOS) || defined(INC_FREERTOS_H)
#include "FreeRTOS.h"
#include "task.h"

static TaskHandle_t s_adc_task_handle = NULL;
static StackType_t  s_adc_stack[STACK_ADC_TASK];
static StaticTask_t s_adc_tcb;

/* DMA circular buffer: 20 samples total (10 half + 10 full) */
#define TOTAL_DMA_SAMPLES (CFG_DMA_HALF_BUF_SAMPLES * 2)
static uint16_t s_adc_dma_buffer[TOTAL_DMA_SAMPLES];
#endif

volatile LatestAdcData_t g_latest_adc = {0, 0, 0, 0};

static void process_adc_buffer(const uint16_t *samples, uint32_t count) {
    if (count == 0) return;

    dbg_adc_set();

    int32_t sum = 0;
    int16_t min_val = (int16_t)samples[0];
    int16_t max_val = (int16_t)samples[0];

    for (uint32_t i = 0; i < count; i++) {
        int16_t s = (int16_t)samples[i];
        sum += s;
        if (s < min_val) min_val = s;
        if (s > max_val) max_val = s;
    }

    int16_t mean_val = (int16_t)(sum / (int32_t)count);

    /* Update shared slot under lock */
    staging_lock_acquire();
    g_latest_adc.mean = mean_val;
    g_latest_adc.min = min_val;
    g_latest_adc.max = max_val;
    /* Timestamp can be read from free-running microsecond timer (e.g. TIM3->CNT) */
#if defined(TIM3)
    g_latest_adc.timestamp_us = TIM3->CNT;
#else
    g_latest_adc.timestamp_us = 0;
#endif
    staging_lock_release();

    dbg_adc_clr();
}

void adc_task_code(void *params) {
    (void)params;
    uint32_t notif_value = 0;

    for (;;) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
        /* Wait for notification from DMA ISR */
        if (xTaskNotifyWait(0, ULONG_MAX, &notif_value, portMAX_DELAY) == pdTRUE) {
            if (notif_value & ADC_NOTIF_HALF_TRANSFER) {
                process_adc_buffer(&s_adc_dma_buffer[0], CFG_DMA_HALF_BUF_SAMPLES);
            }
            if (notif_value & ADC_NOTIF_FULL_TRANSFER) {
                process_adc_buffer(&s_adc_dma_buffer[CFG_DMA_HALF_BUF_SAMPLES], CFG_DMA_HALF_BUF_SAMPLES);
            }

            /* Check in with watchdog monitor */
            monitor_task_checkin(MONITOR_BIT_ADC);
        }
#else
        break;
#endif
    }
}

void adc_task_start(void) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    s_adc_task_handle = xTaskCreateStatic(
        adc_task_code,
        "AdcTask",
        STACK_ADC_TASK,
        NULL,
        PRI_ADC_TASK,
        s_adc_stack,
        &s_adc_tcb
    );
#endif
}

void adc_dma_half_transfer_isr(void) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (s_adc_task_handle) {
        xTaskNotifyFromISR(s_adc_task_handle, ADC_NOTIF_HALF_TRANSFER, eSetBits, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
#endif
}

void adc_dma_full_transfer_isr(void) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (s_adc_task_handle) {
        xTaskNotifyFromISR(s_adc_task_handle, ADC_NOTIF_FULL_TRANSFER, eSetBits, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
#endif
}
