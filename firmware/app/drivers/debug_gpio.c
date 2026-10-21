#include "debug_gpio.h"

void debug_gpio_init(void) {
#if defined(__arm__) || defined(__thumb__)
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure GPIOA pins: PA8, PA9, PA10 */
    GPIO_InitStruct.Pin = DBG_ADC_PIN | DBG_SENSOR_PIN | DBG_LOGGER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure GPIOB pins: PB0, PB1 */
    GPIO_InitStruct.Pin = DBG_LOCK_PIN | DBG_WAITING_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Initialize all pins low */
    dbg_adc_clr();
    dbg_sensor_clr();
    dbg_logger_clr();
    dbg_lock_clr();
    dbg_waiting_clr();
#endif
}
