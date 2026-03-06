// Benchmark for user's specific configuration: buffer=5000, window=200

#include "mpx/Mpx.hpp"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>

using namespace MatrixProfile;
using namespace std::chrono;

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

void generate_data(float *data, uint16_t size, uint16_t offset) {
  const float period = 100.0F;
  const float two_pi = 2.0F * 3.14159265358979323846F;
  for (uint16_t i = 0; i < size; i++) {
    float t = static_cast<float>(i + offset);
    data[i] = sinf(two_pi * t / period) + 0.1F * sinf(two_pi * t / (period * 3.0F));
  }
}

int main() {
  printf("\n╔═══════════════════════════════════════════════════════╗\n");
  printf("║  User Configuration Benchmark                        ║\n");
  printf("║  buffer=5000, window=200                             ║\n");
  printf("╚═══════════════════════════════════════════════════════╝\n\n");

  const uint16_t window_size = 200;
  const uint16_t buffer_size = 5000;
  const uint16_t warmup_size = 4800;
  const uint16_t update_size = 100;  // User's actual chunk size
  const uint16_t num_updates = 50;

  // Memory calculation
  uint16_t profile_len = buffer_size - window_size + 1;
  uint32_t data_buffer_mem = buffer_size * 4;
  uint32_t profile_arrays_mem = profile_len * 4 * 7; // vmmu, vsig, vddf, vddg, vmatrix, floss, iac
  uint32_t index_mem = profile_len * 2;              // int16_t
  uint32_t vww_mem = window_size * 4;
  uint32_t total_mem = data_buffer_mem + profile_arrays_mem + index_mem + vww_mem;

  printf("═══════════════════════════════════════════════════════\n");
  printf("  Memory Footprint Analysis\n");
  printf("═══════════════════════════════════════════════════════\n\n");
  printf("Configuration:\n");
  printf("  Window size:     %u\n", window_size);
  printf("  Buffer size:     %u\n", buffer_size);
  printf("  Profile length:  %u\n\n", profile_len);

  printf("Memory breakdown:\n");
  printf("  data_buffer_:      %6u bytes (%5.1f KB)\n", data_buffer_mem, data_buffer_mem / 1024.0);
  printf("  Profile arrays:    %6u bytes (%5.1f KB)\n", profile_arrays_mem, profile_arrays_mem / 1024.0);
  printf("  Index array:       %6u bytes (%5.1f KB)\n", index_mem, index_mem / 1024.0);
  printf("  Window array:      %6u bytes (%5.1f KB)\n", vww_mem, vww_mem / 1024.0);
  printf("  ───────────────────────────────────────\n");
  printf("  TOTAL:             %6u bytes (%5.1f KB)\n\n", total_mem, total_mem / 1024.0);

  printf("If adding scratch buffer for linearization:\n");
  printf("  + scratch_buffer:  %6u bytes (%5.1f KB)\n", data_buffer_mem, data_buffer_mem / 1024.0);
  printf("  ───────────────────────────────────────\n");
  printf("  NEW TOTAL:         %6u bytes (%5.1f KB)\n", total_mem + data_buffer_mem,
         (total_mem + data_buffer_mem) / 1024.0);
  printf("  Extra memory:      +%.1f%%\n\n", (data_buffer_mem * 100.0) / total_mem);

  printf("═══════════════════════════════════════════════════════\n");
  printf("  Performance Benchmark\n");
  printf("═══════════════════════════════════════════════════════\n\n");

  std::vector<float> warmup_data(warmup_size);
  std::vector<float> update_data(update_size);
  generate_data(warmup_data.data(), warmup_size, 0);

  Timer timer;
  Mpx mpx(window_size, 0.5f, 0, buffer_size);
  double init_time = timer.elapsed_ms();

  timer.reset();
  mpx.compute(warmup_data.data(), warmup_size);
  double first_pass_time = timer.elapsed_ms();

  timer.reset();
  mpx.floss();
  double floss_time = timer.elapsed_ms();

  printf("First pass (cold start):\n");
  printf("  Initialization:  %7.3f ms\n", init_time);
  printf("  Compute:         %7.3f ms\n", first_pass_time);
  printf("  FLOSS:           %7.3f ms\n", floss_time);
  printf("  TOTAL:           %7.3f ms\n\n", init_time + first_pass_time + floss_time);

  // Streaming benchmark
  timer.reset();
  double total_stream = 0.0;
  double min_stream = 1e9;
  double max_stream = 0.0;

  for (uint16_t i = 0; i < num_updates; i++) {
    generate_data(update_data.data(), update_size, warmup_size + i * update_size);
    Timer update_timer;
    mpx.compute(update_data.data(), update_size);
    double update_time = update_timer.elapsed_ms();
    total_stream += update_time;
    if (update_time < min_stream)
      min_stream = update_time;
    if (update_time > max_stream)
      max_stream = update_time;
  }

  double avg_stream = total_stream / num_updates;

  printf("Streaming updates (%u samples, %u iterations):\n", update_size, num_updates);
  printf("  Average:         %7.3f ms/update (%6.0f updates/sec)\n", avg_stream, 1000.0 / avg_stream);
  printf("  Min:             %7.3f ms\n", min_stream);
  printf("  Max:             %7.3f ms\n", max_stream);
  printf("  Total:           %7.3f ms\n\n", total_stream);

  // Shift overhead analysis
  printf("═══════════════════════════════════════════════════════\n");
  printf("  Shift Overhead Estimate\n");
  printf("═══════════════════════════════════════════════════════\n\n");

  printf("Based on Benchmark 4 scaling (buffer size impact):\n");
  printf("  Buffer 2000:     ~0.082 ms/update\n");
  printf("  Buffer 4000:     ~0.179 ms/update (2.18x)\n");
  printf("  Buffer 5000:     ~0.224 ms/update (estimated, 2.73x)\n\n");

  printf("Your actual measurement:\n");
  printf("  Streaming:       %7.3f ms/update\n\n", avg_stream);

  double estimated_shift = 0.082 * (5000.0 / 2000.0);
  double estimated_compute = avg_stream - estimated_shift;
  double shift_percentage = (estimated_shift / avg_stream) * 100.0;

  printf("Rough breakdown (estimated):\n");
  printf("  Shift overhead:  ~%.3f ms (%.0f%% of time)\n", estimated_shift, shift_percentage);
  printf("  Compute work:    ~%.3f ms (%.0f%% of time)\n\n", estimated_compute, 100.0 - shift_percentage);

  printf("═══════════════════════════════════════════════════════\n");
  printf("  Recommendations\n");
  printf("═══════════════════════════════════════════════════════\n\n");

  if (shift_percentage > 40.0) {
    printf("⚠️  Shift overhead is significant (>40%% of update time)\n");
    printf("   Consider ring buffer or memmove optimization\n\n");
  } else {
    printf("✅ Compute dominates (%.0f%% of time)\n", 100.0 - shift_percentage);
    printf("   Current contiguous layout is optimal\n");
    printf("   Ring buffer would add 20 KB memory for minimal gain\n\n");
  }

  printf("Sample rate capacity:\n");
  printf("  @ 100 samples/update: %6.0f Hz\n", (100.0 * 1000.0) / avg_stream);
  printf("  @ 1 kHz sample rate:  %.1f ms latency/update\n\n", avg_stream);

  printf("Memory budget check:\n");
  printf("  Total RAM used:       %.1f KB\n", total_mem / 1024.0);
  printf("  Typical ESP32 free:   200-300 KB\n");
  printf("  Percentage:           %.1f%% - %.1f%%\n\n", (total_mem / 1024.0) / 3.0,
         (total_mem / 1024.0) / 2.0);

  return 0;
}
