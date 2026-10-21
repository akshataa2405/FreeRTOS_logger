CC ?= gcc
CFLAGS = -Wall -Wextra -std=c11 -I. -I../../firmware/app/common -I../../firmware/app/tasks

SRCS = test_runner.c \
       unity.c \
       test_crc32.c \
       test_record_format.c \
       test_block_builder.c \
       test_shell_parser.c \
       ../../firmware/app/common/crc32.c \
       ../../firmware/app/common/block_builder.c \
       ../../firmware/app/tasks/shell_parser.c

TARGET = run_tests

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
