#!make

-include .env

URL ?= "http://localhost:5001"

# override default variable, bt leave it alone if set in environment
ifeq ($(origin CC),default)
CC = gcc
endif

CFLAGS ?= -O0 -ggdb3
CFLAGS += -std=c17
CFLAGS += -Wall -Wextra -Wshadow -Wpedantic -Wuninitialized -Wformat=2 -Wno-unknown-pragmas
#CFLAGS += -Werror
CFLAGS += -fstack-protector-strong

# gcc only
ifneq "$(findstring gcc,$(CC))" ""
CFLAGS += -Wformat-overflow=2 -Wformat-truncation=2
CFLAGS += -ftree-vrp -fanalyzer
endif
#
# clang only
ifneq "$(findstring clang,$(CC))" ""
CFLAGS += -Weverything
CFLAGS += -Wno-declaration-after-statement -Wno-disabled-macro-expansion
endif

SOURCES := json.c
OBJS := $(SOURCES:%.c=%.o)

# default target
.PHONY: all
all: json.o

# dependencies
json.c: json.h
test.c: json.h json.c

%.o: %.c %.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

.PHONY: clean
clean:
	-rm -f $(OBJS)

.PHONY: test
test: test.o json.o
	$(CC) -o $@ json.o $<

