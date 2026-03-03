#include <iostream>
#include <cmath>
#include "mpx/Mpx.hpp"

using namespace MatrixProfile;

/**
 * @brief Example of using Mpx library
 *
 * Demonstrates how to use the Mpx class for time series analysis
 * using Matrix Profile and FLOSS.
 */

int main() {
  std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║        Example - Mpx Library                       ║" << std::endl;
  std::cout << "║   Matrix Profile for Time Series Analysis          ║" << std::endl;
  std::cout << "╚════════════════════════════════════════════════════╝" << std::endl << std::endl;

  // Parameters
  const uint16_t window_size = 64;   // Window size
  const float exclusion_zone = 0.5f; // Exclusion zone (as fraction)
  const uint16_t buffer_size = 2000; // Buffer size

  std::cout << "1. Initializing Mpx object with parameters:" << std::endl;
  std::cout << "   - Window size: " << window_size << std::endl;
  std::cout << "   - Exclusion zone: " << exclusion_zone << std::endl;
  std::cout << "   - Buffer size: " << buffer_size << std::endl << std::endl;

  Mpx mpx(window_size, exclusion_zone, 0, buffer_size);

  // Generate test data: a time series with patterns
  std::cout << "2. Generating test time series..." << std::endl;
  float test_data[512];

  // First half: sinusoidal signal
  for (int i = 0; i < 256; i++) {
    float angle = (i / 16.0f) * 3.14159f; // Pi radians
    test_data[i] = 2.0f * sinf(angle) + 0.1f * (i % 10);
  }

  // Second half: noise with pattern
  for (int i = 256; i < 512; i++) {
    test_data[i] = 2.0f * sinf((i / 16.0f) * 3.14159f) + 0.2f * (i % 10);
  }

  std::cout << "   Data generated: 512 samples" << std::endl << std::endl;

  // Process data in chunks (simulating stream)
  std::cout << "3. Processing data stream..." << std::endl;
  uint16_t chunks = 2;
  for (uint16_t chunk = 0; chunk < chunks; chunk++) {
    uint16_t offset = chunk * 256;
    mpx.compute(&test_data[offset], 256);
    std::cout << "   ✓ Processed chunk " << (chunk + 1) << "/" << chunks << std::endl;
  }

  std::cout << std::endl;

  // Information after processing
  std::cout << "4. Information after processing:" << std::endl;
  std::cout << "   - Buffer used: " << mpx.get_buffer_used() << " / " << buffer_size << std::endl;
  std::cout << "   - Profile length: " << mpx.get_profile_len() << std::endl;
  std::cout << "   - Buffer position: " << mpx.get_buffer_start() << std::endl;

  // Last movsum
  float last_movsum = mpx.get_last_movsum();
  float last_mov2sum = mpx.get_last_mov2sum();
  std::cout << "   - Last movsum: " << last_movsum << std::endl;
  std::cout << "   - Last mov2sum: " << last_mov2sum << std::endl << std::endl;

  // Compute FLOSS (Fast Low-cost Online Semantic Segmentation)
  std::cout << "5. Computing FLOSS for change detection..." << std::endl;
  mpx.floss();
  std::cout << "   ✓ FLOSS computed successfully" << std::endl << std::endl;

  // Analyze results
  std::cout << "6. Analyzing results:" << std::endl;

  float *matrix_profile = mpx.get_matrix();
  int16_t *profile_index = mpx.get_indexes();
  float *floss = mpx.get_floss();

  if (matrix_profile != nullptr) {
    // Find minimum matrix profile value (motif)
    float min_mp = matrix_profile[0];
    uint16_t min_idx = 0;

    uint16_t prof_len = mpx.get_profile_len();
    for (uint16_t i = 1; i < prof_len && i < 50; i++) { // Limit to 50 for example
      if (matrix_profile[i] > 0 && matrix_profile[i] < min_mp) {
        min_mp = matrix_profile[i];
        min_idx = i;
      }
    }

    std::cout << "   - Minimum Matrix Profile value (motif): " << min_mp << " at position " << min_idx << std::endl;

    if (profile_index[min_idx] >= 0) {
      std::cout << "   - Its matching pattern is at: " << profile_index[min_idx] << std::endl;
    }
  }

  if (floss != nullptr) {
    // Show some FLOSS values
    uint16_t prof_len = mpx.get_profile_len();
    std::cout << "   - First 10 FLOSS values:" << std::endl;
    std::cout << "     ";
    for (uint16_t i = 0; i < 10 && i < prof_len; i++) {
      std::cout << " " << floss[i];
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;
  std::cout << "╔════════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║       Example completed successfully!               ║" << std::endl;
  std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;

  return 0;
}
