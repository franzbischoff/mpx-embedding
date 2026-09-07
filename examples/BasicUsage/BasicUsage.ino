#include <Mpx.hpp>

using MatrixProfile::Mpx;

Mpx mpx(16, 0.5F, 0U, 64U);
float samples[64];

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  for (uint16_t index = 0; index < 64U; ++index) {
    samples[index] = (index % 16U < 8U) ? 1.0F : -1.0F;
  }

  const uint16_t remaining = mpx.compute(samples, 64U);
  Serial.print("Configuration valid: ");
  Serial.println(mpx.is_valid() ? "yes" : "no");
  Serial.print("Remaining buffer capacity: ");
  Serial.println(remaining);
  Serial.print("Profile length: ");
  Serial.println(mpx.get_profile_len());
}

void loop() {
}
