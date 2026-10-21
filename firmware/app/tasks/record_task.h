#ifndef RECORD_TASK_H
#define RECORD_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "record_format.h"

#ifdef __cplusplus
extern "C" {
#endif

void record_task_start(void);
void record_task_code(void *params);

uint32_t record_task_get_sample_count(void);
uint32_t record_task_get_drop_count(void);
void     record_task_set_rate_hz(uint32_t rate_hz);
uint32_t record_task_get_rate_hz(void);

#ifdef __cplusplus
}
#endif

#endif /* RECORD_TASK_H */
