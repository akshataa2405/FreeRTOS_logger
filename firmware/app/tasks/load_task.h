#ifndef LOAD_TASK_H
#define LOAD_TASK_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void load_task_start(void);
void load_task_code(void *params);
void load_task_enable(bool enable);
bool load_task_is_enabled(void);

#ifdef __cplusplus
}
#endif

#endif /* LOAD_TASK_H */
