# Compilation variables
COMMON_FLAGS  := -std=c18 -Wall -Wextra -pedantic -pipe -lm
RELEASE_FLAGS := $(COMMON_FLAGS) -O2 -flto=auto -march=native
DEBUG_FLAGS   := $(COMMON_FLAGS) -Og -g3 -fsanitize=address,undefined

all: keybuilder Makefile

keybuilder-debug: src/main.c Makefile
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o $@ $<

keybuilder: src/main.c Makefile
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o $@ $<

clean:
	rm -vf keybuilder keybuilder-debug

.PHONY: all clean
