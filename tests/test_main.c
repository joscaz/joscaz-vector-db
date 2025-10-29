/**
 * test_main.c - Test runner for VDB
 *
 * This file contains the main test runner and basic sanity tests
 * to verify the test framework itself is working correctly.
 */

#include "test_framework.h"

/**
 * Sanity test: basic arithmetic
 */
TEST(sanity_arithmetic) {
    ASSERT_EQ(2 + 2, 4);
    ASSERT_EQ(10 - 5, 5);
    ASSERT_TRUE(1 == 1);
    ASSERT_FALSE(1 == 2);
}

/**
 * Sanity test: string comparison
 */
TEST(sanity_strings) {
    const char *hello = "hello";
    const char *world = "world";
    
    ASSERT_STR_EQ("hello", hello);
    ASSERT_STR_EQ("world", world);
    ASSERT_TRUE(strcmp(hello, world) != 0);
}

/**
 * Sanity test: pointer checks
 */
TEST(sanity_pointers) {
    int value = 42;
    int *ptr = &value;
    int *null_ptr = NULL;
    
    ASSERT_NOT_NULL(ptr);
    ASSERT_NULL(null_ptr);
    ASSERT_EQ(*ptr, 42);
}

/**
 * Sanity test: floating point comparison
 */
TEST(sanity_floats) {
    float a = 1.0f;
    float b = 1.0f + 1e-7f;
    float c = 2.0f;
    
    ASSERT_FLOAT_EQ(a, b, 1e-6f);
    ASSERT_TRUE(c > a);
}

/**
 * Main test runner
 */
int main(void) {
    printf("===========================================\n");
    printf("VDB Test Suite\n");
    printf("===========================================\n\n");
    
    // Run sanity tests
    printf("Running sanity tests...\n");
    RUN_TEST(sanity_arithmetic);
    RUN_TEST(sanity_strings);
    RUN_TEST(sanity_pointers);
    RUN_TEST(sanity_floats);
    
    // Print summary and exit
    TEST_SUMMARY();
    
    return 0;
}

