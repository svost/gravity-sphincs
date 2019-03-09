OBJ=$(patsubst %.c, %.o, $(SRC))

CFLAGS_COMMON=-std=c99 -pedantic -Wall -Wextra -Wformat -Wno-long-long -march=native

CC_WIN=x86_64-w64-mingw32-gcc

LDFLAGS=

VERSION=$(VERSION_NS)

CFLAGS_COMMON+=$(VERSION)
CFLAGS=$(CFLAGS_COMMON) -O3 -fomit-frame-pointer
CFLAGS_WIN=$(CFLAGS_COMMON) -O3 -fomit-frame-pointer -static -DWIN32=1
CFLAGS_SANITIZED=$(CFLAGS_COMMON) -fsanitize=address,undefined -g

all: help

help:
			@echo "Please choose a target:"
			@echo "\tanalyze\t\t runs static analyzers"
			@echo "\tharakasum\t\t file hashsum utility"

harakasum:		harakasum.c primitives/hash.c primitives/haraka.c primitives/kiss99.c
			$(CC) $(CFLAGS) $(LDFLAGS) $^ -o harakasum

harakasumw64:		harakasum.c primitives/hash.c primitives/haraka.c primitives/kiss99.c
			$(CC_WIN) $(CFLAGS_WIN) $(LDFLAGS) $^ -o harakasum.exe
			x86_64-w64-mingw32-strip harakasum.exe

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
			@ -rm -rf harakasum harakasum.exe *.dSYM *.o

.PHONY: clean format analyze cloc harakasum harakasumw64

