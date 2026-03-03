# Mpx Library - AI Agent Instructions

## Project Overview
Time series analysis library implementing Matrix Profile algorithm (STOMP/STAMP) with FLOSS. Designed for **embedded microcontrollers** (ESP32) with Linux development/testing support. C++17, no external dependencies, compiled inline (no DLL/SO).

## Architecture Decisions

### Platform-Aware Design
- **ESP32**: Uses `ESP_PLATFORM` macro, `esp_random()`, `ESP_LOG` (see `include/mpx/Mpx.hpp:4-28`)
- **Desktop**: Falls back to `rand()`, `printf`-based logging
- **Why**: Single codebase for embedded deployment + desktop debugging

### Memory Management Pattern
```cpp
data_buffer_(std::make_unique<float[]>(buffer_size).release())
```
Uses `std::make_unique<>().release()` instead of raw `new[]`. **Why**: Exception safety during construction, but raw pointer storage for embedded compatibility (no shared_ptr overhead).

### Naming Convention
- Private methods end with underscore: `movmean_()`, `muinvn_()`, `mp_next_()`
- Member variables end with underscore: `window_size_`, `buffer_start_`, `data_buffer_`
- Public API has no underscores: `compute()`, `get_matrix()`, `floss()`

## Critical Workflows

### Build & Test (Not CMake!)
```bash
make              # Compile tests + example
make run-test     # Compile and execute tests (shows output in new panel)
make run-example  # Compile and execute example
make debug        # Compile with -g -O0 for GDB
make clean        # Remove build/
```
**Why Makefile over CMake**: Simplicity (1 header + 1 source), transparency, low overhead for embedded, works with ESP-IDF.

### VSCode Integration
- **Ctrl+Shift+B**: Default build (compiles all)
- **Ctrl+Shift+T**: Run tests
- **F5**: Debug tests with GDB (uses `.vscode/launch.json` configs)
- VSCode tasks defined in `.vscode/tasks.json` map to Makefile targets

### Testing Pattern
Manual tests using `assert()` + `print_test_result()` helper (see `tests/test_mpx.cpp`). **Not** using Google Test/Catch2 to avoid dependencies. Each test function:
1. Creates `Mpx` instance
2. Performs operations
3. Asserts conditions
4. Prints PASS/FAIL

## Code Patterns

### Algorithm Implementation (src/Mpx.cpp)
- **Kahan summation** for precision (see `movmean_()`, `movsig_()`)
- **Monte Carlo simulation** for Ideal Arc Counts (`floss_iac_()`) - alternative to Kumaraswamy distribution
- **Streaming data processing**: `compute()` accepts chunks, maintains circular buffer

### Data Access
All getters return **raw pointers** to internal buffers (no copies). Caller must not delete:
```cpp
float *matrix = mpx.get_matrix();        // Matrix Profile results
int16_t *indexes = mpx.get_indexes();    // Nearest neighbor indexes
float *floss = mpx.get_floss();          // Semantic segmentation
```

### Type Constraints
- `uint16_t` for sizes (max 65535 samples - sufficient for embedded)
- `int16_t` for indexes (allows -1 for "not found")
- `float` (32-bit) - assumes FPU on target microcontroller

## Development Guidelines

### Adding New Features
1. Update `include/mpx/Mpx.hpp` for public API
2. Implement in `src/Mpx.cpp` (private methods with `_` suffix)
3. Add test in `tests/test_mpx.cpp` following existing pattern
4. Update example in `examples/example.cpp` if user-facing
5. Run `make run-test` to validate

### Platform-Specific Code
Always check `ESP_PLATFORM` for conditional compilation:
```cpp
#if defined(ESP_PLATFORM)
    ESP_LOGD(TAG, "Debug info");
#else
    LOG_DEBUG("mpx", "Debug info");
#endif
```

### Code Style
- clang-format configuration in `.clang-format`
- clang-tidy enabled (install: `sudo apt install clang-tidy`)
- C++ Tools extension (`ms-vscode.cpptools`) provides inline linting
