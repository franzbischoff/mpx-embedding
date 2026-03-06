/**
 * @file test_mpx_golden.cpp
 * @brief Golden reference regression test for Mpx library
 *
 * This test validates that the Mpx implementation produces identical results
 * to a previously validated "golden reference" CSV file.
 *
 * Test Process:
 * 1. Load test data (test_data.csv)
 * 2. Process using same parameters as golden reference generation
 * 3. Load golden reference (golden_reference.csv)
 * 4. Compare all buffer values with tolerance
 * 5. Report any discrepancies
 *
 * Parameters:
 *   - window_size: 210
 *   - buffer_size: 5000
 *   - chunk_size: 500
 *   - num_iterations: 54
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <map>
#include <sstream>
#include "mpx/Mpx.hpp"

using namespace MatrixProfile;

// ============================================================================
// TEST FRAMEWORK UTILITIES
// ============================================================================

int g_test_count = 0;
int g_test_failed = 0;

void print_test_result(const char *test_name, bool passed) {
  g_test_count++;
  if (!passed) g_test_failed++;
  std::cout << "[" << (passed ? "PASS" : "FAIL") << "] " << test_name << std::endl;
}

void print_test_message(const char *message) {
  std::cout << "  INFO: " << message << std::endl;
}

#define TEST_ASSERT_TRUE(condition) \
  do { \
    if (!(condition)) { \
      std::cerr << "  ERROR: " << __FILE__ << ":" << __LINE__ << std::endl; \
      std::cerr << "    Condition failed: " << #condition << std::endl; \
      throw std::runtime_error("Assertion failed"); \
    } \
  } while(0)

// ============================================================================
// CSV UTILITIES
// ============================================================================

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
      } catch (const std::exception &) {
        // Skip invalid lines
      }
    }
  }

  file.close();
  return data;
}

struct GoldenEntry {
  std::string buffer_type;
  uint16_t index;
  float value_float;
  int16_t value_int;
};

std::vector<GoldenEntry> read_golden_reference(const std::string &filename) {
  std::vector<GoldenEntry> entries;
  std::ifstream file(filename);

  if (!file.is_open()) {
    std::cerr << "ERROR: Could not open " << filename << std::endl;
    return entries;
  }

  std::string line;
  bool skip_first = true;

  while (std::getline(file, line)) {
    if (skip_first) {
      skip_first = false;
      continue;
    }

    std::stringstream ss(line);
    std::string buffer_type, index_str, value_float_str, value_int_str;

    if (std::getline(ss, buffer_type, ',') &&
        std::getline(ss, index_str, ',') &&
        std::getline(ss, value_float_str, ',') &&
        std::getline(ss, value_int_str, ',')) {

      GoldenEntry entry;
      entry.buffer_type = buffer_type;

      // For metadata, index_str might be a string (e.g., "buffer_used")
      // For regular entries, it's a number
      try {
        entry.index = static_cast<uint16_t>(std::stoi(index_str));
      } catch (...) {
        // It's metadata with string index - store as hash
        entry.index = 0;
      }

      entry.value_float = std::stof(value_float_str);
      entry.value_int = static_cast<int16_t>(std::stoi(value_int_str));

      entries.push_back(entry);
    }
  }

  file.close();
  return entries;
}

// ============================================================================
// GOLDEN REFERENCE TEST
// ============================================================================

void test_golden_reference_validation() {
  std::cout << "\n=== Golden Reference Regression Test ===" << std::endl;

  try {
    // Load test data
    std::vector<float> data = read_csv_data("tests/test_data.csv");
    TEST_ASSERT_TRUE(!data.empty());
    print_test_message("Test data loaded");

    // Configuration (must match golden reference generator)
    const uint16_t window_size = 210;
    const uint16_t buffer_size = 5000;
    const uint16_t chunk_size = 500;
    const uint16_t num_iterations = 54;

    // Initialize and process
    Mpx mpx(window_size, 0.5f, 0, buffer_size);

    for (uint16_t iter = 0; iter < num_iterations; iter++) {
      uint32_t offset = iter * chunk_size;
      if (offset + chunk_size > data.size()) {
        break;
      }
      (void)mpx.compute(&data[offset], chunk_size);
    }

    mpx.floss();
    print_test_message("Data processed");

    // Load golden reference
    std::vector<GoldenEntry> golden = read_golden_reference("tests/golden_reference.csv");
    TEST_ASSERT_TRUE(!golden.empty());

    char msg[100];
    snprintf(msg, sizeof(msg), "Golden reference loaded (%zu entries)", golden.size());
    print_test_message(msg);

    // Get current buffers
    float *data_buffer = mpx.get_data_buffer();
    float *matrix = mpx.get_matrix();
    int16_t *indexes = mpx.get_indexes();
    float *floss = mpx.get_floss();
    float *iac = mpx.get_iac();
    float *vmmu = mpx.get_vmmu();
    float *vsig = mpx.get_vsig();
    float *vddf = mpx.get_ddf();
    float *vddg = mpx.get_ddg();

    // Validate against golden reference
    const float TOLERANCE = 1e-6f;
    uint32_t mismatches = 0;
    uint32_t compared = 0;

    for (const auto &entry : golden) {
      float actual_float = 0.0f;
      int16_t actual_int = 0;
      bool should_compare = true;

      // Get actual values based on buffer type
      if (entry.buffer_type == "metadata") {
        // Metadata validation is more complex, skip for now
        continue;
      } else if (entry.buffer_type == "data_buffer") {
        actual_float = data_buffer[entry.index];
      } else if (entry.buffer_type == "matrix_profile") {
        actual_float = matrix[entry.index];
      } else if (entry.buffer_type == "profile_indexes") {
        actual_int = indexes[entry.index];
      } else if (entry.buffer_type == "floss") {
        actual_float = floss[entry.index];
      } else if (entry.buffer_type == "iac") {
        actual_float = iac[entry.index];
      } else if (entry.buffer_type == "vmmu") {
        actual_float = vmmu[entry.index];
      } else if (entry.buffer_type == "vsig") {
        actual_float = vsig[entry.index];
      } else if (entry.buffer_type == "vddf") {
        actual_float = vddf[entry.index];
      } else if (entry.buffer_type == "vddg") {
        actual_float = vddg[entry.index];
      } else {
        should_compare = false;
      }

      if (!should_compare) continue;

      compared++;

      // Compare values
      if (entry.buffer_type == "profile_indexes") {
        if (actual_int != entry.value_int) {
          mismatches++;
          if (mismatches <= 5) { // Only print first 5 mismatches
            char err[150];
            snprintf(err, sizeof(err), "Mismatch in %s[%u]: expected %d, got %d",
                     entry.buffer_type.c_str(), entry.index, entry.value_int, actual_int);
            std::cerr << "  " << err << std::endl;
          }
        }
      } else {
        float diff = std::abs(actual_float - entry.value_float);
        if (diff > TOLERANCE) {
          mismatches++;
          if (mismatches <= 5) { // Only print first 5 mismatches
            char err[150];
            snprintf(err, sizeof(err), "Mismatch in %s[%u]: expected %.10f, got %.10f (diff: %.10f)",
                     entry.buffer_type.c_str(), entry.index, entry.value_float, actual_float, diff);
            std::cerr << "  " << err << std::endl;
          }
        }
      }
    }

    // Report results
    snprintf(msg, sizeof(msg), "Compared %u values, found %u mismatches", compared, mismatches);
    print_test_message(msg);

    TEST_ASSERT_TRUE(mismatches == 0);
    print_test_result("All buffer values match golden reference", mismatches == 0);

  } catch (const std::exception &e) {
    print_test_result("Golden reference validation test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
  std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║   Golden Reference Test - Mpx Library              ║" << std::endl;
  std::cout << "║   Regression Testing for Consistency               ║" << std::endl;
  std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;

  try {
    test_golden_reference_validation();

    std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║              Test Summary                          ║" << std::endl;
    std::cout << "║  Total Tests: " << g_test_count << std::endl;
    std::cout << "║  Passed: " << (g_test_count - g_test_failed) << std::endl;
    std::cout << "║  Failed: " << g_test_failed << std::endl;

    if (g_test_failed == 0) {
      std::cout << "║  Status: ALL TESTS PASSED ✓                        ║" << std::endl;
      std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;
      return 0;
    } else {
      std::cout << "║  Status: SOME TESTS FAILED ✗                       ║" << std::endl;
      std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "\nFATAL ERROR: " << e.what() << std::endl;
    std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║              Test Summary                          ║" << std::endl;
    std::cout << "║  Total Tests: " << g_test_count << std::endl;
    std::cout << "║  Failed: " << g_test_failed << std::endl;
    std::cout << "║  Status: TEST SUITE CRASHED ✗                     ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;
    return 1;
  }
}
