TARGET=keyuser

# Directories
SRC_DIR := src
OBJ_DIR := obj

SRCS         := $(wildcard $(SRC_DIR)/*.cpp)
OBJS         := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEBUG_OBJS   := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/debug_%.o,$(SRCS))

# ABSL
# git clone https://github.com/abseil/abseil-cpp.git
# cd abseil-cpp/
# mkdir build && cd build
# cmake -DABSL_BUILD_TESTING=ON -DABSL_USE_GOOGLETEST_HEAD=ON -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=/path/to/your/abseil ..
# cmake --build . --target install
#ABSL_DIR := /path/to/your/abseil
#ABSL_LIB := $(ABSL_DIR)/lib
#ABSL_INC := $(ABSL_DIR)/include
#ABSL_LIB_FILES := $(shell find $(ABSL_LIB) -iname "*.a" -exec basename -s .a {} \;)
#ABSL_LIB_FILES := $(patsubst lib%, -l%, $(ABSL_LIB_FILES))

# Compilation variables
COMMON_FLAGS  := -std=c++20 -Wall -Wextra -pedantic
RELEASE_FLAGS := $(COMMON_FLAGS) -O2 -pipe -flto=auto -march=native
DEBUG_FLAGS   := $(COMMON_FLAGS) -Og -g3 -fsanitize=address,undefined -mbmi2 -msse2 -msse3 -mavx -mavx2

all: $(TARGET) 

$(OBJ_DIR)/debug_%.o: $(SRC_DIR)/%.cpp $(OBJ_DIR) Makefile
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS)  -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(OBJ_DIR) Makefile
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p obj/

keyuser-debug: $(DEBUG_OBJS)
	$(CXX)  $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

keyuser: $(OBJS)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -o $@ $^

# temp: src/absl.cpp
# 	$(CXX) -I$(ABSL_INC) -L$(ABSL_LIB) $(CXXFLAGS) $(RELEASE_FLAGS) -o $@ $^ $(ABSL_LIB_FILES) 

clean:
	rm -vf keyuser keyuser-debug  \
		$(DEBUG_OBJS) \
		$(OBJS)

.PHONY: all clean tests
