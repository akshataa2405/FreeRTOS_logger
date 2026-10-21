#ifndef HARDFAULT_H
#define HARDFAULT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FAULT_MAGIC_VALUE       0xDEADC0DEUL

/**
 * @brief Persistent fault diagnostics captured in non-zeroed SRAM (.noinit).
 */
typedef struct {
    uint32_t magic;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;
    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t mmfar;
    uint32_t bfar;
    uint32_t missing_tasks_mask; /* Populated if reset was forced by watchdog failure */
    uint32_t crc32;
} FaultInfo_t;

/**
 * @brief Core C handler called by the assembly stub during HardFault exception.
 *
 * @param frame Pointer to the active stack frame (MSP or PSP)
 */
void HardFault_C_Handler(uint32_t *frame);

/**
 * @brief Checks if a previous fault report exists in the .noinit region.
 * If valid, prints the register dump to UART and clears the region.
 *
 * @return true if a valid fault was retrieved and reported, false otherwise.
 */
bool fault_check_and_report(void);

/**
 * @brief Induces a deliberate fault for testing and interview demonstrations.
 *
 * @param type "null", "divzero", "unaligned", or "invstate"
 */
void fault_trigger(const char *type);

/**
 * @brief Writes missing task mask into noinit storage prior to watchdog reset.
 */
void fault_set_missing_tasks(uint32_t mask);

#ifdef __cplusplus
}
#endif

#endif /* HARDFAULT_H */
