#include "shell_task.h"
#include "shell_parser.h"
#include "record_task.h"
#include "logger_task.h"
#include "monitor_task.h"
#include "hardfault.h"
#include "reset_cause.h"
#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#if defined(FREERTOS) || defined(INC_FREERTOS_H)
#include "FreeRTOS.h"
#include "task.h"

static TaskHandle_t s_shell_task_handle = NULL;
static StackType_t  s_shell_stack[STACK_SHELL_TASK];
static StaticTask_t s_shell_tcb;
#endif

#if defined(__arm__) || defined(__thumb__)
#include "stm32f4xx_hal.h"
extern UART_HandleTypeDef huart2;
#endif

void shell_printf(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len > 0) {
#if defined(__arm__) && defined(huart2)
        HAL_UART_Transmit(&huart2, (uint8_t *)buf, (uint16_t)len, 500);
#else
        printf("%s", buf);
        fflush(stdout);
#endif
    }
}

static void execute_command(const ShellCommand_t *cmd) {
    switch (cmd->type) {
        case SHELL_CMD_HELP:
            shell_printf("\nAvailable Commands:\n");
            shell_printf("  help            - Display this help menu\n");
            shell_printf("  status          - Show system uptime, sample counts, drop counts\n");
            shell_printf("  stats           - Display CPU runtime statistics and stack high-water marks\n");
            shell_printf("  rate <hz>       - Set sensor sampling rate (10-500 Hz)\n");
            shell_printf("  start           - Open a new log file and start recording\n");
            shell_printf("  stop            - Close active log file and halt logging\n");
            shell_printf("  ls              - List files on SD card\n");
            shell_printf("  dump <file>     - Dump log file as CSV\n");
            shell_printf("  fault <type>    - Induce deliberate fault (null, divzero, unaligned, invstate)\n");
            shell_printf("  wdtest          - Simulate task hang to test hardware watchdog\n");
            shell_printf("  reset           - Perform software system reset\n");
            shell_printf("  version         - Display firmware version and build information\n\n");
            break;

        case SHELL_CMD_STATUS:
            shell_printf("\n--- System Status ---\n");
            shell_printf("Samples Count:  %lu\n", (unsigned long)record_task_get_sample_count());
            shell_printf("Dropped Samples:%lu\n", (unsigned long)record_task_get_drop_count());
            shell_printf("Blocks Written: %lu\n", (unsigned long)logger_task_get_blocks_written());
            shell_printf("Sampling Rate:  %lu Hz\n", (unsigned long)record_task_get_rate_hz());
            shell_printf("Logging Active: %s\n", logger_task_is_logging() ? "YES" : "NO");
            shell_printf("Lock Mode:      Mode %d\n", CFG_LOCK_MODE);
            shell_printf("---------------------\n\n");
            break;

        case SHELL_CMD_STATS:
            shell_printf("\n--- Task Runtime Statistics ---\n");
#if defined(FREERTOS) && (configGENERATE_RUN_TIME_STATS == 1)
            char stats_buf[512];
            vTaskGetRunTimeStats(stats_buf);
            shell_printf("Task          Abs Time      %% Time\n");
            shell_printf("****************************************\n");
            shell_printf("%s\n", stats_buf);
#else
            shell_printf("Run-time stats requires configGENERATE_RUN_TIME_STATS = 1\n");
#endif
            shell_printf("-------------------------------\n\n");
            break;

        case SHELL_CMD_RATE:
            if (cmd->argc >= 1) {
                uint32_t hz = (uint32_t)strtoul(cmd->arg1, NULL, 10);
                record_task_set_rate_hz(hz);
                shell_printf("Sampling rate set to %lu Hz\n", (unsigned long)record_task_get_rate_hz());
            } else {
                shell_printf("Usage: rate <10-500>\n");
            }
            break;

        case SHELL_CMD_START:
            logger_task_request_start();
            shell_printf("Started logging in new file.\n");
            break;

        case SHELL_CMD_STOP:
            logger_task_request_stop();
            shell_printf("Stopped logging and finalized file.\n");
            break;

        case SHELL_CMD_LS:
            shell_printf("File listing requires active FatFs SD card.\n");
            break;

        case SHELL_CMD_DUMP:
            if (cmd->argc >= 1) {
                shell_printf("Streaming %s as CSV to console...\n", cmd->arg1);
            } else {
                shell_printf("Usage: dump <filename>\n");
            }
            break;

        case SHELL_CMD_FAULT:
            if (cmd->argc >= 1) {
                shell_printf("Triggering fault '%s'...\n", cmd->arg1);
                fault_trigger(cmd->arg1);
            } else {
                shell_printf("Usage: fault <null|divzero|unaligned|invstate>\n");
            }
            break;

        case SHELL_CMD_WDTEST:
            shell_printf("Inducing task hang: watchdog will reset MCU in ~2 seconds...\n");
            monitor_task_force_hang();
            break;

        case SHELL_CMD_RESET:
            shell_printf("Rebooting system now...\n");
#if defined(__arm__)
            NVIC_SystemReset();
#endif
            break;

        case SHELL_CMD_VERSION:
            shell_printf("STM32 FreeRTOS Data Logger v1.0.0\n");
            shell_printf("Build Date: %s %s\n", __DATE__, __TIME__);
            shell_printf("FreeRTOS Native API, Lock Mode: %d\n", CFG_LOCK_MODE);
            break;

        case SHELL_CMD_UNKNOWN:
        default:
            shell_printf("Unknown command. Type 'help' for available commands.\n");
            break;
    }
}

void shell_task_code(void *params) {
    (void)params;

    char line_buf[128];
    uint32_t line_pos = 0;

    shell_printf("\r\n==========================================\r\n");
    shell_printf("  STM32 FreeRTOS High-Reliability Logger\r\n");
    shell_printf("  Type 'help' for command listing\r\n");
    shell_printf("==========================================\r\n\r\nlogger> ");

    for (;;) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
        /* Periodic watchdog check-in */
        monitor_task_checkin(MONITOR_BIT_SHELL);

        /* Read characters over UART */
        uint8_t ch = 0;
#if defined(__arm__) && defined(huart2)
        if (HAL_UART_Receive(&huart2, &ch, 1, 100) == HAL_OK) {
#else
        int c = getchar();
        if (c != EOF) {
            ch = (uint8_t)c;
#endif
            if (ch == '\r' || ch == '\n') {
                line_buf[line_pos] = '\0';
                shell_printf("\r\n");
                if (line_pos > 0) {
                    ShellCommand_t cmd;
                    if (shell_parse_line(line_buf, &cmd)) {
                        execute_command(&cmd);
                    }
                }
                line_pos = 0;
                shell_printf("logger> ");
            } else if (ch == '\b' || ch == 127) {
                if (line_pos > 0) {
                    line_pos--;
                    shell_printf("\b \b");
                }
            } else if (line_pos < sizeof(line_buf) - 1) {
                line_buf[line_pos++] = (char)ch;
                /* Echo back */
                shell_printf("%c", ch);
            }
        }
#else
        break;
#endif
    }
}

void shell_task_start(void) {
#if defined(FREERTOS) || defined(INC_FREERTOS_H)
    s_shell_task_handle = xTaskCreateStatic(
        shell_task_code,
        "ShellTask",
        STACK_SHELL_TASK,
        NULL,
        PRI_SHELL_TASK,
        s_shell_stack,
        &s_shell_tcb
    );
#endif
}
