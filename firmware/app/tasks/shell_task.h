#ifndef SHELL_TASK_H
#define SHELL_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

void shell_task_start(void);
void shell_task_code(void *params);

/**
 * @brief Thread-safe formatted print to the active UART console.
 */
void shell_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* SHELL_TASK_H */
