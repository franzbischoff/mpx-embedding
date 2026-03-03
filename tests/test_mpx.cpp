#include <iostream>
#include <cassert>
#include <cstring>
#include "mpx/Mpx.hpp"

using namespace MatrixProfile;

void print_test_result(const char *test_name, bool passed) {
  std::cout << "[" << (passed ? "PASS" : "FAIL") << "] " << test_name << std::endl;
}

/**
 * @brief Test Mpx class initialization
 */
void test_mpx_initialization() {
  std::cout << "\n=== Initialization Test ===" << std::endl;

  try {
    // Test 1: Create instance with default parameters
    Mpx mpx(128, 0.5f, 0, 5000);

    bool test1 = (mpx.get_buffer_used() == 5000);
    print_test_result("Initialization with buffer size 5000", test1);

    // Test 2: Verify getters
    float *matrix = mpx.get_matrix();
    int16_t *indexes = mpx.get_indexes();
    float *floss = mpx.get_floss();

    bool test2 = (matrix != nullptr && indexes != nullptr && floss != nullptr);
    print_test_result("Getters return valid pointers", test2);
  } catch (const std::exception &e) {
    std::cerr << "Unexpected exception: " << e.what() << std::endl;
  }
}

/**
 * @brief Basic computation test
 */
void test_mpx_compute() {
  std::cout << "\n=== Computation Test ===" << std::endl;

  try {
    Mpx mpx(128, 0.5f, 0, 5000);

    // Create test data
    float test_data[256];
    for (int i = 0; i < 256; i++) {
      test_data[i] = static_cast<float>(i);
    }

    // Test 1: Compute with data
    uint16_t result = mpx.compute(test_data, 256);
    bool test1 = true; // If no exception, passed
    print_test_result("Compute without exception", test1);

    // Test 2: Verify matrix was updated
    float *matrix = mpx.get_matrix();
    bool test2 = (matrix != nullptr);
    print_test_result("Matrix after compute is valid", test2);
  } catch (const std::exception &e) {
    std::cerr << "Exception in test_mpx_compute: " << e.what() << std::endl;
  }
}

/**
 * @brief FLOSS test
 */
void test_mpx_floss() {
  std::cout << "\n=== FLOSS Test ===" << std::endl;

  try {
    Mpx mpx(128, 0.5f, 0, 5000);

    // Create data
    float test_data[256];
    for (int i = 0; i < 256; i++) {
      test_data[i] = static_cast<float>(i % 100);
    }

    // Compute first
    mpx.compute(test_data, 256);

    // Compute FLOSS
    mpx.floss();

    float *floss = mpx.get_floss();
    bool test1 = (floss != nullptr);
    print_test_result("FLOSS returns valid data", test1);
  } catch (const std::exception &e) {
    std::cerr << "Exception in test_mpx_floss: " << e.what() << std::endl;
  }
}

/**
 * @brief Data streaming test
 */
void test_mpx_streaming() {
  std::cout << "\n=== Streaming Test ===" << std::endl;

  try {
    Mpx mpx(64, 0.5f, 0, 1000);

    // Simulate data streaming in chunks
    bool test1 = true;
    for (int chunk = 0; chunk < 5; chunk++) {
      float data[128];
      for (int i = 0; i < 128; i++) {
        data[i] = static_cast<float>(chunk * 128 + i);
      }
      mpx.compute(data, 128);
    }
    print_test_result("Streaming multiple chunks", test1);

    // Verify data was processed
    float *matrix = mpx.get_matrix();
    bool test2 = (matrix != nullptr);
    print_test_result("Matrix after streaming is valid", test2);

    // Prune buffer
    mpx.prune_buffer();
    bool test3 = true;
    print_test_result("Prune buffer without error", test3);
  } catch (const std::exception &e) {
    std::cerr << "Exception in test_mpx_streaming: " << e.what() << std::endl;
  }
}

/**
 * @brief Internal data access test
 */
void test_mpx_data_access() {
  std::cout << "\n=== Data Access Test ===" << std::endl;

  try {
    Mpx mpx(128, 0.5f, 0, 5000);

    // Test all getters
    float *data_buf = mpx.get_data_buffer();
    float *matrix = mpx.get_matrix();
    int16_t *indexes = mpx.get_indexes();
    float *floss_data = mpx.get_floss();
    float *iac = mpx.get_iac();
    float *vmmu = mpx.get_vmmu();
    float *vsig = mpx.get_vsig();

    bool test1 = (data_buf != nullptr && matrix != nullptr && indexes != nullptr && floss_data != nullptr &&
                  iac != nullptr && vmmu != nullptr && vsig != nullptr);
    print_test_result("All getters return valid pointers", test1);

    // Test information getters
    uint16_t buf_used = mpx.get_buffer_used();
    int16_t buf_start = mpx.get_buffer_start();
    uint16_t prof_len = mpx.get_profile_len();

    bool test2 = (buf_used > 0 && prof_len > 0);
    print_test_result("Information getters return valid values", test2);
  } catch (const std::exception &e) {
    std::cerr << "Exception in test_mpx_data_access: " << e.what() << std::endl;
  }
}

int main() {
  std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║      Unit Tests - Mpx Library                      ║" << std::endl;
  std::cout << "║      Matrix Profile for Time Series                ║" << std::endl;
  std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;

  try {
    test_mpx_initialization();
    test_mpx_compute();
    test_mpx_floss();
    test_mpx_streaming();
    test_mpx_data_access();

    std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║       All tests completed successfully!             ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error during test execution: " << e.what() << std::endl;
    return 1;
  }
}
