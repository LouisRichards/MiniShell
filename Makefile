# Configuration

CC		= gcc
LD		= gcc
AR		= ar
CFLAGS		= -g -std=gnu99 -Wall -Iinclude -fPIC
LDFLAGS		= -Llib
LIBS		= -lm
ARFLAGS		= rcs

# Variables

LIB_HDRS	= $(wildcard include/fs/*.h)
LIB_SRCS	= $(wildcard src/fs/*.c)
LIB_OBJS	= $(LIB_SRCS:.c=.o)
LIBRARY	= lib/libsfs.a

SRCS	= $(wildcard src/*.c)
OBJS	= $(SRCS:.c=.o)

SHELL	= bin/shell

TEST_SRCS   = $(wildcard src/tests/*.c)
TEST_OBJS   = $(TEST_SRCS:.c=.o)
UNIT_TESTS	= $(patsubst src/tests/%,bin/%,$(patsubst %.c,%,$(wildcard src/tests/unit_*.c)))

# Rules

all:		$(LIBRARY) $(UNIT_TESTS) $(SHELL)

%.o:		%.c $(LIB_HDRS)
	@echo "Compiling $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(LIBRARY):	$(LIB_OBJS)
	@echo "Linking   $@"
	@$(AR) $(ARFLAGS) $@ $^

$(SHELL):	$(OBJS) $(LIBRARY)
	@echo "Linking   $@"
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

bin/unit_%:	src/tests/unit_%.o $(LIBRARY)
	@echo "Linking   $@"
	@$(LD) $(LDFLAGS) -o $@ $^

test-unit:	$(UNIT_TESTS)
	@for test in bin/unit_*; do 		\
	    for i in $$(seq 0 $$($$test 2>&1 | tail -n 1 | awk '{print $$1}')); do \
		echo "Running   $$(basename $$test) $$i";		\
		valgrind $$test $$i > test.log 2>&1;	\
		grep -q 'ERROR SUMMARY: 0' test.log || cat test.log;	\
		! grep -q 'Assertion' test.log || cat test.log; 	\
	    done				\
	done

test-shell:	$(SHELL)
	@for test in bin/test_*.sh; do		\
	    $$test;				\
	done

test:	test-unit test-shell

clean:
	@echo "Removing  objects"
	@rm -f $(LIB_OBJS) $(OBJS) $(TEST_OBJS)

	@echo "Removing  libraries"
	@rm -f $(LIBRARY)

	@echo "Removing  programs"
	@rm -f $(SHELL)

	@echo "Removing  tests"
	@rm -f $(UNIT_TESTS) test.log

.PRECIOUS: %.o
