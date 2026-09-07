## Quick Start - Mpx Library (Matrix Profile)

### 1. Quick Build

```bash
# Compile everything
make

# Compile and run basic tests
make run-test

# Compile and run robustness tests
make run-test-robustness

# Compile and run golden reference test
make run-test-golden

# Compile and run example
make run-example
```

### 2. File Structure

The implementation is located in:
- `src/Mpx.hpp` - Mpx class definition
- `src/Mpx.cpp` - Implementation (compiled into the binary)

### 3. Basic Usage

```cpp
#include <Mpx.hpp>

using namespace MatrixProfile;

int main() {
    // Create instance
    Mpx mpx(64, 0.5f, 0, 2000);  // window_size, exclusion_zone, time_constraint, buffer_size

    // Process data
    float data[256];
    mpx.compute(data, 256);

    // Analysis
    mpx.floss();

    // Access results
    float *matrix = mpx.get_matrix();
    int16_t *indexes = mpx.get_indexes();

    return 0;
}
```

### 4. Add Tests

Edit test files:

**Basic tests** (`tests/test_mpx.cpp`):
```cpp
void test_your_functionality() {
    Mpx mpx(64, 0.5f, 0, 2000);
    // your test
    print_test_result("Description", result);
}

// Call in main()
test_your_functionality();
```

**Robustness tests** (`tests/test_mpx_robustness.cpp`): Test edge cases and numerical stability

**Golden reference**: Generate with `make run-gen-golden`, test with `make run-test-golden`

### 5. Debug with GDB

```bash
# Compile with debug symbols
make debug

# Run with gdb
gdb ./build/bin/test_mpx

(gdb) break main
(gdb) run
(gdb) next
(gdb) print mpx.get_buffer_used()
```

### 6. Embedded Compilation

The library is ready to compile as part of the final binary:

**ESP32 with ESP-IDF**: The library auto-detects `ESP_PLATFORM` and uses `ESP_LOG` for logging.

**Linux**: Uses `printf`-based logging for debugging.

### 7. Cleanup

```bash
# Remove all compiled artifacts
make clean
```

### 8. Important Parameters

- **window_size**: Window size for Matrix Profile (should be < buffer_size/2)
- **exclusion_zone**: Fraction of window_size to avoid self-matches (typically 0.5)
- **buffer_size**: Total buffer size (larger = more history, more memory)

---

**Dica**: Use `make help` para ver todos os comandos disponíveis! 🚀
