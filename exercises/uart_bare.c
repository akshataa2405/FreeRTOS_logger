/**
 * @file uart_bare.c
 * @brief Bare-metal register-level USART2 transmission on STM32F446RE (no HAL).
 * Target pins: PA2 (USART2_TX, AF7) -> ST-LINK Virtual COM Port (115200 8N1).
 *
 * Core Concept:
 * Default clock after reset is HSI (16 MHz). USART2 is on APB1.
 * 115200 Baud = 16,000,000 / (16 * 115200) = 8.6805
 * Mantissa = 8 (0x08), Fraction = 0.6805 * 16 = 11 (0x0B) -> BRR = 0x008B.
 */

#include <stdint.h>

#define RCC_BASE            0x40023800UL
#define RCC_AHB1ENR         (*(volatile uint32_t *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR         (*(volatile uint32_t *)(RCC_BASE + 0x40UL))

#define GPIOA_BASE          0x40020000UL
#define GPIOA_MODER         (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_AFRL          (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))

#define USART2_BASE         0x40004400UL
#define USART2_SR           (*(volatile uint32_t *)(USART2_BASE + 0x00UL))
#define USART2_DR           (*(volatile uint32_t *)(USART2_BASE + 0x04UL))
#define USART2_BRR          (*(volatile uint32_t *)(USART2_BASE + 0x08UL))
#define USART2_CR1          (*(volatile uint32_t *)(USART2_BASE + 0x0CUL))

#define USART_SR_TXE        (1UL << 7)

void uart2_init(void) {
    /* 1. Enable GPIOA and USART2 clocks */
    RCC_AHB1ENR |= (1UL << 0);
    RCC_APB1ENR |= (1UL << 17);

    /* 2. Configure PA2 as Alternate Function (MODER bits 5:4 = 10b) */
    GPIOA_MODER &= ~(3UL << (2 * 2));
    GPIOA_MODER |=  (2UL << (2 * 2));

    /* 3. Map PA2 to AF7 (USART2) in AFRL (bits 11:8 = 0111b) */
    GPIOA_AFRL &= ~(0xFUL << (2 * 4));
    GPIOA_AFRL |=  (0x7UL << (2 * 4));

    /* 4. Set baud rate to 115200 @ 16 MHz HSI */
    USART2_BRR = 0x008BUL;

    /* 5. Enable Transmitter (TE, bit 3) and USART (UE, bit 13) */
    USART2_CR1 = (1UL << 3) | (1UL << 13);
}

void uart2_putc(char c) {
    /* Wait until Transmit Data Register Empty (TXE) bit is set */
    while (!(USART2_SR & USART_SR_TXE)) {}
    USART2_DR = (uint8_t)c;
}

void uart2_puts(const char *str) {
    while (*str) {
        uart2_putc(*str++);
    }
}

int main(void) {
    uart2_init();
    uart2_puts("STM32 Bare-Metal UART Initialized!\r\n");

    while (1) {
        uart2_puts("Heartbeat from bare-metal register driver\r\n");
        for (volatile int i = 0; i < 1000000; i++) {}
    }

    return 0;
}
