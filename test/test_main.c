#include <unity.h>
#include "alarm.h"
#include "input.h"
#include "system_state.h"

void setUp(void) {}
void tearDown(void) {}

// --- Alarm Logic Tests (5 Tests) ---
void test_temp_below_lower_threshold(void) {
    TEST_ASSERT_EQUAL(ALARM_LOW_TEMP, evaluateTemperature(17.5f));
}

void test_temp_exactly_lower_threshold(void) {
    TEST_ASSERT_EQUAL(ALARM_NORMAL, evaluateTemperature(18.0f));
}

void test_temp_normal_value(void) {
    TEST_ASSERT_EQUAL(ALARM_NORMAL, evaluateTemperature(24.0f));
}

void test_temp_exactly_upper_threshold(void) {
    TEST_ASSERT_EQUAL(ALARM_NORMAL, evaluateTemperature(30.0f));
}

void test_temp_above_upper_threshold(void) {
    TEST_ASSERT_EQUAL(ALARM_HIGH_TEMP, evaluateTemperature(31.2f));
}

// --- Navigation Logic Tests (4 Tests) ---
void test_nav_forward_transition(void) {
    TEST_ASSERT_EQUAL(MODE_HUMIDITY, nextDisplayMode(MODE_TEMP));
}

void test_nav_forward_wraparound(void) {
    TEST_ASSERT_EQUAL(MODE_TEMP, nextDisplayMode(MODE_MOTION));
}

void test_nav_reverse_transition(void) {
    TEST_ASSERT_EQUAL(MODE_TEMP, previousDisplayMode(MODE_HUMIDITY));
}

void test_nav_reverse_wraparound(void) {
    TEST_ASSERT_EQUAL(MODE_MOTION, previousDisplayMode(MODE_TEMP));
}

// --- System State Tests (4 Tests) ---
void test_state_active_no_timeout(void) {
    TEST_ASSERT_EQUAL(STATE_ACTIVE, evaluateSystemState(STATE_ACTIVE, false, 5));
}

void test_state_active_timeout(void) {
    TEST_ASSERT_EQUAL(STATE_INACTIVE, evaluateSystemState(STATE_ACTIVE, false, 15));
}

void test_state_inactive_no_motion(void) {
    TEST_ASSERT_EQUAL(STATE_INACTIVE, evaluateSystemState(STATE_INACTIVE, false, 20));
}

void test_state_inactive_motion(void) {
    TEST_ASSERT_EQUAL(STATE_ACTIVE, evaluateSystemState(STATE_INACTIVE, true, 20));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Alarm tests
    RUN_TEST(test_temp_below_lower_threshold);
    RUN_TEST(test_temp_exactly_lower_threshold);
    RUN_TEST(test_temp_normal_value);
    RUN_TEST(test_temp_exactly_upper_threshold);
    RUN_TEST(test_temp_above_upper_threshold);

    // Navigation tests
    RUN_TEST(test_nav_forward_transition);
    RUN_TEST(test_nav_forward_wraparound);
    RUN_TEST(test_nav_reverse_transition);
    RUN_TEST(test_nav_reverse_wraparound);

    // State tests
    RUN_TEST(test_state_active_no_timeout);
    RUN_TEST(test_state_active_timeout);
    RUN_TEST(test_state_inactive_no_motion);
    RUN_TEST(test_state_inactive_motion);

    return UNITY_END();
}