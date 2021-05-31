CFLAGS += -Wall -Werror -Wextra -pedantic -Wno-unused-parameter -static -I. -L.
CC ?= clang
BIN ?= shell

DEPS := strutil.o builtin.o child.o

.PHONY: build all clean valgrind
all: build

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

build: main.o $(DEPS)
	$(CC) -o $(BIN) main.o $(DEPS) $(CFLAGS)

clean:
	@rm -f *.o

valgrind:
	@valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./shell
