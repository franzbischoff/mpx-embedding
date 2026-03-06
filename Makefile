# Makefile for Mpx Library (Matrix Profile)
# Project for time series analysis for embedded systems

# Compiler setup
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
DEBUG_FLAGS := -g -O0 -DDEBUG
PROFILE_FLAGS := -pg -O2
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
TEST_ROBUSTNESS_TARGET := $(BIN_DIR)/test_mpx_robustness
TEST_GOLDEN_TARGET := $(BIN_DIR)/test_mpx_golden
GEN_GOLDEN_TARGET := $(BIN_DIR)/generate_golden_reference
EXAMPLE_TARGET := $(BIN_DIR)/example
DEBUG_BUFFERS_TARGET := $(BIN_DIR)/debug_buffers
BENCHMARK_PROFILE_TARGET := $(BIN_DIR)/benchmark_profile
BENCHMARK_USER_TARGET := $(BIN_DIR)/benchmark_user_config

# Source files
SRC_FILES := $(SRC_DIR)/Mpx.cpp
TEST_SRC := $(TEST_DIR)/test_mpx.cpp
TEST_ROBUSTNESS_SRC := $(TEST_DIR)/test_mpx_robustness.cpp
TEST_GOLDEN_SRC := $(TEST_DIR)/test_mpx_golden.cpp
GEN_GOLDEN_SRC := $(TEST_DIR)/generate_golden_reference.cpp
EXAMPLE_SRC := $(EXAMPLE_DIR)/example.cpp
DEBUG_BUFFERS_SRC := $(EXAMPLE_DIR)/debug_buffers.cpp
BENCHMARK_PROFILE_SRC := $(EXAMPLE_DIR)/benchmark_profile.cpp
BENCHMARK_USER_SRC := $(EXAMPLE_DIR)/benchmark_user_config.cpp

# Object files
SRC_OBJ := $(OBJ_DIR)/Mpx.o
TEST_OBJ := $(OBJ_DIR)/test_mpx.o
TEST_ROBUSTNESS_OBJ := $(OBJ_DIR)/test_mpx_robustness.o
TEST_GOLDEN_OBJ := $(OBJ_DIR)/test_mpx_golden.o
GEN_GOLDEN_OBJ := $(OBJ_DIR)/generate_golden_reference.o
EXAMPLE_OBJ := $(OBJ_DIR)/example.o
DEBUG_BUFFERS_OBJ := $(OBJ_DIR)/debug_buffers.o
BENCHMARK_PROFILE_OBJ := $(OBJ_DIR)/benchmark_profile.o
BENCHMARK_USER_OBJ := $(OBJ_DIR)/benchmark_user_config.o

# Phony targets
.PHONY: all test test-robustness test-golden gen-golden example debug-buffers benchmark-profile clean debug help profile run-debug-buffers run-test-robustness run-test-golden run-gen-golden run-profile

# Default target
all: test test-robustness test-golden example

# Build tests
test: $(TEST_TARGET)
	@echo "✓ Tests compiled successfully!"
	@echo "  Run with: make run-test"

$(TEST_TARGET): $(SRC_OBJ) $(TEST_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -o $@ $^

$(OBJ_DIR)/test_mpx.o: $(TEST_SRC) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

# Build robustness tests
test-robustness: $(TEST_ROBUSTNESS_TARGET)
	@echo "✓ Robustness tests compiled successfully!"
	@echo "  Run with: make run-test-robustness"

$(TEST_ROBUSTNESS_TARGET): $(SRC_OBJ) $(TEST_ROBUSTNESS_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -o $@ $^

$(OBJ_DIR)/test_mpx_robustness.o: $(TEST_ROBUSTNESS_SRC) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

# Build golden reference test
test-golden: $(TEST_GOLDEN_TARGET)
	@echo "✓ Golden reference test compiled successfully!"
	@echo "  Run with: make run-test-golden"

$(TEST_GOLDEN_TARGET): $(SRC_OBJ) $(TEST_GOLDEN_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -o $@ $^

$(OBJ_DIR)/test_mpx_golden.o: $(TEST_GOLDEN_SRC) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

# Build golden reference generator
gen-golden: $(GEN_GOLDEN_TARGET)
	@echo "✓ Golden reference generator compiled successfully!"
	@echo "  Run with: make run-gen-golden"

$(GEN_GOLDEN_TARGET): $(SRC_OBJ) $(GEN_GOLDEN_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -o $@ $^

$(OBJ_DIR)/generate_golden_reference.o: $(GEN_GOLDEN_SRC) | $(OBJ_DIR)
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

# Build benchmark_profile
benchmark-profile: $(BENCHMARK_PROFILE_TARGET)
	@echo "✓ Benchmark profile compiled successfully!"
	@echo "  Run with: make run-profile"

$(BENCHMARK_PROFILE_TARGET): $(SRC_OBJ) $(BENCHMARK_PROFILE_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -o $@ $^

$(OBJ_DIR)/benchmark_profile.o: $(BENCHMARK_PROFILE_SRC) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

# Build benchmark_user_config
benchmark-user: $(BENCHMARK_USER_TARGET)
	@echo "✓ User config benchmark compiled successfully!"
	@echo "  Run with: make run-user-benchmark"

$(BENCHMARK_USER_TARGET): $(SRC_OBJ) $(BENCHMARK_USER_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -o $@ $^

$(OBJ_DIR)/benchmark_user_config.o: $(BENCHMARK_USER_SRC) | $(OBJ_DIR)
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

# Run robustness tests
run-test-robustness: test-robustness
	@echo ""
	@echo "╔════════════════════════════════════╗"
	@echo "║    Running Robustness Tests        ║"
	@echo "╚════════════════════════════════════╝"
	@echo ""
	@$(TEST_ROBUSTNESS_TARGET)

# Run golden reference test
run-test-golden: test-golden
	@echo ""
	@echo "╔════════════════════════════════════╗"
	@echo "║    Running Golden Reference Test   ║"
	@echo "╚════════════════════════════════════╝"
	@echo ""
	@$(TEST_GOLDEN_TARGET)

# Generate golden reference
run-gen-golden: gen-golden
	@echo ""
	@echo "╔════════════════════════════════════╗"
	@echo "║    Generating Golden Reference     ║"
	@echo "╚════════════════════════════════════╝"
	@echo ""
	@$(GEN_GOLDEN_TARGET)

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

# Build and run profiling benchmark
profile: clean
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║  Building with profiling support (gprof)              ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@echo ""
	@$(MAKE) CXXFLAGS="$(CXXFLAGS) $(PROFILE_FLAGS)" benchmark-profile
	@echo ""
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║  Running benchmark to generate profiling data         ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@echo ""
	@$(BENCHMARK_PROFILE_TARGET)
	@echo ""
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║  Generating profiling report                           ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@echo ""
	@if [ -f gmon.out ]; then \
		gprof $(BENCHMARK_PROFILE_TARGET) gmon.out > profiling_report.txt; \
		echo "✓ Profiling complete!"; \
		echo ""; \
		echo "View results:"; \
		echo "  cat profiling_report.txt         # Full report"; \
		echo "  head -n 50 profiling_report.txt  # Quick summary"; \
		echo ""; \
		echo "Key sections:"; \
		echo "  - Flat profile: time per function"; \
		echo "  - Call graph: caller/callee relationships"; \
		echo ""; \
		echo "═══════════════════════════════════════════════════════"; \
		echo "Top functions by time:"; \
		echo "═══════════════════════════════════════════════════════"; \
		head -n 30 profiling_report.txt | tail -n +7; \
	else \
		echo "✗ No profiling data generated (gmon.out not found)"; \
	fi

# Run profiling (alias for profile)
run-profile: profile

# Run user-specific benchmark
run-user-benchmark: benchmark-user
	@echo ""
	@$(BENCHMARK_USER_TARGET)

# Build with debug
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean test test-robustness test-golden gen-golden example debug-buffers benchmark-profile
	@echo "✓ Debug build completed!"

# Clean up
clean:
	@echo "Cleaning up temporary files..."
	rm -rf $(BUILD_DIR)
	rm -f gmon.out profiling_report.txt
	@echo "✓ Clean up completed!"

# Information
help:
	@echo "╔════════════════════════════════════════════╗"
	@echo "║   Mpx Library - Makefile for C++17         ║"
	@echo "╚════════════════════════════════════════════╝"
	@echo ""
	@echo "Available targets:"
	@echo "  make all                   - Compile tests and example (default)"
	@echo "  make test                  - Compile basic tests only"
	@echo "  make test-robustness       - Compile robustness tests only"
	@echo "  make test-golden           - Compile golden reference test"
	@echo "  make gen-golden            - Compile golden reference generator"
	@echo "  make example               - Compile example only"
	@echo "  make debug-buffers         - Compile debug buffer inspector"
	@echo "  make run-test              - Compile and run basic tests"
	@echo "  make run-test-robustness   - Compile and run robustness tests"
	@echo "  make run-test-golden       - Compile and run golden reference test"
	@echo "  make run-gen-golden        - Generate golden reference CSV"
	@echo "  make run-example           - Compile and run example"
	@echo "  make run-debug-buffers     - Compile and run debug buffer inspector"
	@echo "  make debug                 - Compile with debug symbols"
	@echo "  make profile               - Profile with gprof (benchmark + report)"
	@echo "  make benchmark-profile     - Compile benchmark only"
	@echo "  make clean                 - Remove compiled artifacts"
	@echo "  make help                  - Show this message"
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
