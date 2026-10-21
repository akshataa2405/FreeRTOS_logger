#include "unity.h"
#include "shell_parser.h"

void test_shell_parser_basic(void) {
    ShellCommand_t cmd;

    TEST_ASSERT_TRUE(shell_parse_line("status\r\n", &cmd));
    TEST_ASSERT_EQUAL_INT(SHELL_CMD_STATUS, cmd.type);
    TEST_ASSERT_EQUAL_INT(0, cmd.argc);

    TEST_ASSERT_TRUE(shell_parse_line("rate 200\n", &cmd));
    TEST_ASSERT_EQUAL_INT(SHELL_CMD_RATE, cmd.type);
    TEST_ASSERT_EQUAL_INT(1, cmd.argc);
    TEST_ASSERT_TRUE(strcmp("200", cmd.arg1) == 0);

    TEST_ASSERT_TRUE(shell_parse_line("dump log_0001.bin", &cmd));
    TEST_ASSERT_EQUAL_INT(SHELL_CMD_DUMP, cmd.type);
    TEST_ASSERT_EQUAL_INT(1, cmd.argc);
    TEST_ASSERT_TRUE(strcmp("log_0001.bin", cmd.arg1) == 0);

    TEST_ASSERT_TRUE(shell_parse_line("fault null", &cmd));
    TEST_ASSERT_EQUAL_INT(SHELL_CMD_FAULT, cmd.type);
    TEST_ASSERT_EQUAL_INT(1, cmd.argc);
    TEST_ASSERT_TRUE(strcmp("null", cmd.arg1) == 0);
}

void test_shell_parser_empty_and_unknown(void) {
    ShellCommand_t cmd;

    TEST_ASSERT_FALSE(shell_parse_line("", &cmd));
    TEST_ASSERT_FALSE(shell_parse_line("   \r\n", &cmd));

    TEST_ASSERT_TRUE(shell_parse_line("invalid_cmd foo bar", &cmd));
    TEST_ASSERT_EQUAL_INT(SHELL_CMD_UNKNOWN, cmd.type);
}
