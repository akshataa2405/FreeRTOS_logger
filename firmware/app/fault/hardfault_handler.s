/**
 * @file hardfault_handler.s
 * @brief Cortex-M4 HardFault vector entry point.
 * Extracts either Main Stack Pointer (MSP) or Process Stack Pointer (PSP)
 * based on EXC_RETURN (bit 2 of LR), placing it in R0 as the first parameter
 * to HardFault_C_Handler.
 */

    .syntax unified
    .arch armv7-m
    .thumb

    .global HardFault_Handler
    .type HardFault_Handler, %function

HardFault_Handler:
    tst   lr, #4            /* Test bit 2 of EXC_RETURN */
    ite   eq
    mrseq r0, msp           /* If 0, MSP was used (e.g. within an ISR or prior to OS start) */
    mrsne r0, psp           /* If 1, PSP was used (FreeRTOS user task thread context) */
    b     HardFault_C_Handler

    .size HardFault_Handler, .-HardFault_Handler
