#ifndef RESET_CAUSE_H
#define RESET_CAUSE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RESET_CAUSE_UNKNOWN = 0,
    RESET_CAUSE_POWER_ON,       /* Power-on / Brown-out Reset */
    RESET_CAUSE_EXTERNAL_PIN,   /* NRST pin asserted */
    RESET_CAUSE_SOFTWARE,       /* Software reset via NVIC_SystemReset() */
    RESET_CAUSE_INDEPENDENT_WDG,/* Independent Watchdog timed out */
    RESET_CAUSE_WINDOW_WDG,     /* Window Watchdog reset */
    RESET_CAUSE_LOW_POWER       /* Low-power entry reset */
} ResetCause_t;

/**
 * @brief Reads RCC status flags at boot, detects the reset source,
 * clears the flags in hardware, and logs the source.
 *
 * @return Detected reset cause enum
 */
ResetCause_t reset_cause_detect(void);

/**
 * @brief Returns the last detected reset cause.
 */
ResetCause_t reset_cause_get(void);

/**
 * @brief Returns a human-readable string describing the reset cause.
 */
const char *reset_cause_to_string(ResetCause_t cause);

/**
 * @brief Prints the boot reset report to console.
 */
void reset_cause_report(void);

#ifdef __cplusplus
}
#endif

#endif /* RESET_CAUSE_H */
