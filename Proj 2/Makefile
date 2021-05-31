CFLAGS += -Wall -Wextra -pedantic -static -Wno-unused-parameter -I. -std=c99
CC ?= clang
BIN ?= appserver
TEST_BIN ?= test

DEPS := server.o Bank.o chan.o queue.o process.o strutil.o

.PHONY: all build debug fine fine-debug coarse coarse-debug clean test valgrind
all: build

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

build: $(DEPS)
	$(CC) -o $(BIN) $(DEPS) $(CFLAGS) -lpthread

debug: CFLAGS += -DDEBUG -Werror -Og -g
debug: build

fine: build

fine-debug: debug

coarse: CFLAGS += -DCOARSE_LOCK
coarse: BIN := $(BIN)-coarse
coarse: build

coarse-debug: CFLAGS += -DDEBUG -Werror -Og -g
coarse-debug: coarse

clean:
	@rm -f *.o

test:
	$(CC) -o $(TEST_BIN) Project2Test.c $(CFLAGS) -lpthread

valgrind:
	@valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./$(BIN) 2 4 valgrind.test.out
