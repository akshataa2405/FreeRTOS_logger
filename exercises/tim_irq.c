/**
 * @file tim_irq.c
 * @brief Bare-metal register-level Timer 2 interrupt and NVIC configuration on STM32F446RE.
 * Toggles PA5 LED every 1.0s via TIM2 Update Interrupt (no HAL).
 *
 * Core Concepts:
 * 1. Timer Clock Calculation:
 *    HSI = 16 MHz.
 *    PSC = 15999 -> Counter clock = 16 MHz / (15999 + 1) = 1 kHz.
 *    ARR = 999   -> Interrupt rate = 1 kHz / (999 + 1) = 1 Hz (1.000 second).
 * 2. NVIC (Nested Vectored Interrupt Controller):
 *    TIM2 IRQ number = 28.
 *    NVIC_ISER[0] bit 28 enables IRQ28.
 *    Priority set in NVIC_IPR[28].
 */

#include <stdint.h>

#define RCC_BASE            0x40023800UL
#define RCC_AHB1ENR         (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR         (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

#define GPIOA_BASE          0x40020000UL
#define GPIOA_MODER         (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_ODR           (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))

#define TIM2_BASE           0x40000000UL
#define TIM2_CR1            (*(volatile uint32_t *)(TIM2_BASE + 0x00UL))
#define TIM2_DIER           (*(volatile uint32_t *)(TIM2_BASE + 0x0CUL))
#define TIM2_SR             (*(volatile uint32_t *)(TIM2_BASE + 0x10UL))
#define TIM2_PSC            (*(volatile uint32_t *)(TIM2_BASE + 0x28UL))
#define TIM2_ARR            (*(volatile uint32_t *)(TIM2_BASE + 0x2CUL))

#define NVIC_ISER0          (*(volatile uint32_t *)(0xE000E100UL))
#define NVIC_IPR7           (*(volatile uint32_t *)(0xE000E41CUL)) /* Bytes for IRQ 28-31 */

#define TIM2_IRQN           28

void TIM2_IRQHandler(void) {
    if (TIM2_SR & (1UL << 0)) { /* UIF: Update Interrupt Flag */
        TIM2_SR &= ~(1UL << 0);  /* Clear flag */
        GPIOA_ODR ^= (1UL << 5); /* Toggle LED */
    }
}

int main(void) {
    /* 1. Enable GPIOA and TIM2 peripheral clocks */
    RCC_AHB1ENR |= (1UL << 0);
    RCC_APB1ENR |= (1UL << 0);

    /* 2. Configure PA5 as output */
    GPIOA_MODER &= ~(3UL << (5 * 2));
    GPIOA_MODER |=  (1UL << (5 * 2));

    /* 3. Configure TIM2 for 1 Hz update */
    TIM2_PSC = 15999UL;
    TIM2_ARR = 999UL;

    /* 4. Enable Update Interrupt in TIM2 */
    TIM2_DIER |= (1UL << 0);

    /* 5. Configure NVIC for TIM2 (IRQ 28) */
    /* Priority 5 in bits [7:4] of byte 0 in NVIC_IPR7 (IRQ 28) */
    NVIC_IPR7 &= ~(0xFFUL << 0);
    NVIC_IPR7 |=  (0x50UL << 0);
    NVIC_ISER0 = (1UL << TIM2_IRQN);

    /* 6. Enable Counter (CEN bit in CR1) */
    TIM2_CR1 |= (1UL << 0);

    while (1) {
        __asm__ volatile ("wfi"); /* Sleep until interrupt */
    }

    return 0;
}
