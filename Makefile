TARGET=keyuser

# Directories
SRC_DIR := src
OBJ_DIR := obj

SRCS         := $(wildcard $(SRC_DIR)/*.cpp)
OBJS         := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEBUG_OBJS   := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/debug_%.o,$(SRCS))

# Compilation variables
COMMON_FLAGS  := -std=c++20 -Wall -Wextra -pedantic
RELEASE_FLAGS := $(COMMON_FLAGS) -O2 -pipe -flto=auto -march=native -mtune=native
DEBUG_FLAGS   := $(COMMON_FLAGS) -Og -g3 -fsanitize=address,undefined

all: $(OBJ_DIR) $(TARGET) 

$(OBJ_DIR)/debug_%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS)  -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p obj/

generator-debug: $(DEBUG_OBJS)
	$(CXX) $(DEBUG_FLAGS) -o $@ $^

generator: $(OBJS)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -o $@ $^

clean:
	rm -vf $(TARGET) \
		$(OBJS) $(DEBUG_OBJS)

.PHONY: all clean tests
