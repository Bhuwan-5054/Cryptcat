CC = gcc
CFLAGS = -I../src/include -Iframeworks -I.. -Wall -Wextra -O2 -g -std=c11
LDFLAGS = -lssl -lcrypto -lpthread

SOURCES = \
	frameworks/test_runner.c \
	frameworks/test_main.c \
	unit/test_crypto.c \
	integration/test_end_to_end.c \
	performance/benchmark_crypto.c \
	../src/core/crypto_engine.c \
	../src/platform/unix_network.c \
	../src/platform/os_utils.c \
	../src/utils/logger.c \
	../src/utils/error_handler.c \
	../src/utils/memory_utils.c \
	../src/utils/string_utils.c \
	../src/utils/hex_utils.c

TARGET = run_tests

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o */*.o

.PHONY: all clean
