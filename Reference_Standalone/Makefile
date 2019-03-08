OBJ=$(patsubst %.c, %.o, $(SRC))

CFLAGS_COMMON=-std=c99 -pedantic -Wall -Wextra -Wno-long-long -march=native
LDFLAGS=

VERSION=$(VERSION_NS)

CFLAGS_COMMON+=$(VERSION)
CFLAGS=$(CFLAGS_COMMON) -O3 -fomit-frame-pointer
CFLAGS_SANITIZED=$(CFLAGS_COMMON) -fsanitize=address,undefined -g

all: help

help:
			@echo "Please choose a target:"
			@echo "\tanalyze\t\t runs static analyzers"
			@echo "\tharakasum\t\t file hashsum utility"

harakasum:		harakasum.c primitives/hash.c primitives/haraka.c primitives/kiss99.c
			$(CC) $(CFLAGS) $(LDFLAGS) $^ -o harakasum

format:
			clang-format -i *.c *.h

analyze:
			cppcheck . --std=c99
			scan-build gcc -c $(CFLAGS_SANITIZED) *.c
			cppclean .
			rm -f *.o

cloc:
			cloc $(wildcard *.c) $(wildcard *.h)

clean:
			rm -f harakasum *.dSYM *.o

.PHONY: clean format analyze cloc harakasum

