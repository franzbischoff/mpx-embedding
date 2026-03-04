#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "../include/mpx/Mpx.hpp"

/**
 * DEBUG BUFFERS TEST
 *
 * Inspects all internal buffers of the Mpx class after processing
 * a small dataset (100 values) with a large buffer (2000).
 *
 * This helps identify inconsistencies and debug point-by-point.
 */

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

void print_buffer(const std::string &name, float *buffer, uint16_t size) {
  std::cout << "\n" << name << " (" << size << " elements):" << std::endl;
  std::cout << std::string(80, '-') << std::endl;

  if (buffer == nullptr) {
    std::cout << "NULL pointer" << std::endl;
    return;
  }

  for (uint16_t i = 0; i < size; i++) {
    std::cout << "[" << std::setw(4) << i << "] = " << std::setw(12) << std::fixed
              << std::setprecision(6) << buffer[i];
    if ((i + 1) % 4 == 0) {
      std::cout << std::endl;
    } else {
      std::cout << "  ";
    }
  }
  if (size % 4 != 0) {
    std::cout << std::endl;
  }
  std::cout << std::string(80, '-') << std::endl;
}

void print_index_buffer(const std::string &name, int16_t *buffer, uint16_t size) {
  std::cout << "\n" << name << " (" << size << " elements):" << std::endl;
  std::cout << std::string(80, '-') << std::endl;

  if (buffer == nullptr) {
    std::cout << "NULL pointer" << std::endl;
    return;
  }

  for (uint16_t i = 0; i < size; i++) {
    std::cout << "[" << std::setw(4) << i << "] = " << std::setw(8) << buffer[i];
    if ((i + 1) % 6 == 0) {
      std::cout << std::endl;
    } else {
      std::cout << "  ";
    }
  }
  if (size % 6 != 0) {
    std::cout << std::endl;
  }
  std::cout << std::string(80, '-') << std::endl;
}

int main() {
  std::cout << "\n" << std::string(80, '=') << std::endl;
  std::cout << "DEBUG: INTERNAL BUFFERS INSPECTION" << std::endl;
  std::cout << std::string(80, '=') << std::endl;

  // Load data
  std::cout << "\nReading input data from test_data.csv..." << std::endl;
  std::vector<float> data = read_csv_data("tests/test_data.csv");

  if (data.empty()) {
    std::cerr << "ERROR: No data loaded from test_data.csv" << std::endl;
    return 1;
  }

  std::cout << "Loaded " << data.size() << " total samples" << std::endl;

  // Configuration
  const uint16_t window_size = 210;
  const uint16_t buffer_size = 5000;
  const uint16_t chunk_size = 500;                // Size of each chunk to process
  const uint16_t num_iterations = 54;              // Number of iterations (change to 3, 4, 5...)

  std::cout << "\n" << std::string(80, '-') << std::endl;
  std::cout << "CONFIGURATION:" << std::endl;
  std::cout << "  Window size: " << window_size << std::endl;
  std::cout << "  Buffer size: " << buffer_size << std::endl;
  std::cout << "  Chunk size: " << chunk_size << std::endl;
  std::cout << "  Number of iterations: " << num_iterations << std::endl;
  std::cout << std::string(80, '-') << std::endl;

  // Initialize Mpx
  std::cout << "\nInitializing Mpx(" << window_size << ", 0.5f, 0, " << buffer_size << ")..." << std::endl;
  MatrixProfile::Mpx mpx(window_size, 0.5f, 0, buffer_size);

  // Process data in chunks
  std::cout << "\nProcessing " << num_iterations << " iterations of " << chunk_size << " values each..." << std::endl;
  for (uint16_t iter = 0; iter < num_iterations; iter++) {
    uint32_t offset = iter * chunk_size;
    std::cout << "\n  Iteration " << (iter + 1) << ": Processing values [" << offset << ":"
              << (offset + chunk_size - 1) << "]" << std::endl;

    if (offset + chunk_size > data.size()) {
      std::cerr << "  WARNING: Not enough data. Stopping at iteration " << (iter + 1) << std::endl;
      break;
    }

    uint16_t result = mpx.compute(&data[offset], chunk_size);
    std::cout << "  compute() returned: " << result << std::endl;
  }

  // Compute FLOSS segmentation
  (void)mpx.floss();


  // Get all buffer information
  std::cout << "\n" << std::string(80, '=') << std::endl;
  std::cout << "BUFFER STATE AFTER PROCESSING" << std::endl;
  std::cout << std::string(80, '=') << std::endl;

  uint16_t buffer_used = mpx.get_buffer_used();
  int16_t buffer_start = mpx.get_buffer_start();
  uint16_t profile_len = mpx.get_profile_len();
  float last_movsum = mpx.get_last_movsum();
  float last_mov2sum = mpx.get_last_mov2sum();

  std::cout << "\nMetadata:" << std::endl;
  std::cout << "  buffer_used: " << buffer_used << std::endl;
  std::cout << "  buffer_start: " << buffer_start << std::endl;
  std::cout << "  profile_len: " << profile_len << std::endl;
  std::cout << "  last_movsum: " << std::fixed << std::setprecision(6) << last_movsum << std::endl;
  std::cout << "  last_mov2sum: " << std::fixed << std::setprecision(6) << last_mov2sum << std::endl;

  // Print all buffers
  std::cout << "\n" << std::string(80, '=') << std::endl;
  std::cout << "INTERNAL BUFFERS CONTENT" << std::endl;
  std::cout << std::string(80, '=') << std::endl;

  // Data buffer (all available data)
  float *data_buffer = mpx.get_data_buffer();
  print_buffer("DATA_BUFFER", data_buffer, buffer_used);

  // Matrix profile
  float *matrix = mpx.get_matrix();
  print_buffer("MATRIX_PROFILE", matrix, profile_len);

  // Profile indexes
  int16_t *indexes = mpx.get_indexes();
  print_index_buffer("PROFILE_INDEXES", indexes, profile_len);

  // FLOSS (if applicable)
  float *floss = mpx.get_floss();
  print_buffer("FLOSS", floss, profile_len);

  // Save FLOSS to CSV file
  std::cout << "\nSaving FLOSS buffer to CSV file..." << std::endl;
  std::ofstream floss_file("floss_output.csv");
  floss_file << "index,floss_value\n";
  for (uint16_t i = 0; i < profile_len; i++) {
    floss_file << i << "," << std::fixed << std::setprecision(6) << floss[i] << "\n";
  }
  floss_file.close();
  std::cout << "✓ FLOSS buffer saved to floss_output.csv (" << profile_len << " values)" << std::endl;

  // Save Profile Indexes to CSV file
  std::cout << "\nSaving PROFILE_INDEXES buffer to CSV file..." << std::endl;
  std::ofstream indexes_file("profile_indexes_output.csv");
  indexes_file << "index,profile_index\n";
  for (uint16_t i = 0; i < profile_len; i++) {
    indexes_file << i << "," << indexes[i] << "\n";
  }
  indexes_file.close();
  std::cout << "✓ Profile indexes saved to profile_indexes_output.csv (" << profile_len << " values)" << std::endl;

  // IAC (Ideal Arc Counts)
  float *iac = mpx.get_iac();
  print_buffer("IAC", iac, profile_len);

  // VMU (moving mean)
  float *vmmu = mpx.get_vmmu();
  print_buffer("VMMU (Moving Mean)", vmmu, profile_len);

  // VSIG (moving std dev)
  float *vsig = mpx.get_vsig();
  print_buffer("VSIG (Moving StdDev)", vsig, profile_len);

  // DDF (first derivative forward)
  float *vddf = mpx.get_ddf();
  print_buffer("VDDF (First Derivative Forward)", vddf, profile_len);

  // DDG (first derivative backward)
  float *vddg = mpx.get_ddg();
  print_buffer("VDDG (First Derivative Backward)", vddg, profile_len);

  std::cout << "\n" << std::string(80, '=') << std::endl;
  std::cout << "DEBUG INSPECTION COMPLETE" << std::endl;
  std::cout << std::string(80, '=') << std::endl << std::endl;

  return 0;
}
