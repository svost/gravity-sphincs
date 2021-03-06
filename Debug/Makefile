SRC=$(wildcard *.c) $(wildcard primitives/*.c)
OBJ=$(patsubst %.c, %.o, $(SRC))
IVSFILE=IntermediateValues.txt
BIN=gen_ivs


CFLAGS_COMMON=-std=c99 -pedantic -Wall -Wextra -Wno-long-long -march=native

# Some of sample configurations from https://eprint.iacr.org/2017/933.pdf

# small
VERSION_S= -DPORS_k=24 -DMERKLE_h=5  -DGRAVITY_d=1  -DGRAVITY_c=10

# batched

VERSION_B= -DPORS_k=32 -DMERKLE_h=8  -DGRAVITY_d=3  -DGRAVITY_c=16

# fast
VERSION_F= -DPORS_k=32 -DMERKLE_h=5  -DGRAVITY_d=7  -DGRAVITY_c=15

# NIST-fast
VERSION_NF= -DPORS_k=28 -DMERKLE_h=5  -DGRAVITY_d=10 -DGRAVITY_c=14

# NIST
VERSION_N= -DPORS_k=28 -DMERKLE_h=8  -DGRAVITY_d=6 -DGRAVITY_c=16

# NIST-slow
VERSION_NS= -DPORS_k=28 -DMERKLE_h=14  -DGRAVITY_d=4 -DGRAVITY_c=8

VERSION=$(VERSION_NS)

CFLAGS_COMMON+=$(VERSION)
CFLAGS=$(CFLAGS_COMMON) -O3 -fomit-frame-pointer
CFLAGS_SANITIZED=$(CFLAGS_COMMON) -fsanitize=address,undefined -g

KERNEL_NAME := $(shell uname -s)

all: help

help:
			@echo "Please choose a target:"
			@echo "\tanalyze\t\t runs static analyzers"
			@echo "\tcheck\t\t verifies intermediate values"
			@echo "\tclean\t\t cleans up"
			@echo "\tformat\t\t formats the code using .clang-format rules"
			@echo "\tivs\t\t generates intermediate values"

check:		$(SRC) $(IVSFILE)
			@$(CC) $(CFLAGS) $(LDFLAGS) -DDEBUG $(SRC) -o $(BIN)
			@./gen_ivs > $(IVSFILE)_tmp
			@diff $(IVSFILE) $(IVSFILE)_tmp ; if [ $$? -ne 0 ] ; then echo "error" ; else echo "ok" ; fi
			@rm -f $(IVSFILE)_tmp

ivs:		$(SRC) gen_ivs.c
			$(CC) $(CFLAGS) $(LDFLAGS) -DDEBUG $^ -o $(BIN)
			./$(BIN) > $(IVSFILE)

format:
			clang-format -i *.c *.h

analyze:
			cppcheck . --std=c89
			scan-build gcc -c $(CFLAGS_SANITIZED) *.c
			cppclean .
			rm -f *.o

cloc:
			cloc $(wildcard *.c) $(wildcard *.h)

clean:
			rm -f $(BIN) *.dSYM *.o

.PHONY: clean format analyze cloc

