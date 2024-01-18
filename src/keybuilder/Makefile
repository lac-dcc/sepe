# Directories
SRC_DIR := src
OBJ_DIR := obj

SRCS         := $(wildcard $(SRC_DIR)/*.c)
OBJS         := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEBUG_OBJS   := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/debug_%.o,$(SRCS))

# Compilation variables
COMMON_FLAGS  := -std=c18 -Wall -Wextra -pedantic
RELEASE_FLAGS := $(COMMON_FLAGS) -O2 -pipe -flto=auto -march=native
DEBUG_FLAGS   := $(COMMON_FLAGS) -Og -g3 -fsanitize=address,undefined

all: keybuilder

$(OBJ_DIR)/debug_%.o: $(SRC_DIR)/%.c $(OBJ_DIR) Makefile
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR) Makefile
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p obj/

keybuilder-debug: $(DEBUG_OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o $@ $^

keybuilder: $(OBJS)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o $@ $^

clean:
	rm -vf keybuilder keybuilder-debug  \
		$(DEBUG_OBJS)                   \
		$(OBJS)

.PHONY: all clean tests
