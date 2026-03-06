#include <iostream>
#include <cassert>
#include <cstring>
#include <cmath>
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

// Helper assertion macros (simplified from Unity framework)
#define TEST_ASSERT_EQUAL_UINT16(expected, actual) \
  do { \
    if ((expected) != (actual)) { \
      std::cerr << "  ERROR: " << __FILE__ << ":" << __LINE__ << std::endl; \
      std::cerr << "    Expected: " << (expected) << ", Got: " << (actual) << std::endl; \
      throw std::runtime_error("Assertion failed"); \
    } \
  } while(0)

#define TEST_ASSERT_EQUAL_INT16(expected, actual) \
  do { \
    if ((expected) != (actual)) { \
      std::cerr << "  ERROR: " << __FILE__ << ":" << __LINE__ << std::endl; \
      std::cerr << "    Expected: " << (expected) << ", Got: " << (actual) << std::endl; \
      throw std::runtime_error("Assertion failed"); \
    } \
  } while(0)

#define TEST_ASSERT_FLOAT_WITHIN(tolerance, expected, actual) \
  do { \
    float diff = std::abs((expected) - (actual)); \
    if (diff > (tolerance)) { \
      std::cerr << "  ERROR: " << __FILE__ << ":" << __LINE__ << std::endl; \
      std::cerr << "    Expected: " << (expected) << " ±" << (tolerance) << ", Got: " << (actual) << std::endl; \
      throw std::runtime_error("Assertion failed"); \
    } \
  } while(0)

#define TEST_ASSERT_TRUE(condition) \
  do { \
    if (!(condition)) { \
      std::cerr << "  ERROR: " << __FILE__ << ":" << __LINE__ << std::endl; \
      std::cerr << "    Condition failed: " << #condition << std::endl; \
      throw std::runtime_error("Assertion failed"); \
    } \
  } while(0)


// ============================================================================
// TEST 1: Constructor Initialization and Initial State
// ============================================================================
/**
 * VALIDATION:
 * - buffer_used_ initialized to buffer_size
 * - buffer_start_ initialized to 0
 * - profile_len correctly computed: buffer_size - window_size + 1
 * - Matrix profile array initialized to -1000000.0F (sentinel)
 * - Profile index array initialized to -1 (no match found)
 */
void test_mpx_constructor_initial_state() {
  std::cout << "\n=== Test 1: Constructor Initial State ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    // CHECK 1: Buffer tracking state
    TEST_ASSERT_EQUAL_UINT16(buffer_size, mpx.get_buffer_used());
    print_test_result("Buffer used equals buffer_size", true);

    // CHECK 2: Buffer read position
    TEST_ASSERT_EQUAL_INT16(0, mpx.get_buffer_start());
    print_test_result("Buffer start is 0", true);

    // CHECK 3: Profile length calculation
    const uint16_t expected_profile_len = buffer_size - window_size + 1U;
    TEST_ASSERT_EQUAL_UINT16(expected_profile_len, mpx.get_profile_len());
    print_test_result("Profile length correctly calculated", true);

    // CHECK 4: Matrix profile initialization (sentinel values)
    float *matrix = mpx.get_matrix();
    int16_t *indexes = mpx.get_indexes();
    const uint16_t profile_len = mpx.get_profile_len();

    bool all_matrix_init_correct = true;
    bool all_indexes_init_correct = true;

    for (uint16_t i = 0U; i < profile_len; i++) {
      if (std::abs(matrix[i] - (-1000000.0F)) > 0.001F) {
        all_matrix_init_correct = false;
        break;
      }
      if (indexes[i] != -1) {
        all_indexes_init_correct = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_matrix_init_correct);
    print_test_result("Matrix initialized to sentinel (-1000000.0)", all_matrix_init_correct);

    TEST_ASSERT_TRUE(all_indexes_init_correct);
    print_test_result("Indexes initialized to -1 (no match)", all_indexes_init_correct);
  } catch (const std::exception &e) {
    print_test_result("Constructor initial state test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

// ============================================================================
// TEST 2: Buffer Pruning and State Consistency
// ============================================================================
/**
 * VALIDATION:
 * - prune_buffer() initializes data correctly
 * - Internal buffers (ddf, ddg) are computed
 * - Numerical stability maintained (finite values)
 * - State consistency after pruning
 */
void test_mpx_prune_buffer_invariants() {
  std::cout << "\n=== Test 2: Prune Buffer Invariants ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    // Explicitly call prune_buffer to ensure clean state
    mpx.prune_buffer();

    const uint16_t profile_len = mpx.get_profile_len();
    float *data = mpx.get_data_buffer();
    float *ddf = mpx.get_ddf();
    float *ddg = mpx.get_ddg();

    // CHECK 1: Data buffer sanity (sin(0) = 0 for sinusoidal pattern)
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 0.0F, data[0]);
    print_test_result("Data buffer first element is 0 (sin(0))", true);

    // CHECK 2: Buffer state consistency
    TEST_ASSERT_EQUAL_UINT16(buffer_size, mpx.get_buffer_used());
    TEST_ASSERT_EQUAL_INT16(0, mpx.get_buffer_start());
    print_test_result("Buffer state consistent after prune", true);

    // CHECK 3: Differential arrays are computed
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 0.0F, ddf[profile_len - 1U]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001F, 0.0F, ddg[profile_len - 1U]);
    print_test_result("Differential arrays (ddf, ddg) computed", true);

    // CHECK 4: Internal accumulators are finite (no NaN, Inf)
    bool movsum_finite = std::isfinite(mpx.get_last_movsum());
    bool mov2sum_finite = std::isfinite(mpx.get_last_mov2sum());

    TEST_ASSERT_TRUE(movsum_finite);
    TEST_ASSERT_TRUE(mov2sum_finite);
    print_test_result("Internal accumulators are finite (no NaN/Inf)", movsum_finite && mov2sum_finite);
  } catch (const std::exception &e) {
    print_test_result("Prune buffer invariants test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

// ============================================================================
// TEST 3: Compute and FLOSS Output Validity (MAIN FUNCTIONAL TEST)
// ============================================================================
/**
 * VALIDATION:
 * - compute() produces valid matrix profile correlations
 * - floss() generates valid goodness-of-fit scores
 * - Nearest neighbors are found and stored in indexes
 * - FLOSS edge values follow expected behavior
 */
void test_mpx_compute_and_floss_produce_valid_output() {
  std::cout << "\n=== Test 3: Compute and FLOSS Functional Test ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    // Generate synthetic signal: sine + trend
    // This has structure to produce interesting matrix profile
    float input[16];
    for (uint16_t i = 0U; i < 16U; i++) {
      input[i] = std::sin(static_cast<float>(i) * 0.2F) + (static_cast<float>(i) * 0.05F);
    }

    // SUBSTEP 1: Run matrix profile computation
    uint16_t free_space = mpx.compute(input, 16U);
    TEST_ASSERT_EQUAL_UINT16(0U, free_space);
    print_test_result("Compute() processes data and fills buffer", true);

    // SUBSTEP 2: Compute FLOSS (goodness of fit) scores
    mpx.floss();
    print_test_result("FLOSS computation runs without error", true);

    // SUBSTEP 3: Retrieve outputs
    const uint16_t profile_len = mpx.get_profile_len();
    float *matrix = mpx.get_matrix();
    int16_t *indexes = mpx.get_indexes();
    float *floss = mpx.get_floss();

    // CHECK 1: At least some matches were found
    bool has_valid_match = false;
    for (uint16_t i = 0U; i < profile_len; i++) {
      if (indexes[i] >= 0 && matrix[i] > -999999.0F) {
        has_valid_match = true;
        break;
      }
    }
    TEST_ASSERT_TRUE(has_valid_match);
    print_test_result("At least one valid nearest neighbor found", has_valid_match);

    // CHECK 2: All FLOSS values are finite (no NaN, Inf)
    bool all_floss_finite = true;
    for (uint16_t i = 0U; i < profile_len; i++) {
      if (!std::isfinite(floss[i])) {
        all_floss_finite = false;
        break;
      }
    }
    TEST_ASSERT_TRUE(all_floss_finite);
    print_test_result("All FLOSS values are finite", all_floss_finite);

    // CHECK 3: First window_size positions should have FLOSS ≈ 1.0 (boundary effect)
    bool first_floss_correct = true;
    for (uint16_t i = 0U; i < window_size && i < profile_len; i++) {
      if (std::abs(floss[i] - 1.0F) > 0.0001F) {
        first_floss_correct = false;
        break;
      }
    }
    TEST_ASSERT_TRUE(first_floss_correct);
    print_test_result("First window_size FLOSS values ≈ 1.0 (edge case)", first_floss_correct);

    // CHECK 4: Last window_size-1 positions should also have FLOSS ≈ 1.0
    bool last_floss_correct = true;
    for (uint16_t i = (profile_len > window_size) ? (profile_len - window_size + 1U) : 0U;
         i < (profile_len > 0U ? profile_len - 1U : 0U); i++) {
      if (std::abs(floss[i] - 1.0F) > 0.0001F) {
        last_floss_correct = false;
        break;
      }
    }
    TEST_ASSERT_TRUE(last_floss_correct);
    print_test_result("Last window_size-1 FLOSS values ≈ 1.0 (edge case)", last_floss_correct);
  } catch (const std::exception &e) {
    print_test_result("Compute and FLOSS functional test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

int main() {
  std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║      Unit Tests - Mpx Library (Unity-Style)        ║" << std::endl;
  std::cout << "║      Matrix Profile for Time Series                ║" << std::endl;
  std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;

  try {
    // Run the comprehensive tests from Unity framework
    test_mpx_constructor_initial_state();
    test_mpx_prune_buffer_invariants();
    test_mpx_compute_and_floss_produce_valid_output();

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
