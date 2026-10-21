#include "hardfault.h"
#include "crc32.h"
#include <stdio.h>
#include <string.h>

#if defined(__arm__) || defined(__thumb__)
#include "stm32f4xx.h"
#else
/* Dummy addresses for host syntax check if compiled outside ARM target */
#define SCB_CFSR_ADDR   0xE000ED28UL
#define SCB_HFSR_ADDR   0xE000ED2CUL
#define SCB_MMFAR_ADDR  0xE000ED34UL
#define SCB_BFAR_ADDR   0xE000ED38UL
#endif

/* Place in noinit section to prevent C runtime startup from zeroing it */
#if defined(__GNUC__)
__attribute__((section(".noinit")))
#endif
static volatile FaultInfo_t s_fault_storage;

void fault_set_missing_tasks(uint32_t mask) {
    s_fault_storage.missing_tasks_mask = mask;
}

void HardFault_C_Handler(uint32_t *frame) {
    /* Stacked registers on Cortex-M exception entry */
    s_fault_storage.r0   = frame[0];
    s_fault_storage.r1   = frame[1];
    s_fault_storage.r2   = frame[2];
    s_fault_storage.r3   = frame[3];
    s_fault_storage.r12  = frame[4];
    s_fault_storage.lr   = frame[5];
    s_fault_storage.pc   = frame[6];
    s_fault_storage.xpsr = frame[7];

#if defined(SCB)
    s_fault_storage.cfsr  = SCB->CFSR;
    s_fault_storage.hfsr  = SCB->HFSR;
    s_fault_storage.mmfar = SCB->MMFAR;
    s_fault_storage.bfar  = SCB->BFAR;
#else
    s_fault_storage.cfsr  = *(volatile uint32_t *)SCB_CFSR_ADDR;
    s_fault_storage.hfsr  = *(volatile uint32_t *)SCB_HFSR_ADDR;
    s_fault_storage.mmfar = *(volatile uint32_t *)SCB_MMFAR_ADDR;
    s_fault_storage.bfar  = *(volatile uint32_t *)SCB_BFAR_ADDR;
#endif

    s_fault_storage.magic = FAULT_MAGIC_VALUE;

    /* Compute CRC32 over all fields except the crc32 field itself */
    size_t payload_len = sizeof(FaultInfo_t) - sizeof(uint32_t);
    s_fault_storage.crc32 = crc32_compute((const uint8_t *)&s_fault_storage, payload_len);

    /* Software reset to reboot the MCU cleanly */
#if defined(NVIC_SystemReset)
    NVIC_SystemReset();
#else
    while (1) {}
#endif
}

bool fault_check_and_report(void) {
    if (s_fault_storage.magic != FAULT_MAGIC_VALUE) {
        return false;
    }

    size_t payload_len = sizeof(FaultInfo_t) - sizeof(uint32_t);
    uint32_t expected_crc = crc32_compute((const uint8_t *)&s_fault_storage, payload_len);

    if (s_fault_storage.crc32 != expected_crc) {
        /* RAM contents corrupt or uninitialized */
        s_fault_storage.magic = 0;
        return false;
    }

    printf("\n========================================\n");
    printf("   CRITICAL: HARDFAULT CAPTURE DETECTED\n");
    printf("========================================\n");
    printf("Stacked Registers:\n");
    printf("  R0  = 0x%08lX   R1  = 0x%08lX\n", (unsigned long)s_fault_storage.r0, (unsigned long)s_fault_storage.r1);
    printf("  R2  = 0x%08lX   R3  = 0x%08lX\n", (unsigned long)s_fault_storage.r2, (unsigned long)s_fault_storage.r3);
    printf("  R12 = 0x%08lX   LR  = 0x%08lX\n", (unsigned long)s_fault_storage.r12, (unsigned long)s_fault_storage.lr);
    printf("  PC  = 0x%08lX   xPSR= 0x%08lX\n", (unsigned long)s_fault_storage.pc, (unsigned long)s_fault_storage.xpsr);
    printf("\nFault Status Registers:\n");
    printf("  CFSR  = 0x%08lX\n", (unsigned long)s_fault_storage.cfsr);
    printf("  HFSR  = 0x%08lX\n", (unsigned long)s_fault_storage.hfsr);
    printf("  MMFAR = 0x%08lX\n", (unsigned long)s_fault_storage.mmfar);
    printf("  BFAR  = 0x%08lX\n", (unsigned long)s_fault_storage.bfar);
    if (s_fault_storage.missing_tasks_mask != 0) {
        printf("  Watchdog Missing Tasks Mask = 0x%02lX\n", (unsigned long)s_fault_storage.missing_tasks_mask);
    }
    printf("\nTo decode line of code:\n");
    printf("  arm-none-eabi-addr2line -e logger.elf 0x%08lX\n", (unsigned long)s_fault_storage.pc);
    printf("========================================\n\n");

    /* Clear the magic to prevent reporting on subsequent clean boots */
    s_fault_storage.magic = 0;
    return true;
}

void fault_trigger(const char *type) {
    if (!type) return;

    if (strcmp(type, "null") == 0) {
        /* BusFault / HardFault on invalid memory access */
        volatile uint32_t *bad_ptr = (volatile uint32_t *)0x00000000;
        *bad_ptr = 0xDEADBEEF;
    } else if (strcmp(type, "divzero") == 0) {
#if defined(SCB)
        /* Enable divide by zero trap */
        SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;
#endif
        volatile int numerator = 100;
        volatile int denominator = 0;
        volatile int result = numerator / denominator;
        (void)result;
    } else if (strcmp(type, "unaligned") == 0) {
#if defined(SCB)
        SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif
        volatile uint8_t buf[8] = {0};
        volatile uint32_t *unaligned_ptr = (volatile uint32_t *)&buf[1];
        volatile uint32_t val = *unaligned_ptr;
        (void)val;
    } else if (strcmp(type, "invstate") == 0) {
        /* Jump to address without Thumb bit set (bit 0 = 0) */
        void (*bad_func)(void) = (void (*)(void))0x08000100;
        bad_func();
    }
}
