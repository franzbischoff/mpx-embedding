/**
 * @file generate_golden_reference.cpp
 * @brief Generate golden reference CSV for regression testing
 *
 * This program processes test data using specific parameters and saves
 * all internal buffer states to a CSV file. This CSV serves as the
 * "golden reference" (ground truth) for regression testing.
 *
 * Parameters match debug_buffers.cpp:
 *   - window_size: 210
 *   - buffer_size: 5000
 *   - chunk_size: 500
 *   - num_iterations: 54
 *
 * Output: tests/golden_reference.csv
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include "../include/mpx/Mpx.hpp"

std::vector<float> read_csv_data(const std::string &filename) {
  std::vector<float> data;
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cerr << "ERROR: Could not open " << filename << std::endl;
    return data;
  }

  std::string line;
  bool skip_first = true;

  while (std::getline(file, line)) {
    if (skip_first) {
      skip_first = false;
      continue;
    }

    line.erase(0, line.find_first_not_of("\" \t"));
    line.erase(line.find_last_not_of("\" \t") + 1);

    if (!line.empty()) {
      try {
        data.push_back(std::stof(line));
      } catch (const std::exception &e) {
        std::cerr << "WARNING: Could not parse line: " << line << std::endl;
      }
    }
  }

  file.close();
  return data;
}

int main() {
  std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║   Golden Reference Generator                       ║" << std::endl;
  std::cout << "║   Regression Testing - Mpx Library                 ║" << std::endl;
  std::cout << "╚════════════════════════════════════════════════════╝\n" << std::endl;

  // Load test data
  std::cout << "1. Loading test data..." << std::endl;
  std::vector<float> data = read_csv_data("tests/test_data.csv");

  if (data.empty()) {
    std::cerr << "   ERROR: No data loaded from test_data.csv" << std::endl;
    return 1;
  }
  std::cout << "   ✓ Loaded " << data.size() << " samples" << std::endl;

  // Configuration (matches debug_buffers.cpp)
  const uint16_t window_size = 210;
  const uint16_t buffer_size = 5000;
  const uint16_t chunk_size = 500;
  const uint16_t num_iterations = 54;

  std::cout << "\n2. Configuration:" << std::endl;
  std::cout << "   - Window size: " << window_size << std::endl;
  std::cout << "   - Buffer size: " << buffer_size << std::endl;
  std::cout << "   - Chunk size: " << chunk_size << std::endl;
  std::cout << "   - Iterations: " << num_iterations << std::endl;

  // Initialize Mpx
  std::cout << "\n3. Initializing Mpx..." << std::endl;
  MatrixProfile::Mpx mpx(window_size, 0.5f, 0, buffer_size);
  std::cout << "   ✓ Mpx initialized" << std::endl;

  // Process data in chunks
  std::cout << "\n4. Processing " << num_iterations << " chunks of " << chunk_size << " samples..." << std::endl;
  for (uint16_t iter = 0; iter < num_iterations; iter++) {
    uint32_t offset = iter * chunk_size;

    if (offset + chunk_size > data.size()) {
      std::cerr << "   WARNING: Not enough data. Stopping at iteration " << (iter + 1) << std::endl;
      break;
    }

    uint16_t result = mpx.compute(&data[offset], chunk_size);

    if ((iter + 1) % 10 == 0) {
      std::cout << "   ✓ Processed " << (iter + 1) << "/" << num_iterations
                << " chunks (free space: " << result << ")" << std::endl;
    }
  }
  std::cout << "   ✓ All chunks processed" << std::endl;

  // Compute FLOSS
  std::cout << "\n5. Computing FLOSS..." << std::endl;
  mpx.floss();
  std::cout << "   ✓ FLOSS computed" << std::endl;

  // Get final state
  uint16_t buffer_used = mpx.get_buffer_used();
  int16_t buffer_start = mpx.get_buffer_start();
  uint16_t profile_len = mpx.get_profile_len();
  float last_movsum = mpx.get_last_movsum();
  float last_mov2sum = mpx.get_last_mov2sum();

  std::cout << "\n6. Final state:" << std::endl;
  std::cout << "   - Buffer used: " << buffer_used << std::endl;
  std::cout << "   - Buffer start: " << buffer_start << std::endl;
  std::cout << "   - Profile length: " << profile_len << std::endl;
  std::cout << "   - Last movsum: " << std::fixed << std::setprecision(6) << last_movsum << std::endl;
  std::cout << "   - Last mov2sum: " << std::fixed << std::setprecision(6) << last_mov2sum << std::endl;

  // Get all buffers
  float *data_buffer = mpx.get_data_buffer();
  float *matrix = mpx.get_matrix();
  int16_t *indexes = mpx.get_indexes();
  float *floss = mpx.get_floss();
  float *iac = mpx.get_iac();
  float *vmmu = mpx.get_vmmu();
  float *vsig = mpx.get_vsig();
  float *vddf = mpx.get_ddf();
  float *vddg = mpx.get_ddg();

  // Write golden reference CSV
  std::cout << "\n7. Writing golden reference CSV..." << std::endl;
  std::ofstream golden("tests/golden_reference.csv");

  if (!golden.is_open()) {
    std::cerr << "   ERROR: Could not create golden_reference.csv" << std::endl;
    return 1;
  }

  // Write header
  golden << "buffer_type,index,value_float,value_int\n";

  // Write metadata
  golden << "metadata,buffer_used," << buffer_used << ",0\n";
  golden << "metadata,buffer_start," << buffer_start << ",0\n";
  golden << "metadata,profile_len," << profile_len << ",0\n";
  golden << "metadata,last_movsum," << std::fixed << std::setprecision(10) << last_movsum << ",0\n";
  golden << "metadata,last_mov2sum," << std::fixed << std::setprecision(10) << last_mov2sum << ",0\n";

  // Write data_buffer (only used portion)
  for (uint16_t i = 0; i < buffer_used; i++) {
    golden << "data_buffer," << i << "," << std::setprecision(10) << data_buffer[i] << ",0\n";
  }

  // Write matrix profile
  for (uint16_t i = 0; i < profile_len; i++) {
    golden << "matrix_profile," << i << "," << std::setprecision(10) << matrix[i] << ",0\n";
  }

  // Write profile indexes
  for (uint16_t i = 0; i < profile_len; i++) {
    golden << "profile_indexes," << i << ",0.0," << indexes[i] << "\n";
  }

  // Write FLOSS
  for (uint16_t i = 0; i < profile_len; i++) {
    golden << "floss," << i << "," << std::setprecision(10) << floss[i] << ",0\n";
  }

  // Write IAC
  for (uint16_t i = 0; i < profile_len; i++) {
    golden << "iac," << i << "," << std::setprecision(10) << iac[i] << ",0\n";
  }

  // Write VMMU (moving mean)
  for (uint16_t i = 0; i < profile_len; i++) {
    golden << "vmmu," << i << "," << std::setprecision(10) << vmmu[i] << ",0\n";
  }

  // Write VSIG (moving stddev)
  for (uint16_t i = 0; i < profile_len; i++) {
    golden << "vsig," << i << "," << std::setprecision(10) << vsig[i] << ",0\n";
  }

  // Write VDDF
  for (uint16_t i = 0; i < profile_len; i++) {
    golden << "vddf," << i << "," << std::setprecision(10) << vddf[i] << ",0\n";
  }

  // Write VDDG
  for (uint16_t i = 0; i < profile_len; i++) {
    golden << "vddg," << i << "," << std::setprecision(10) << vddg[i] << ",0\n";
  }

  golden.close();

  // Count total entries
  uint32_t total_entries = 5 + buffer_used + (profile_len * 9);

  std::cout << "   ✓ Golden reference saved to tests/golden_reference.csv" << std::endl;
  std::cout << "   ✓ Total entries: " << total_entries << std::endl;

  std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║   Golden reference generated successfully!         ║" << std::endl;
  std::cout << "╚════════════════════════════════════════════════════╝\n" << std::endl;

  return 0;
}
