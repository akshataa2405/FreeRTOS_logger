#ifndef STAGING_LOCK_H
#define STAGING_LOCK_H

#include "config.h"

#if defined(FREERTOS) || defined(INC_FREERTOS_H)
#include "FreeRTOS.h"
#include "semphr.h"

extern SemaphoreHandle_t g_staging_lock_handle;

static inline void staging_lock_init(void) {
#if CFG_LOCK_MODE == 0
    /* Mode A: Binary semaphore WITHOUT priority inheritance */
    g_staging_lock_handle = xSemaphoreCreateBinary();
    xSemaphoreGive(g_staging_lock_handle);
#else
    /* Mode B & C: Mutex WITH priority inheritance */
    g_staging_lock_handle = xSemaphoreCreateMutex();
#endif
}

static inline void staging_lock_acquire(void) {
    if (g_staging_lock_handle) {
        xSemaphoreTake(g_staging_lock_handle, portMAX_DELAY);
    }
}

static inline void staging_lock_release(void) {
    if (g_staging_lock_handle) {
        xSemaphoreGive(g_staging_lock_handle);
    }
}

#else

/* Host/simulated fallback */
static inline void staging_lock_init(void) {}
static inline void staging_lock_acquire(void) {}
static inline void staging_lock_release(void) {}

#endif

#endif /* STAGING_LOCK_H */
