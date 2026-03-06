# Mpx Library - Matrix Profile for C++17

Time series analysis library developed in C++17, implementing optimized Matrix Profile algorithm with FLOSS. Optimized for embedded microcontrollers with native Linux debugging and testing support.

## Features

- **Matrix Profile**: Efficient implementation of STOMP/STAMP algorithm
- **FLOSS**: Fast Low-cost Online Semantic Segmentation for change detection
- **Streaming**: Processes data streams with low memory overhead
- **C++17**: Modern, type-safe and optimized
- **No external dependencies**: Compilable with standard library only
- **Inline compilation**: Compiled directly into final binary, no DLL/SO
- **Embedded-friendly**: Optimized for microcontrollers

## Implementation Features

- **Algorithm**: Right Matrix Profile (RMP) with exclusion zone handling
- **Adaptive logging**: ESP_LOG for ESP32, `printf` on desktop
- **Precision**: Cumulative calculation with residuals to minimize floating-point errors
- **IAC Computation**: Analytical Kumaraswamy distribution for FLOSS normalization

## Project Structure

```
mpx-embedding/
├── include/mpx/          # Library headers
│   └── Mpx.hpp           # Mpx class (Matrix Profile)
├── src/                  # Implementation
│   └── Mpx.cpp           # Mpx implementation
├── tests/                # Unit tests
│   ├── test_mpx.cpp      # Basic unit tests
│   ├── test_mpx_robustness.cpp  # Robustness tests
│   ├── test_mpx_golden.cpp      # Regression tests
│   ├── generate_golden_reference.cpp  # Golden reference generator
│   ├── test_data.csv     # Test data
│   └── golden_reference.csv  # Golden reference (generated)
├── examples/             # Usage examples
│   ├── example.cpp       # Complete practical example
│   └── debug_buffers.cpp # Buffer inspection tool
├── build/                # Build directory (generated)
├── Makefile              # Build system
└── README.md             # This file
```

## Building the Project

### Requirements

- GCC or Clang with C++17 support
- Make
- Linux

### Basic Commands

```bash
# Compile everything (tests and example)
make

# Compile and run basic tests
make run-test

# Compile and run robustness tests
make run-test-robustness

# Compile and run golden reference test
make run-test-golden

# Generate golden reference CSV
make run-gen-golden

# Compile and run example
make run-example

# Compile with debug symbols
make debug

# Clean build artifacts
make clean

# Show all available targets
make help
```

## Basic Usage

```cpp
#include <mpx/Mpx.hpp>

using namespace MatrixProfile;

int main() {
    // Create Mpx instance
    // window_size, exclusion_zone ratio, time_constraint, buffer_size
    Mpx mpx(64, 0.5f, 0, 2000);

    // Process data stream
    float data[256];
    // ... fill with data ...

    uint16_t result = mpx.compute(data, 256);

    // Compute FLOSS for change detection
    mpx.floss();

    // Access results
    float *matrix_profile = mpx.get_matrix();
    int16_t *profile_indexes = mpx.get_indexes();
    float *floss = mpx.get_floss();

    return 0;
}
```

## Mpx Class API

### Constructors

```cpp
Mpx(uint16_t window_size,
    float ez = 0.5f,                    // Exclusion zone ratio
    uint16_t time_constraint = 0,
    uint16_t buffer_size = 5000);
```

### Processing

```cpp
uint16_t compute(const float *data, uint16_t size);
void floss();                           // Compute FLOSS for segmentation
void prune_buffer();                    // Reinitialize buffer with sinusoidal pattern
```

### Getters - Data

```cpp
float *get_data_buffer();               // Data buffer
float *get_matrix();                    // Matrix Profile
int16_t *get_indexes();                 // Matrix Profile indexes
float *get_floss();                     // FLOSS (segmentation)
float *get_iac();                       // Ideal Arc Counts
```

### Getters - Intermediate

```cpp
float *get_vmmu();                      // Moving Mean array
float *get_vsig();                      // Moving Sigma array
float *get_ddf();                       // Differential of data forward
float *get_ddg();                       // Differential of data (gradient-like)
float *get_vww();                       // Query window weights
```

### Getters - Information

```cpp
uint16_t get_buffer_used();             // Number of samples in buffer
int16_t get_buffer_start();             // Buffer start position
uint16_t get_profile_len();             // Profile length
float get_last_movsum();                // Last cumulative movement
float get_last_mov2sum();               // Last cumulative movement²
```

## Debugging on Linux

To debug line by line:

```bash
# Compile with debug symbols
make debug

# Use GDB
gdb ./build/bin/test_mpx

# Inside GDB
(gdb) break test_mpx_compute
(gdb) run
(gdb) next     # next line
(gdb) step     # enter function
(gdb) print mpx.get_buffer_used()
```

## Compiling for Microcontrollers

The library was designed to compile as part of the final binary (no DLL/SO):

### For ESP32 with ESP-IDF

```c
// In your ESP-IDF project add the directory as a component
// The library will auto-detect ESP_PLATFORM
```

### For Linux (development/testing)

```bash
make test       # Compile and create test executable
make run-test   # Run tests
```

## Build Model

- **Desktop (Linux)**: Simple Makefile compilation, no dependencies
- **Embedded (ESP32)**: Compatible with ESP-IDF, uses `ESP_LOG` for logging
- **No DLL**: Header-only compilation + source files linked directly to final binary

## Why Makefile?

This project uses **Makefile** (not CMake) because:

1. **Simplicity**: Library with few files (1 header + 1 source)
2. **Fast compilation**: No CMake overhead
3. **Portability**: Works on Linux, ESP-IDF, embedded
4. **Transparency**: Easy to see exactly how compilation happens
5. **Low overhead**: Ideal for embedded

## Data Types

- `uint16_t`: Maximum size is 65535 (sufficient for embedded buffers)
- `float`: 32-bit, suitable for microcontrollers with FPU
- `int16_t`: Indexes (allows -1 for "not found")

## References

The algorithm is based on:

- **Matrix Profile**: Discovery of Time Series Motifs (https://www.cs.ucr.edu/~eamonn/MatrixProfile.html)
- **FLOSS**: Fast Low-cost Online Semantic Segmentation (Yeh et al.)
- **STOMP**: Scalable Time series Ordered Motif (Pattern) Discovery

## Testing

The project includes comprehensive test suites:

1. **Basic Tests** (`test_mpx.cpp`): Core functionality validation
2. **Robustness Tests** (`test_mpx_robustness.cpp`): Edge cases and numerical stability
3. **Golden Reference Tests** (`test_mpx_golden.cpp`): Regression testing against known-good results

## Next Steps for Development

1. Use `make run-test` to iterate quickly
2. Add your specific tests in `tests/test_mpx.cpp`
3. Run `make run-test-robustness` to verify robustness
4. Generate golden reference with `make run-gen-golden` for regression testing
5. Use `make debug` + GDB to inspect internal calculations
6. Modify `include/mpx/Mpx.hpp` to customize as needed

## License

[Add your license here]

---

**Developed for Linux to facilitate debugging and testing of time series analysis for embedded systems.**
