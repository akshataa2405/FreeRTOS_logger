#ifndef LOGGER_TASK_H
#define LOGGER_TASK_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void logger_task_start(void);
void logger_task_code(void *params);

void logger_task_request_start(void);
void logger_task_request_stop(void);
bool logger_task_is_logging(void);
uint32_t logger_task_get_blocks_written(void);

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_TASK_H */
