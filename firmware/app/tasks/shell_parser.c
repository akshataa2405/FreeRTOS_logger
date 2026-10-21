#include "shell_parser.h"
#include <string.h>
#include <ctype.h>


bool shell_parse_line(const char *line, ShellCommand_t *cmd) {
    if (!line || !cmd) return false;

    memset(cmd, 0, sizeof(ShellCommand_t));

    char buf[128];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    /* Strip leading/trailing whitespaces and newlines */
    char *p = buf;
    while (isspace((unsigned char)*p)) p++;
    if (*p == '\0') return false;

    char *token = strtok(p, " \t\r\n");
    if (!token) return false;

    /* Match command token */
    if (strcmp(token, "help") == 0) {
        cmd->type = SHELL_CMD_HELP;
    } else if (strcmp(token, "status") == 0) {
        cmd->type = SHELL_CMD_STATUS;
    } else if (strcmp(token, "stats") == 0) {
        cmd->type = SHELL_CMD_STATS;
    } else if (strcmp(token, "rate") == 0) {
        cmd->type = SHELL_CMD_RATE;
    } else if (strcmp(token, "start") == 0) {
        cmd->type = SHELL_CMD_START;
    } else if (strcmp(token, "stop") == 0) {
        cmd->type = SHELL_CMD_STOP;
    } else if (strcmp(token, "ls") == 0) {
        cmd->type = SHELL_CMD_LS;
    } else if (strcmp(token, "dump") == 0) {
        cmd->type = SHELL_CMD_DUMP;
    } else if (strcmp(token, "fault") == 0) {
        cmd->type = SHELL_CMD_FAULT;
    } else if (strcmp(token, "wdtest") == 0) {
        cmd->type = SHELL_CMD_WDTEST;
    } else if (strcmp(token, "reset") == 0) {
        cmd->type = SHELL_CMD_RESET;
    } else if (strcmp(token, "version") == 0) {
        cmd->type = SHELL_CMD_VERSION;
    } else {
        cmd->type = SHELL_CMD_UNKNOWN;
    }

    /* Extract arguments */
    token = strtok(NULL, " \t\r\n");
    if (token) {
        strncpy(cmd->arg1, token, sizeof(cmd->arg1) - 1);
        cmd->argc = 1;

        token = strtok(NULL, " \t\r\n");
        if (token) {
            strncpy(cmd->arg2, token, sizeof(cmd->arg2) - 1);
            cmd->argc = 2;
        }
    }

    return true;
}
