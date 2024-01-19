TARGET=keysynth

# Compilation variables
COMMON_FLAGS  := -std=c++20 -Wall -Wextra -pedantic
RELEASE_FLAGS := $(COMMON_FLAGS) -O2 -pipe -flto=auto -march=native
DEBUG_FLAGS   := $(COMMON_FLAGS) -Og -g3 -fsanitize=address,undefined -mbmi2 -msse2 -msse3

all: $(TARGET) 

keysynth-debug: src/main.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

keysynth: src/main.cpp
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -o $@ $^

clean:
	rm -vf keysynth keysynth-debug

.PHONY: all clean
