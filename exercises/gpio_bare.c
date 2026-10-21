/**
 * @file gpio_bare.c
 * @brief Bare-metal register-level GPIO toggle on STM32F446RE (no HAL).
 * Target LED: User LED LD2 on PA5.
 *
 * What happens under the hood:
 * 1. RCC AHB1 peripheral clock enable register (RCC_AHB1ENR): Enable GPIOA clock (bit 0).
 * 2. GPIO port mode register (GPIOA_MODER): Configure PA5 as General purpose output mode (01b at bits 11:10).
 * 3. GPIO port output type register (GPIOA_OTYPER): Push-pull (bit 5 = 0).
 * 4. GPIO port output data register (GPIOA_ODR): Toggle bit 5 to turn LED on/off.
 */

#include <stdint.h>

#define RCC_BASE            0x40023800UL
#define RCC_AHB1ENR         (*(volatile uint32_t *)(RCC_BASE + 0x30UL))

#define GPIOA_BASE          0x40020000UL
#define GPIOA_MODER         (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_OTYPER        (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))
#define GPIOA_ODR           (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))
#define GPIOA_BSRR          (*(volatile uint32_t *)(GPIOA_BASE + 0x18UL))

static void delay(volatile uint32_t count) {
    while (count--) {
        __asm__ volatile ("nop");
    }
}

int main(void) {
    /* 1. Enable GPIOA peripheral clock */
    RCC_AHB1ENR |= (1UL << 0);

    /* 2. Configure PA5 as output (MODER bits 11:10 = 01) */
    GPIOA_MODER &= ~(3UL << (5 * 2));
    GPIOA_MODER |=  (1UL << (5 * 2));

    /* 3. Output push-pull */
    GPIOA_OTYPER &= ~(1UL << 5);

    /* 4. Main loop: toggle LED via BSRR (atomic set/reset) */
    while (1) {
        GPIOA_BSRR = (1UL << 5);       /* Turn ON (Set bit 5) */
        delay(500000);
        GPIOA_BSRR = (1UL << (5 + 16));/* Turn OFF (Reset bit 5 via high half-word) */
        delay(500000);
    }

    return 0;
}
