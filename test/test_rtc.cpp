#include "test_runner.h"
#include "../../firmware/include/drivers/RTC_DS1302.h"
// Test that DateTime validation works correctly
void test_time_validation() {
  // Valid date/time
  RTC_DS1302::DateTime validDt = {25, 6, 15, 1, 12, 34, 56};
  TEST_ASSERT_TRUE(validDt.isValid());
  
  // Invalid month
  RTC_DS1302::DateTime invalidMonth = {25, 13, 15, 1, 12, 34, 56};
  TEST_ASSERT_FALSE(invalidMonth.isValid());
}

// Test that date/time can be parsed from string
void test_time_parsing() {
  // TODO: Add test once parsing function is implemented
  TEST_ASSERT_TRUE(true);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_time_validation);
  RUN_TEST(test_time_parsing);
  UNITY_END();
}

void loop() {
  // Nothing to do
}
