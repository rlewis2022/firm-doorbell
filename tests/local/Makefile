# Local tier (FIRM): compile every logic source and every test with the PC compiler, then run.
# If logic code includes hardware headers it won't compile here - the second half of absolute A-02.
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Werror -O1
ROOT     := ../..
LOGIC    := $(shell find $(ROOT)/firmware -path '*/logic/*' -name '*.cpp' 2>/dev/null)
TESTS    := $(wildcard *.cpp) $(wildcard locked/*.cpp) $(wildcard unlocked/*.cpp)
INCLUDES := -I. -I$(ROOT)/firmware

.PHONY: test clean
test: build/run_tests
	./build/run_tests

build/run_tests: $(TESTS) $(LOGIC)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TESTS) $(LOGIC) -o $@

clean:
	rm -rf build
