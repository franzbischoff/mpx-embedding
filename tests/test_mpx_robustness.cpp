/**
 * @file test_mpx_robustness.cpp
 * @brief Robustness test suite for Matrix Profile Streaming (Mpx) implementation
 *
 * This file contains extensive robustness and invariant validation tests for the Mpx class.
 * Unlike test_mpx.cpp (which tests basic functionality), these tests validate:
 * - Numerical stability under various signal conditions
 * - Edge cases and boundary conditions
 * - Output invariants (finite values, valid ranges)
 * - Sequential processing reliability
 *
 * Test Organization:
 * - HELPER FIXTURES: TestSignalGenerator class for reproducible test signals
 * - NUMERICAL STABILITY: Verify finite outputs from movmean, movsig, differentials
 * - MATRIX PROFILE INVARIANTS: Validate MP values and indices are within expected ranges
 * - FLOSS OUTPUT SANITY: Check FLOSS returns finite, reasonable values
 * - EDGE CASES: Test minimal buffers, extreme parameters
 * - DATA PATTERN ROBUSTNESS: Constant signals, noise, mixed patterns
 * - SEQUENTIAL PROCESSING: Multiple compute() calls in succession
 */

#include <iostream>
#include <cassert>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
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
// HELPER FIXTURES & GENERATORS
// ============================================================================

/**
 * @class TestSignalGenerator
 * @brief Utility class for generating reproducible test signals
 *
 * Provides 6 different signal patterns for comprehensive testing:
 * - SINE_WAVE: Smooth periodic signal for typical time series
 * - LINEAR_TREND: Monotonic increasing signal
 * - CONSTANT: Zero-variance signal (tests numerical stability)
 * - RANDOM_UNIFORM: Uniform random noise
 * - STEP_FUNCTION: Abrupt changes (tests segmentation detection)
 * - NOISE: Gaussian-like noise (tests robustness to irregularity)
 */
class TestSignalGenerator {
public:
  enum Pattern { SINE_WAVE, LINEAR_TREND, CONSTANT, RANDOM_UNIFORM, STEP_FUNCTION, NOISE };

  static std::vector<float> generate(Pattern pattern, uint16_t length, float amplitude = 1.0f, float frequency = 0.1f) {
    std::vector<float> signal(length);

    switch (pattern) {
    case SINE_WAVE:
      for (uint16_t i = 0; i < length; i++) {
        signal[i] = amplitude * std::sin(frequency * static_cast<float>(i));
      }
      break;

    case LINEAR_TREND:
      for (uint16_t i = 0; i < length; i++) {
        signal[i] = amplitude * static_cast<float>(i) / static_cast<float>(length);
      }
      break;

    case CONSTANT:
      for (uint16_t i = 0; i < length; i++) {
        signal[i] = amplitude;
      }
      break;

    case STEP_FUNCTION:
      for (uint16_t i = 0; i < length; i++) {
        signal[i] = (i < length / 2) ? 0.0f : amplitude;
      }
      break;

    case NOISE:
      for (uint16_t i = 0; i < length; i++) {
        signal[i] = amplitude * static_cast<float>(rand() % 100 - 50) / 50.0f;
      }
      break;

    case RANDOM_UNIFORM:
      for (uint16_t i = 0; i < length; i++) {
        signal[i] = amplitude * (static_cast<float>(rand() % 100) / 100.0f - 0.5f);
      }
      break;
    }

    return signal;
  }
};

// ============================================================================
// TEST SUITE: Basic Numerical Stability
// ============================================================================

/**
 * @test test_movmean_returns_finite_values
 * @brief Verify moving mean computation produces finite outputs
 */
void test_movmean_returns_finite_values() {
  std::cout << "\n=== Test: Moving Mean Returns Finite Values ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::SINE_WAVE, 32, 1.0f, 0.1f);
    (void)mpx.compute(signal.data(), 32U);

    float *mmu = mpx.get_vmmu();
    uint16_t profile_len = mpx.get_profile_len();

    bool all_finite = true;
    for (uint16_t i = 0; i < profile_len; i++) {
      if (!std::isfinite(mmu[i])) {
        all_finite = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_finite);
    print_test_result("All moving mean values are finite", all_finite);
  } catch (const std::exception &e) {
    print_test_result("Moving mean finite values test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

/**
 * @test test_movsig_returns_valid_values
 * @brief Verify moving standard deviation produces valid outputs
 */
void test_movsig_returns_valid_values() {
  std::cout << "\n=== Test: Moving Sigma Returns Valid Values ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::SINE_WAVE, 32, 1.0f, 0.1f);
    (void)mpx.compute(signal.data(), 32U);

    float *sig = mpx.get_vsig();
    uint16_t profile_len = mpx.get_profile_len();

    uint16_t invalid_count = 0;
    bool all_valid = true;
    for (uint16_t i = 0; i < profile_len; i++) {
      if (sig[i] == -1.0f) {
        invalid_count++;
      }
      // Must be either -1.0 (invalid/constant window) or positive finite
      if (!((sig[i] == -1.0f) || (sig[i] > 0.0f && std::isfinite(sig[i])))) {
        all_valid = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_valid);
    print_test_result("All moving sigma values are valid (-1.0 or positive finite)", all_valid);

    char msg[80];
    snprintf(msg, sizeof(msg), "Invalid windows (sig=-1.0): %u / %u", invalid_count, profile_len);
    print_test_message(msg);
  } catch (const std::exception &e) {
    print_test_result("Moving sigma valid values test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

/**
 * @test test_differential_arrays_are_finite
 * @brief Verify differential arrays (ddf, ddg) contain finite values
 */
void test_differential_arrays_are_finite() {
  std::cout << "\n=== Test: Differential Arrays Are Finite ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::LINEAR_TREND, 32, 1.0f);
    (void)mpx.compute(signal.data(), 32U);

    float *ddf = mpx.get_ddf();
    float *ddg = mpx.get_ddg();
    uint16_t profile_len = mpx.get_profile_len();

    bool all_finite = true;
    for (uint16_t i = 0; i < profile_len; i++) {
      if (!std::isfinite(ddf[i]) || !std::isfinite(ddg[i])) {
        all_finite = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_finite);
    print_test_result("All differential array values (ddf, ddg) are finite", all_finite);
  } catch (const std::exception &e) {
    print_test_result("Differential arrays finite test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

// ============================================================================
// TEST SUITE: Matrix Profile Invariants
// ============================================================================

/**
 * @test test_mp_profile_values_bounded
 * @brief Verify matrix profile values are within valid correlation range
 */
void test_mp_profile_values_bounded() {
  std::cout << "\n=== Test: Matrix Profile Values Bounded ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::SINE_WAVE, 32, 1.0f, 0.1f);
    (void)mpx.compute(signal.data(), 32U);

    float *matrix = mpx.get_matrix();
    uint16_t profile_len = mpx.get_profile_len();

    uint16_t uninitialized_count = 0;
    uint16_t computed_count = 0;
    bool all_bounded = true;

    for (uint16_t i = 0; i < profile_len; i++) {
      float val = matrix[i];
      if (val <= -999000.0f) {
        uninitialized_count++;
      } else {
        computed_count++;
      }
      if (!(val <= -999000.0f || (val >= -1.0f && val <= 1.0f))) {
        all_bounded = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_bounded);
    print_test_result("All matrix profile values are bounded [-1, 1] or uninitialized", all_bounded);

    char msg[80];
    snprintf(msg, sizeof(msg), "Matrix Profile: %u computed, %u uninitialized (of %u)",
             computed_count, uninitialized_count, profile_len);
    print_test_message(msg);
  } catch (const std::exception &e) {
    print_test_result("Matrix profile values bounded test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

/**
 * @test test_mp_indexes_valid_or_empty
 * @brief Verify matrix profile indices are valid array positions
 */
void test_mp_indexes_valid_or_empty() {
  std::cout << "\n=== Test: Matrix Profile Indexes Valid ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::SINE_WAVE, 32, 1.0f, 0.1f);
    (void)mpx.compute(signal.data(), 32U);

    int16_t *indexes = mpx.get_indexes();
    uint16_t profile_len = mpx.get_profile_len();

    uint16_t invalid_count = 0;
    uint16_t valid_count = 0;
    bool all_valid = true;

    for (uint16_t i = 0; i < profile_len; i++) {
      int16_t idx = indexes[i];
      if (idx < 0) {
        invalid_count++;
      } else {
        valid_count++;
      }
      if (!((idx < 0) || (idx >= 0 && idx < static_cast<int16_t>(profile_len)))) {
        all_valid = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_valid);
    print_test_result("All profile indices are valid or negative (invalid)", all_valid);

    char msg[80];
    snprintf(msg, sizeof(msg), "Profile Indices: %u valid, %u invalid (of %u)",
             valid_count, invalid_count, profile_len);
    print_test_message(msg);
  } catch (const std::exception &e) {
    print_test_result("Matrix profile indexes valid test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

// ============================================================================
// TEST SUITE: FLOSS Output Sanity
// ============================================================================

/**
 * @test test_floss_returns_finite_values
 * @brief Verify FLOSS corrected arc counts are finite
 */
void test_floss_returns_finite_values() {
  std::cout << "\n=== Test: FLOSS Returns Finite Values ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::SINE_WAVE, 32, 1.0f, 0.1f);
    (void)mpx.compute(signal.data(), 32U);
    mpx.floss();

    float *floss = mpx.get_floss();
    uint16_t profile_len = mpx.get_profile_len();

    bool all_finite = true;
    for (uint16_t i = 0; i < profile_len; i++) {
      if (!std::isfinite(floss[i])) {
        all_finite = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_finite);
    print_test_result("All FLOSS values are finite", all_finite);
  } catch (const std::exception &e) {
    print_test_result("FLOSS finite values test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

/**
 * @test test_floss_values_in_reasonable_range
 * @brief Verify FLOSS values are within reasonable bounds
 */
void test_floss_values_in_reasonable_range() {
  std::cout << "\n=== Test: FLOSS Values In Reasonable Range ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::SINE_WAVE, 32, 1.0f, 0.1f);
    (void)mpx.compute(signal.data(), 32U);
    mpx.floss();

    float *floss = mpx.get_floss();
    uint16_t profile_len = mpx.get_profile_len();

    bool all_reasonable = true;
    for (uint16_t i = 0; i < profile_len; i++) {
      if (!std::isfinite(floss[i]) || floss[i] < -10000.0f || floss[i] > 10000.0f) {
        all_reasonable = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_reasonable);
    print_test_result("All FLOSS values are in reasonable range [-10000, 10000]", all_reasonable);
  } catch (const std::exception &e) {
    print_test_result("FLOSS reasonable range test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

// ============================================================================
// TEST SUITE: Edge Cases & Robustness
// ============================================================================

/**
 * @test test_minimal_buffer_size
 * @brief Verify smallest valid configuration works
 */
void test_minimal_buffer_size() {
  std::cout << "\n=== Test: Minimal Buffer Size ===" << std::endl;

  try {
    const uint16_t window_size = 2U;
    const uint16_t buffer_size = 4U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    float input[2] = {1.0f, 2.0f};
    uint16_t free_space = mpx.compute(input, 2U);

    TEST_ASSERT_TRUE(free_space <= buffer_size);
    TEST_ASSERT_EQUAL_UINT16(3U, mpx.get_profile_len());
    print_test_result("Minimal buffer size (window=2, buffer=4) works", true);
  } catch (const std::exception &e) {
    print_test_result("Minimal buffer size test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

/**
 * @test test_large_exclusion_zone
 * @brief Verify high exclusion zone parameter does not crash
 */
void test_large_exclusion_zone() {
  std::cout << "\n=== Test: Large Exclusion Zone ===" << std::endl;

  try {
    const uint16_t window_size = 32U;
    const uint16_t buffer_size = 256U;
    Mpx mpx(window_size, 0.9F, 0U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::LINEAR_TREND, 128, 1.0f);
    (void)mpx.compute(signal.data(), 128U);

    TEST_ASSERT_TRUE(mpx.get_profile_len() > 0);
    print_test_result("Large exclusion zone (ez=0.9) does not crash", true);
  } catch (const std::exception &e) {
    print_test_result("Large exclusion zone test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

/**
 * @test test_time_constraint_accepted
 * @brief Verify time constraint parameter is accepted
 */
void test_time_constraint_accepted() {
  std::cout << "\n=== Test: Time Constraint Accepted ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 20U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::SINE_WAVE, 32, 1.0f, 0.1f);
    (void)mpx.compute(signal.data(), 32U);

    TEST_ASSERT_TRUE(mpx.get_profile_len() > 0);
    print_test_result("Time constraint parameter (tc=20) is accepted", true);
  } catch (const std::exception &e) {
    print_test_result("Time constraint test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

// ============================================================================
// TEST SUITE: Data Pattern Robustness
// ============================================================================

/**
 * @test test_constant_signal_no_crash
 * @brief Verify constant (zero-variance) signal does not crash
 */
void test_constant_signal_no_crash() {
  std::cout << "\n=== Test: Constant Signal No Crash ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    float input[32];
    for (uint16_t i = 0; i < 32; i++) {
      input[i] = 5.0f;
    }

    (void)mpx.compute(input, 32U);
    (void)mpx.compute(input, 32U);
    mpx.floss();

    float *sig = mpx.get_vsig();
    uint16_t profile_len = mpx.get_profile_len();
    uint16_t invalid_windows = 0;
    for (uint16_t i = 0; i < profile_len; i++) {
      if (sig[i] == -1.0f) {
        invalid_windows++;
      }
    }

    char msg[80];
    snprintf(msg, sizeof(msg), "Constant signal: %u / %u windows invalid (expected: all)",
             invalid_windows, profile_len);
    print_test_message(msg);

    float *floss = mpx.get_floss();
    bool all_finite = true;
    for (uint16_t i = 0; i < profile_len; i++) {
      if (!std::isfinite(floss[i])) {
        all_finite = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_finite);
    print_test_result("Constant signal produces finite FLOSS values", all_finite);
  } catch (const std::exception &e) {
    print_test_result("Constant signal test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

/**
 * @test test_mixed_pattern_signal
 * @brief Verify mixed signal patterns are handled correctly
 */
void test_mixed_pattern_signal() {
  std::cout << "\n=== Test: Mixed Pattern Signal ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    auto sine = TestSignalGenerator::generate(TestSignalGenerator::SINE_WAVE, 32, 1.0f, 0.2f);
    auto trend = TestSignalGenerator::generate(TestSignalGenerator::LINEAR_TREND, 32, 1.0f);

    (void)mpx.compute(sine.data(), 32U);
    (void)mpx.compute(trend.data(), 32U);

    float *matrix = mpx.get_matrix();
    uint16_t profile_len = mpx.get_profile_len();

    uint16_t valid_values = 0;
    for (uint16_t i = 0; i < profile_len; i++) {
      if (matrix[i] > -999000.0f) {
        valid_values++;
      }
    }

    char msg[80];
    snprintf(msg, sizeof(msg), "Mixed pattern: %u / %u profile values computed",
             valid_values, profile_len);
    print_test_message(msg);

    char dbg[100];
    snprintf(dbg, sizeof(dbg), "  Samples: matrix[0]=%.2f, matrix[20]=%.2f, matrix[%d]=%.2f",
             matrix[0], matrix[20], (profile_len > 50 ? 50 : profile_len - 1),
             matrix[profile_len > 50 ? 50 : profile_len - 1]);
    print_test_message(dbg);

    TEST_ASSERT_TRUE(valid_values > 0);
    print_test_result("Mixed pattern signal produces valid matrix profile", valid_values > 0);
  } catch (const std::exception &e) {
    print_test_result("Mixed pattern signal test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

/**
 * @test test_noise_signal_stability
 * @brief Verify random noise does not cause numerical instability
 */
void test_noise_signal_stability() {
  std::cout << "\n=== Test: Noise Signal Stability ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::NOISE, 32, 0.1f);
    (void)mpx.compute(signal.data(), 32U);

    float *matrix = mpx.get_matrix();
    uint16_t profile_len = mpx.get_profile_len();

    bool all_finite = true;
    for (uint16_t i = 0; i < profile_len; i++) {
      if (!std::isfinite(matrix[i])) {
        all_finite = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_finite);
    print_test_result("Noise signal produces finite matrix profile", all_finite);
  } catch (const std::exception &e) {
    print_test_result("Noise signal stability test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

// ============================================================================
// TEST SUITE: Sequential Processing
// ============================================================================

/**
 * @test test_sequential_compute_does_not_crash
 * @brief Verify multiple sequential compute() calls work correctly
 */
void test_sequential_compute_does_not_crash() {
  std::cout << "\n=== Test: Sequential Compute Does Not Crash ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::SINE_WAVE, 48, 1.0f, 0.1f);

    (void)mpx.compute(signal.data(), 16U);
    (void)mpx.compute(signal.data() + 16, 16U);
    (void)mpx.compute(signal.data() + 32, 16U);

    float *matrix = mpx.get_matrix();
    uint16_t profile_len = mpx.get_profile_len();

    bool all_valid = true;
    for (uint16_t i = 0; i < profile_len; i++) {
      if (!(std::isfinite(matrix[i]) || matrix[i] <= -999000.0f)) {
        all_valid = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_valid);
    print_test_result("Sequential compute (3 batches) produces valid results", all_valid);
  } catch (const std::exception &e) {
    print_test_result("Sequential compute test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

/**
 * @test test_floss_after_sequential_compute
 * @brief Verify FLOSS works after sequential processing
 */
void test_floss_after_sequential_compute() {
  std::cout << "\n=== Test: FLOSS After Sequential Compute ===" << std::endl;

  try {
    const uint16_t window_size = 8U;
    const uint16_t buffer_size = 64U;
    Mpx mpx(window_size, 0.5F, 0U, buffer_size);

    auto signal = TestSignalGenerator::generate(TestSignalGenerator::SINE_WAVE, 32, 1.0f, 0.1f);

    (void)mpx.compute(signal.data(), 16U);
    (void)mpx.compute(signal.data() + 16, 16U);

    mpx.floss();

    float *floss = mpx.get_floss();
    uint16_t profile_len = mpx.get_profile_len();

    bool all_finite = true;
    for (uint16_t i = 0; i < profile_len; i++) {
      if (!std::isfinite(floss[i])) {
        all_finite = false;
        break;
      }
    }

    TEST_ASSERT_TRUE(all_finite);
    print_test_result("FLOSS after sequential compute produces finite values", all_finite);
  } catch (const std::exception &e) {
    print_test_result("FLOSS after sequential compute test", false);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
  std::cout << "\n╔════════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║   Robustness Tests - Mpx Library                   ║" << std::endl;
  std::cout << "║   Matrix Profile for Time Series                   ║" << std::endl;
  std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;

  try {
    // Numerical Stability Tests
    test_movmean_returns_finite_values();
    test_movsig_returns_valid_values();
    test_differential_arrays_are_finite();

    // Matrix Profile Invariants
    test_mp_profile_values_bounded();
    test_mp_indexes_valid_or_empty();

    // FLOSS Output Sanity
    test_floss_returns_finite_values();
    test_floss_values_in_reasonable_range();

    // Edge Cases & Robustness
    test_minimal_buffer_size();
    test_large_exclusion_zone();
    test_time_constraint_accepted();

    // Data Pattern Robustness
    test_constant_signal_no_crash();
    test_mixed_pattern_signal();
    test_noise_signal_stability();

    // Sequential Processing
    test_sequential_compute_does_not_crash();
    test_floss_after_sequential_compute();

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
