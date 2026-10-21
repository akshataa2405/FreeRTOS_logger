#include "reset_cause.h"
#include <stdio.h>

#if defined(__arm__) || defined(__thumb__)
#include "stm32f4xx_hal.h"
#endif

static ResetCause_t s_detected_cause = RESET_CAUSE_UNKNOWN;

ResetCause_t reset_cause_detect(void) {
#if defined(RCC_FLAG_PORRST) && defined(__HAL_RCC_GET_FLAG)
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) {
        s_detected_cause = RESET_CAUSE_LOW_POWER;
    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) {
        s_detected_cause = RESET_CAUSE_WINDOW_WDG;
    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
        s_detected_cause = RESET_CAUSE_INDEPENDENT_WDG;
    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
        s_detected_cause = RESET_CAUSE_SOFTWARE;
    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST)) {
        s_detected_cause = RESET_CAUSE_POWER_ON;
    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) {
        s_detected_cause = RESET_CAUSE_EXTERNAL_PIN;
    } else {
        s_detected_cause = RESET_CAUSE_UNKNOWN;
    }

    /* Clear reset flags so future resets are unambiguous */
    __HAL_RCC_CLEAR_RESET_FLAGS();
#else
    s_detected_cause = RESET_CAUSE_POWER_ON;
#endif

    return s_detected_cause;
}

ResetCause_t reset_cause_get(void) {
    return s_detected_cause;
}

const char *reset_cause_to_string(ResetCause_t cause) {
    switch (cause) {
        case RESET_CAUSE_POWER_ON:        return "Power-On / Brown-out Reset (POR/BOR)";
        case RESET_CAUSE_EXTERNAL_PIN:    return "External Pin Reset (NRST button/line)";
        case RESET_CAUSE_SOFTWARE:        return "Software Reset (NVIC_SystemReset)";
        case RESET_CAUSE_INDEPENDENT_WDG: return "Independent Hardware Watchdog (IWDG)";
        case RESET_CAUSE_WINDOW_WDG:      return "Window Watchdog (WWDG)";
        case RESET_CAUSE_LOW_POWER:       return "Low-Power Reset";
        case RESET_CAUSE_UNKNOWN:
        default:                          return "Unknown Reset Cause";
    }
}

void reset_cause_report(void) {
    printf("[BOOT] Reset Cause: %s\n", reset_cause_to_string(s_detected_cause));
}
