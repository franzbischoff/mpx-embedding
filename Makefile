# Makefile for Mpx Library (Matrix Profile)
# Project for time series analysis for embedded systems

# Compiler setup
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
DEBUG_FLAGS := -g -O0 -DDEBUG
INCLUDE_DIR := include

# Directories
SRC_DIR := src
TEST_DIR := tests
EXAMPLE_DIR := examples
BUILD_DIR := build

# Output directories
BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj

# Targets
TEST_TARGET := $(BIN_DIR)/test_mpx
EXAMPLE_TARGET := $(BIN_DIR)/example
DEBUG_BUFFERS_TARGET := $(BIN_DIR)/debug_buffers

# Source files
SRC_FILES := $(SRC_DIR)/Mpx.cpp
TEST_SRC := $(TEST_DIR)/test_mpx.cpp
EXAMPLE_SRC := $(EXAMPLE_DIR)/example.cpp
DEBUG_BUFFERS_SRC := $(EXAMPLE_DIR)/debug_buffers.cpp

# Object files
SRC_OBJ := $(OBJ_DIR)/Mpx.o
TEST_OBJ := $(OBJ_DIR)/test_mpx.o
EXAMPLE_OBJ := $(OBJ_DIR)/example.o
DEBUG_BUFFERS_OBJ := $(OBJ_DIR)/debug_buffers.o

# Phony targets
.PHONY: all test example debug-buffers clean debug help run-debug-buffers

# Default target
all: test example

# Build tests
test: $(TEST_TARGET)
	@echo "✓ Tests compiled successfully!"
	@echo "  Run with: make run-test"

$(TEST_TARGET): $(SRC_OBJ) $(TEST_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -o $@ $^

$(OBJ_DIR)/test_mpx.o: $(TEST_SRC) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

# Build example
example: $(EXAMPLE_TARGET)
	@echo "✓ Example compiled successfully!"
	@echo "  Run with: make run-example"

$(EXAMPLE_TARGET): $(SRC_OBJ) $(EXAMPLE_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -o $@ $^

$(OBJ_DIR)/example.o: $(EXAMPLE_SRC) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

# Build debug_buffers
debug-buffers: $(DEBUG_BUFFERS_TARGET)
	@echo "✓ Debug buffers compiled successfully!"
	@echo "  Run with: make run-debug-buffers"

$(DEBUG_BUFFERS_TARGET): $(SRC_OBJ) $(DEBUG_BUFFERS_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -o $@ $^

$(OBJ_DIR)/debug_buffers.o: $(DEBUG_BUFFERS_SRC) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

# Compile Mpx.cpp
$(SRC_OBJ): $(SRC_FILES) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

# Create directories if they don't exist
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Run tests
run-test: test
	@echo ""
	@echo "╔════════════════════════════════════╗"
	@echo "║    Running Unit Tests              ║"
	@echo "╚════════════════════════════════════╝"
	@echo ""
	@$(TEST_TARGET)

# Run example
run-example: example
	@echo ""
	@echo "╔════════════════════════════════════╗"
	@echo "║      Running Practical Example     ║"
	@echo "╚════════════════════════════════════╝"
	@echo ""
	@$(EXAMPLE_TARGET)

# Run debug buffers
run-debug-buffers: debug-buffers
	@echo ""
	@echo "╔════════════════════════════════════╗"
	@echo "║    Running Debug Buffer Inspector  ║"
	@echo "╚════════════════════════════════════╝"
	@echo ""
	@$(DEBUG_BUFFERS_TARGET)

# Build with debug
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean test example debug-buffers
	@echo "✓ Debug build completed!"

# Clean up
clean:
	@echo "Cleaning up temporary files..."
	rm -rf $(BUILD_DIR)
	@echo "✓ Clean up completed!"

# Information
help:
	@echo "╔════════════════════════════════════════════╗"
	@echo "║   Mpx Library - Makefile for C++17         ║"
	@echo "╚════════════════════════════════════════════╝"
	@echo ""
	@echo "Available targets:"
	@echo "  make all               - Compile tests and example (default)"
	@echo "  make test              - Compile tests only"
	@echo "  make example           - Compile example only"
	@echo "  make debug-buffers     - Compile debug buffer inspector"
	@echo "  make run-test          - Compile and run tests"
	@echo "  make run-example       - Compile and run example"
	@echo "  make run-debug-buffers - Compile and run debug buffer inspector"
	@echo "  make debug             - Compile with debug symbols"
	@echo "  make clean             - Remove compiled artifacts"
	@echo "  make help              - Show this message"
	@echo ""
	@echo "Project structure:"
	@echo "  include/mpx/       - Library headers"
	@echo "  src/               - C++ implementation"
	@echo "  tests/             - Unit tests"
	@echo "  examples/          - Usage examples"
	@echo "  build/             - Compilation artifacts"
	@echo ""

# Project info
print-info:
	@echo "Compiler: $(CXX)"
	@echo "C++ Standard: C++17"
	@echo "Flags: $(CXXFLAGS)"
