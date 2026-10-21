#ifndef DEBUG_GPIO_H
#define DEBUG_GPIO_H

#include <stdint.h>
#include <stdbool.h>

#if defined(__arm__) || defined(__thumb__)
#include "stm32f4xx_hal.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Debug GPIO Pin Assignments for USB Logic Analyzer (Saleae / PulseView) */
#define DBG_ADC_PORT            GPIOA
#define DBG_ADC_PIN             GPIO_PIN_8

#define DBG_SENSOR_PORT         GPIOA
#define DBG_SENSOR_PIN          GPIO_PIN_9

#define DBG_LOGGER_PORT         GPIOA
#define DBG_LOGGER_PIN          GPIO_PIN_10

#define DBG_LOCK_PORT           GPIOB
#define DBG_LOCK_PIN            GPIO_PIN_0

#define DBG_WAITING_PORT        GPIOB
#define DBG_WAITING_PIN         GPIO_PIN_1

void debug_gpio_init(void);

static inline void dbg_adc_set(void) {
#if defined(DBG_ADC_PORT) && defined(HAL_GPIO_WritePin)
    HAL_GPIO_WritePin(DBG_ADC_PORT, DBG_ADC_PIN, GPIO_PIN_SET);
#endif
}

static inline void dbg_adc_clr(void) {
#if defined(DBG_ADC_PORT) && defined(HAL_GPIO_WritePin)
    HAL_GPIO_WritePin(DBG_ADC_PORT, DBG_ADC_PIN, GPIO_PIN_RESET);
#endif
}

static inline void dbg_sensor_set(void) {
#if defined(DBG_SENSOR_PORT) && defined(HAL_GPIO_WritePin)
    HAL_GPIO_WritePin(DBG_SENSOR_PORT, DBG_SENSOR_PIN, GPIO_PIN_SET);
#endif
}

static inline void dbg_sensor_clr(void) {
#if defined(DBG_SENSOR_PORT) && defined(HAL_GPIO_WritePin)
    HAL_GPIO_WritePin(DBG_SENSOR_PORT, DBG_SENSOR_PIN, GPIO_PIN_RESET);
#endif
}

static inline void dbg_logger_set(void) {
#if defined(DBG_LOGGER_PORT) && defined(HAL_GPIO_WritePin)
    HAL_GPIO_WritePin(DBG_LOGGER_PORT, DBG_LOGGER_PIN, GPIO_PIN_SET);
#endif
}

static inline void dbg_logger_clr(void) {
#if defined(DBG_LOGGER_PORT) && defined(HAL_GPIO_WritePin)
    HAL_GPIO_WritePin(DBG_LOGGER_PORT, DBG_LOGGER_PIN, GPIO_PIN_RESET);
#endif
}

static inline void dbg_lock_set(void) {
#if defined(DBG_LOCK_PORT) && defined(HAL_GPIO_WritePin)
    HAL_GPIO_WritePin(DBG_LOCK_PORT, DBG_LOCK_PIN, GPIO_PIN_SET);
#endif
}

static inline void dbg_lock_clr(void) {
#if defined(DBG_LOCK_PORT) && defined(HAL_GPIO_WritePin)
    HAL_GPIO_WritePin(DBG_LOCK_PORT, DBG_LOCK_PIN, GPIO_PIN_RESET);
#endif
}

static inline void dbg_waiting_set(void) {
#if defined(DBG_WAITING_PORT) && defined(HAL_GPIO_WritePin)
    HAL_GPIO_WritePin(DBG_WAITING_PORT, DBG_WAITING_PIN, GPIO_PIN_SET);
#endif
}

static inline void dbg_waiting_clr(void) {
#if defined(DBG_WAITING_PORT) && defined(HAL_GPIO_WritePin)
    HAL_GPIO_WritePin(DBG_WAITING_PORT, DBG_WAITING_PIN, GPIO_PIN_RESET);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_GPIO_H */
