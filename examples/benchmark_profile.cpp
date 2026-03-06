// Benchmark for profiling Matrix Profile implementation
// Compares first-pass (cold start) vs streaming updates

#include "mpx/Mpx.hpp"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>

using namespace MatrixProfile;
using namespace std::chrono;

// Timing helper
class Timer {
private:
  high_resolution_clock::time_point start_;

public:
  Timer() : start_(high_resolution_clock::now()) {}

  void reset() { start_ = high_resolution_clock::now(); }

  double elapsed_ms() const {
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start_).count() / 1000.0;
  }
};

// Generate synthetic data
void generate_data(float *data, uint16_t size, uint16_t offset) {
  const float period = 100.0F;
  const float two_pi = 2.0F * 3.14159265358979323846F;

  for (uint16_t i = 0; i < size; i++) {
    float t = static_cast<float>(i + offset);
    data[i] = sinf(two_pi * t / period) + 0.1F * sinf(two_pi * t / (period * 3.0F));
  }
}

// Benchmark 1: First pass (cold start)
void benchmark_first_pass() {
  printf("\n═══════════════════════════════════════════════════════\n");
  printf("  BENCHMARK 1: First Pass (Cold Start)\n");
  printf("═══════════════════════════════════════════════════════\n\n");

  const uint16_t window_size = 64;
  const uint16_t buffer_size = 2000;
  const uint16_t data_size = 1800;

  std::vector<float> data(data_size);
  generate_data(data.data(), data_size, 0);

  Timer timer;
  Mpx mpx(window_size, 0.5f, 0, buffer_size);
  double init_time = timer.elapsed_ms();

  timer.reset();
  mpx.compute(data.data(), data_size);
  double compute_time = timer.elapsed_ms();

  timer.reset();
  mpx.floss();
  double floss_time = timer.elapsed_ms();

  printf("Configuration:\n");
  printf("  Window size:     %u\n", window_size);
  printf("  Buffer size:     %u\n", buffer_size);
  printf("  Data size:       %u\n", data_size);
  printf("  Profile length:  %u\n\n", mpx.get_profile_len());

  printf("Timing:\n");
  printf("  Init:            %.3f ms\n", init_time);
  printf("  First compute(): %.3f ms\n", compute_time);
  printf("  FLOSS:           %.3f ms\n", floss_time);
  printf("  TOTAL:           %.3f ms\n\n", init_time + compute_time + floss_time);
}

// Benchmark 2: Streaming updates
void benchmark_streaming() {
  printf("\n═══════════════════════════════════════════════════════\n");
  printf("  BENCHMARK 2: Streaming Updates (After Warm-up)\n");
  printf("═══════════════════════════════════════════════════════\n\n");

  const uint16_t window_size = 64;
  const uint16_t buffer_size = 2000;
  const uint16_t warmup_size = 1800;
  const uint16_t update_size = 16; // Streaming chunk size
  const uint16_t num_updates = 100;

  std::vector<float> warmup_data(warmup_size);
  std::vector<float> update_data(update_size);
  generate_data(warmup_data.data(), warmup_size, 0);

  Timer timer;
  Mpx mpx(window_size, 0.5f, 0, buffer_size);
  mpx.compute(warmup_data.data(), warmup_size); // Warm-up
  double warmup_time = timer.elapsed_ms();

  // Now measure streaming updates
  timer.reset();
  double total_compute = 0.0;
  double min_compute = 1e9;
  double max_compute = 0.0;

  for (uint16_t i = 0; i < num_updates; i++) {
    generate_data(update_data.data(), update_size, warmup_size + i * update_size);

    Timer compute_timer;
    mpx.compute(update_data.data(), update_size);
    double compute_time = compute_timer.elapsed_ms();

    total_compute += compute_time;
    if (compute_time < min_compute)
      min_compute = compute_time;
    if (compute_time > max_compute)
      max_compute = compute_time;
  }

  double avg_compute = total_compute / num_updates;

  timer.reset();
  mpx.floss();
  double floss_time = timer.elapsed_ms();

  printf("Configuration:\n");
  printf("  Window size:     %u\n", window_size);
  printf("  Buffer size:     %u\n", buffer_size);
  printf("  Warmup size:     %u\n", warmup_size);
  printf("  Update size:     %u\n", update_size);
  printf("  Num updates:     %u\n\n", num_updates);

  printf("Timing:\n");
  printf("  Warmup:          %.3f ms\n", warmup_time);
  printf("  Updates (avg):   %.3f ms\n", avg_compute);
  printf("  Updates (min):   %.3f ms\n", min_compute);
  printf("  Updates (max):   %.3f ms\n", max_compute);
  printf("  Updates (total): %.3f ms\n", total_compute);
  printf("  FLOSS:           %.3f ms\n\n", floss_time);
}

// Benchmark 3: Small streaming updates (sample-by-sample simulation)
void benchmark_small_updates() {
  printf("\n═══════════════════════════════════════════════════════\n");
  printf("  BENCHMARK 3: Small Updates (size=1,2,4)\n");
  printf("═══════════════════════════════════════════════════════\n\n");

  const uint16_t window_size = 64;
  const uint16_t buffer_size = 2000;
  const uint16_t warmup_size = 1800;
  const uint16_t num_updates = 50;

  for (uint16_t update_size : {1, 2, 4}) {
    std::vector<float> warmup_data(warmup_size);
    std::vector<float> update_data(update_size);
    generate_data(warmup_data.data(), warmup_size, 0);

    Mpx mpx(window_size, 0.5f, 0, buffer_size);
    mpx.compute(warmup_data.data(), warmup_size); // Warm-up

    Timer timer;
    double total_compute = 0.0;

    for (uint16_t i = 0; i < num_updates; i++) {
      generate_data(update_data.data(), update_size, warmup_size + i * update_size);

      Timer compute_timer;
      mpx.compute(update_data.data(), update_size);
      total_compute += compute_timer.elapsed_ms();
    }

    double avg_compute = total_compute / num_updates;
    printf("  Update size %u:   %.3f ms/update (%.1f updates/sec)\n", update_size, avg_compute,
           1000.0 / avg_compute);
  }
  printf("\n");
}

// Benchmark 4: Shift overhead analysis
void benchmark_shift_overhead() {
  printf("\n═══════════════════════════════════════════════════════\n");
  printf("  BENCHMARK 4: Shift Overhead Analysis\n");
  printf("═══════════════════════════════════════════════════════\n\n");

  const uint16_t window_size = 64;
  const uint16_t update_size = 16;
  const uint16_t num_updates = 100;

  printf("Testing different buffer sizes (shift cost scales with buffer):\n\n");

  for (uint16_t buffer_size : {1000, 2000, 4000}) {
    const uint16_t warmup_size = buffer_size - 200; // Leave room for updates
    std::vector<float> warmup_data(warmup_size);
    std::vector<float> update_data(update_size);
    generate_data(warmup_data.data(), warmup_size, 0);

    Mpx mpx(window_size, 0.5f, 0, buffer_size);
    mpx.compute(warmup_data.data(), warmup_size);

    Timer timer;
    for (uint16_t i = 0; i < num_updates; i++) {
      generate_data(update_data.data(), update_size, warmup_size + i * update_size);
      mpx.compute(update_data.data(), update_size);
    }
    double total_time = timer.elapsed_ms();
    double avg_time = total_time / num_updates;

    printf("  Buffer %u:  %.3f ms/update\n", buffer_size, avg_time);
  }
  printf("\n");
}

// Benchmark 5: Window size impact
void benchmark_window_size() {
  printf("\n═══════════════════════════════════════════════════════\n");
  printf("  BENCHMARK 5: Window Size Impact on Computation\n");
  printf("═══════════════════════════════════════════════════════\n\n");

  const uint16_t buffer_size = 2000;
  const uint16_t update_size = 16;
  const uint16_t num_updates = 50;

  printf("Testing different window sizes (compute cost increases with window):\n");
  printf("Note: Larger windows = more inner product FLOPs per iteration\n\n");
  printf("  Window | Profile Len | First Pass | Streaming | FLOPs Ratio\n");
  printf("  ------ | ----------- | ---------- | --------- | -----------\n");

  for (uint16_t window_size : {32, 64, 128, 256}) {
    const uint16_t warmup_size = buffer_size - 400;
    std::vector<float> warmup_data(warmup_size);
    std::vector<float> update_data(update_size);
    generate_data(warmup_data.data(), warmup_size, 0);

    // Measure first pass
    Timer first_timer;
    Mpx mpx(window_size, 0.5f, 0, buffer_size);
    mpx.compute(warmup_data.data(), warmup_size);
    double first_pass_time = first_timer.elapsed_ms();

    // Measure streaming updates
    Timer stream_timer;
    for (uint16_t i = 0; i < num_updates; i++) {
      generate_data(update_data.data(), update_size, warmup_size + i * update_size);
      mpx.compute(update_data.data(), update_size);
    }
    double avg_stream_time = stream_timer.elapsed_ms() / num_updates;

    uint16_t profile_len = mpx.get_profile_len();
    double flops_ratio = static_cast<double>(window_size) / 32.0; // Relative to window=32

    printf("  %6u | %11u | %8.3f ms | %7.3f ms | %.2fx\n", window_size, profile_len, first_pass_time,
           avg_stream_time, flops_ratio);
  }
  printf("\n");
  printf("Key insight: Inner product loop (line 524) runs window_size times,\n");
  printf("             so doubling window ≈ doubles compute cost per diagonal.\n");
  printf("\n");
}

int main() {
  printf("\n");
  printf("╔═══════════════════════════════════════════════════════╗\n");
  printf("║  Matrix Profile - Performance Benchmark & Profiling  ║\n");
  printf("╚═══════════════════════════════════════════════════════╝\n");

  benchmark_first_pass();
  benchmark_streaming();
  benchmark_small_updates();
  benchmark_shift_overhead();
  benchmark_window_size();

  printf("═══════════════════════════════════════════════════════\n");
  printf("  Benchmark Complete\n");
  printf("═══════════════════════════════════════════════════════\n\n");

  printf("Profiling notes:\n");
  printf("  - Run 'gprof ./build/bin/benchmark_profile gmon.out' to see function-level timings\n");
  printf("  - Look for time spent in: compute(), muinvn_(), new_data_(), ddf_(), ddg_()\n");
  printf("  - Compare 'self seconds' vs 'cumulative seconds' to find bottlenecks\n");
  printf("  - Shift overhead appears in new_data_(), muinvn_(), ddf_(), ddg_(), mp_next_()\n\n");

  return 0;
}
