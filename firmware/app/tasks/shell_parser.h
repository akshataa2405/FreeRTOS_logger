#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SHELL_CMD_UNKNOWN = 0,
    SHELL_CMD_HELP,
    SHELL_CMD_STATUS,
    SHELL_CMD_STATS,
    SHELL_CMD_RATE,
    SHELL_CMD_START,
    SHELL_CMD_STOP,
    SHELL_CMD_LS,
    SHELL_CMD_DUMP,
    SHELL_CMD_FAULT,
    SHELL_CMD_WDTEST,
    SHELL_CMD_RESET,
    SHELL_CMD_VERSION
} ShellCmdType_t;

typedef struct {
    ShellCmdType_t type;
    char           arg1[32];
    char           arg2[32];
    int            argc;
} ShellCommand_t;

/**
 * @brief Parses an incoming ASCII line into a structured command.
 *
 * @param line Input null-terminated string (may have \r\n)
 * @param cmd Output parsed command struct
 * @return true if parsed, false if line is empty or whitespace
 */
bool shell_parse_line(const char *line, ShellCommand_t *cmd);

#ifdef __cplusplus
}
#endif

#endif /* SHELL_PARSER_H */
