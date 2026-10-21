#ifndef ADC_TASK_H
#define ADC_TASK_H

#include <stdint.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADC_NOTIF_HALF_TRANSFER     (1U << 0)
#define ADC_NOTIF_FULL_TRANSFER     (1U << 1)

typedef struct {
    int16_t  mean;
    int16_t  min;
    int16_t  max;
    uint32_t timestamp_us;
} LatestAdcData_t;

extern volatile LatestAdcData_t g_latest_adc;

void adc_task_start(void);
void adc_task_code(void *params);

/* ISR Notification helpers called from DMA Half/Full Transfer callbacks */
void adc_dma_half_transfer_isr(void);
void adc_dma_full_transfer_isr(void);

#ifdef __cplusplus
}
#endif

#endif /* ADC_TASK_H */
