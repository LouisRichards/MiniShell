# Configuration


CC = gcc

CFLAGS = -g -Wall -Iinclude

# Variables

LIB_HDRS = $(wildcard include/*.h)
FS_HDRS = $(wildcard include/fs/*.h)

LIB_SRCS = $(wildcard src/*.c)
FS_SRCS = $(wildcard src/fs/*.c)

LIB_OBJS = $(LIB_SRCS:.c=.o)
FS_OBJS = $(FS_SRCS:.c=.o)

SRCS = $(LIB_SRCS) $(FS_SRCS)

TEST_HDRS = $(wildcard include/test/*.h)
TEST_SRCS = $(wildcard src/test/*.c)
TEST_OBJS = $(TEST_SRCS:.c=.o)
UNIT_TESTS = $(patsubst src/test/%,bin/test/%,$(patsubst %.c,%,$(wildcard src/test/*_test.c)))

# Rules

bin/test/%: src/test/%.c $(LIB_OBJS) $(FS_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

test: $(UNIT_TESTS)
	@echo "Running unit tests..."
	@for test in bin/test/*_test; do \
		echo "Running $$test..."; \
		$$test; \
	done

%.o: %.c $(LIB_HDRS) $(FS_HDRS)

clean:
	@echo "Cleaning up..."
	@rm -f $(LIB_OBJS) $(FS_OBJS) $(TEST_OBJS) $(UNIT_TESTS) test test.log
	@rm -rf bin/test/*.dSYM
