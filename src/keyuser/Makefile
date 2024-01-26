TARGET=keyuser

# Directories
SRC_DIR := src
OBJ_DIR := obj

SRCS         := $(wildcard $(SRC_DIR)/*.cpp)
OBJS         := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEBUG_OBJS   := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/debug_%.o,$(SRCS))

# ABSL
ABSL_DIR :=/home/renato/abseil
ABSL_LIB := $(ABSL_DIR)/lib
ABSL_INC := $(ABSL_DIR)/include
ABSL_LIB_FILES :=-labsl_hash -labsl_city -labsl_low_level_hash

# Compilation variables
COMMON_FLAGS  := -std=c++20 -Wall -Wextra -pedantic -I$(ABSL_INC) -L$(ABSL_LIB)
RELEASE_FLAGS := $(COMMON_FLAGS) -O2 -pipe -flto=auto -march=native
DEBUG_FLAGS   := $(COMMON_FLAGS) -Og -g3 -fsanitize=address,undefined -mbmi2 -msse2 -msse3

all: $(TARGET) 

$(OBJ_DIR)/debug_%.o: $(SRC_DIR)/%.cpp $(OBJ_DIR) Makefile
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS)  -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(OBJ_DIR) Makefile
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -c $< -o $@

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
