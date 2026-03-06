# Profiling Guide for Mpx Library

## Quick Start

Run the complete profiling workflow with one command:

```bash
make profile
```

This will:
1. Clean previous builds
2. Compile with profiling flags (`-pg` for gprof)
3. Run comprehensive benchmarks
4. Generate `profiling_report.txt` with detailed analysis

## Profiling Targets

### Available Commands

```bash
make profile              # Full workflow: build, run, generate report
make benchmark-profile    # Compile benchmark only  
make run-profile          # Alias for 'make profile'
make clean                # Remove build artifacts and profiling data
```

## Benchmark Scenarios

The benchmark tests four critical scenarios:

### 1. First Pass (Cold Start)
- Tests initial buffer fill and first matrix profile computation
- Measures: initialization, first `compute()`, and `floss()`
- Configuration: window=64, buffer=2000, data=1800

### 2. Streaming Updates
- Tests incremental updates after warm-up
- Measures: average, min, max update times over 100 iterations
- Configuration: chunk size=16, simulates real-time streaming

### 3. Small Updates
- Tests very small update sizes (1, 2, 4 samples)
- Important for sample-by-sample scenarios
- Reports updates per second throughput

### 4. Shift Overhead Analysis
- Tests how buffer size affects shift performance
- Compares buffer sizes: 1000, 2000, 4000
- Isolates the cost of memory copies in `new_data_()`, `muinvn_()`, etc.

## Understanding the Results

### Profiling Report Structure

The generated `profiling_report.txt` contains two main sections:

#### 1. Flat Profile
Shows time spent in each function:
```
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  ms/call  ms/call  name    
```

- **% time**: Percentage of total runtime in this function
- **self seconds**: Time in this function only (excluding callees)
- **calls**: Number of times the function was called
- **self ms/call**: Average time per call

#### 2. Call Graph
Shows caller/callee relationships:
- Which functions call what
- Time attribution through the call stack
- Helps identify bottlenecks in complex call chains

### Key Functions to Watch

When analyzing shift-based vs ring-buffer designs, focus on:

1. **`new_data_()`** - Data buffer shift (line 210 of Mpx.cpp)
2. **`muinvn_()`** - Mean/sigma shift (line 146-148)
3. **`ddf_()`** - Differential shift (line 270-271)
4. **`ddg_()`** - Gradient shift (line 294-295)
5. **`mp_next_()`** - Matrix profile shift (line 244-245)
6. **`compute()`** - Main computation (line 518-560, nested loops)

### What the Numbers Tell You

**If shift overhead dominates:**
- `new_data_()`, `muinvn_()`, `ddf_()`, `ddg_()`, `mp_next_()` have high self-seconds
- Larger buffer sizes show proportionally higher update times in Benchmark 4
- **Ring buffer approach may help**

**If compute dominates:**
- `compute()` inner loops have high self-seconds
- Shift time is small compared to compute time
- **Contiguous layout (current) is better**

## Typical Results Interpretation

### Example Output from Benchmark 4:
```
Buffer 1000:  0.039 ms/update
Buffer 2000:  0.086 ms/update
Buffer 4000:  0.180 ms/update
```

**Analysis:**
- Update time roughly doubles as buffer size doubles
- This suggests O(N) shift overhead is measurable
- For buffer=4000, shifts take ~0.180ms per 16-sample update

**Decision:**
- If your application uses large buffers (>2000) with small updates (<16), consider ring buffer
- If using moderate buffers with larger chunks, current design is fine

## Advanced Profiling

### Using gprof Directly

View specific function details:
```bash
gprof ./build/bin/benchmark_profile gmon.out | less
```

Search for specific functions:
```bash
gprof ./build/bin/benchmark_profile gmon.out | grep -A 10 "muinvn_"
```

### Profiling on ESP32

For embedded profiling, use ESP-IDF tools:

```bash
# Enable profiling in sdkconfig
CONFIG_APPTRACE_ENABLE=y
CONFIG_SYSVIEW_ENABLE=y

# Use SystemView or app_trace
idf.py monitor
```

Alternatively, add manual timing:
```cpp
#include <esp_timer.h>

int64_t start = esp_timer_get_time();
mpx.compute(data, size);
int64_t elapsed_us = esp_timer_get_time() - start;
ESP_LOGI(TAG, "compute took %lld us", elapsed_us);
```

## Performance Optimization Checklist

Before deciding on ring buffer vs shift-based:

- [ ] Run `make profile` with your target buffer size
- [ ] Check Benchmark 4 results - is shift time linear with buffer size?
- [ ] Check Benchmark 2/3 - what's the ratio of shift time to compute time?
- [ ] Consider your typical use case:
  - [ ] Large buffer + small updates → ring buffer likely better
  - [ ] Moderate buffer + chunk updates → current design fine
  - [ ] Compute-heavy (large windows) → stick with contiguous

## Files Generated

After running `make profile`:

- `gmon.out` - Raw profiling data (binary)
- `profiling_report.txt` - Human-readable analysis
- `build/bin/benchmark_profile` - Benchmark executable (with profiling symbols)

## Cleaning Up

Remove all profiling data:
```bash
make clean
```

This removes:
- `build/` directory
- `gmon.out`
- `profiling_report.txt`

## Notes

- Profiling adds ~5-10% overhead; absolute times may differ from release builds
- For accurate embedded performance, profile on target hardware (ESP32)
- The `-O2` optimization in profiling preserves realistic instruction scheduling
- Ring buffer decision should be data-driven based on your actual workload
