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

all: $(UNIT_TESTS)

bin/test/%_test: src/test/%_test.c 
	@mkdir -p bin
	@mkdir -p bin/test
	@echo "Compiling $<..."
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

# Screen
# ------
#  Here we just compile the screen program linking utils.o and screen.o

screen: src/screen/screen.c
	$(CC) $(CFLAGS) -o bin/screen src/screen/screen.c src/utils.c
