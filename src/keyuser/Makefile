TARGET=keyuser

# Directories
SRC_DIR := src
OBJ_DIR := obj

# Find all .cpp and .c files in the project
SRCS := $(shell find . \( -name "*.cpp" -o -name "*.c" \) -printf "%P\n")
OBJS := $(shell find . \( -name "*.cpp" -o -name "*.c" \) -exec basename {} \;)
OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(patsubst %.c,$(OBJ_DIR)/%.o,$(OBJS)))
DEBUG_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/debug_%.o,$(patsubst %.c,$(OBJ_DIR)/debug_%.o,$(SRCS)))

# Make, please look into subdirectories :)
vpath %.cpp $(shell find $(SRC_DIR) -type d)
vpath %.c $(shell find $(SRC_DIR) -type d)

# ABSL
ABSL_DIR := $(shell realpath ../../deps/abseil)
ABSL_LIB := $(ABSL_DIR)/lib
ABSL_INC := $(ABSL_DIR)/include
ABSL_LIB_FILES :=-labsl_hash -labsl_city -labsl_low_level_hash

ifeq ($(PLATFORM),x86_64)
	INTRINSICS=-mbmi2 -msse2 -msse3
endif

ifeq ($(PLATFORM),aarch64)
	INTRINSICS=-march=armv8-a+crypto
endif

ifdef RQ8
    CXXFLAGS += -DRQ8=$(RQ8)
    CFLAGS   += -DRQ8=$(RQ8)
endif

# Compilation variables
CXXFLAGS += -std=c++20 
COMMON_FLAGS  := -Wall -Wextra -pedantic -I$(ABSL_INC) -L$(ABSL_LIB) -Igperf_hashes/
RELEASE_FLAGS := $(COMMON_FLAGS) -O2 -pipe -flto=auto -march=native $(INTRINSICS)
DEBUG_FLAGS   := $(COMMON_FLAGS) -Og -g3 -fsanitize=address,undefined $(INTRINSICS)

all: $(TARGET)

# CXX objects
$(OBJ_DIR)/debug_%.o: %.cpp $(OBJ_DIR) Makefile
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS)  -c $< -o $@
$(OBJ_DIR)/%.o: %.cpp $(OBJ_DIR) Makefile
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -c $< -o $@

# CC objects
$(OBJ_DIR)/debug_%.o: %.c $(OBJ_DIR) Makefile
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@
$(OBJ_DIR)/%.o: %.c $(OBJ_DIR) Makefile
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p obj/

keyuser-debug: $(DEBUG_OBJS)
	$(CXX)  $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^ $(ABSL_LIB_FILES)

keyuser: $(OBJS)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -o $@ $^ $(ABSL_LIB_FILES)

clean:
	rm -vf keyuser keyuser-debug  \
		$(DEBUG_OBJS) \
		$(OBJS)

.PHONY: all clean tests
