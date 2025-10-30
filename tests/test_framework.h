/**
 * test_framework.h - Simple custom test framework for VDB
 *
 * This is a minimal testing framework with no external dependencies.
 * It provides basic assertion macros and a test runner mechanism.
 *
 * Usage:
 *   1. Define test functions using TEST(test_name) { ... }
 *   2. Use ASSERT_* macros inside tests
 *   3. Call RUN_TEST(test_name) in main()
 */

#ifndef VDB_TEST_FRAMEWORK_H
#define VDB_TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Global test statistics */
static int _tests_run = 0;
static int _tests_passed = 0;
static int _tests_failed = 0;
static const char *_current_test = NULL;

/**
 * Define a test function
 * Usage: TEST(my_test_name) { ASSERT_TRUE(1 == 1); }
 */
#define TEST(name) \
    void test_##name(void)

/**
 * Run a test and track results
 * Usage: RUN_TEST(my_test_name);
 */
#define RUN_TEST(name) \
    do { \
        _current_test = #name; \
        _tests_run++; \
        printf("Running test: %s ... ", #name); \
        fflush(stdout); \
        test_##name(); \
        _tests_passed++; \
        printf("PASSED\n"); \
    } while (0)

/**
 * Assert that a condition is true
 */
#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            printf("\n  FAILED: %s:%d: Assertion failed: %s\n", \
                   __FILE__, __LINE__, #condition); \
            _tests_failed++; \
            _tests_passed--; \
            return; \
        } \
    } while (0)

/**
 * Assert that a condition is false
 */
#define ASSERT_FALSE(condition) \
    ASSERT_TRUE(!(condition))

/**
 * Assert that two integers are equal
 */
#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("\n  FAILED: %s:%d: Expected %d, got %d\n", \
                   __FILE__, __LINE__, (int)(expected), (int)(actual)); \
            _tests_failed++; \
            _tests_passed--; \
            return; \
        } \
    } while (0)

/**
 * Assert that two integers are not equal
 */
#define ASSERT_NE(not_expected, actual) \
    do { \
        if ((not_expected) == (actual)) { \
            printf("\n  FAILED: %s:%d: Expected not %d, but got %d\n", \
                   __FILE__, __LINE__, (int)(not_expected), (int)(actual)); \
            _tests_failed++; \
            _tests_passed--; \
            return; \
        } \
    } while (0)

/**
 * Assert that two strings are equal
 */
#define ASSERT_STR_EQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            printf("\n  FAILED: %s:%d: Expected \"%s\", got \"%s\"\n", \
                   __FILE__, __LINE__, (expected), (actual)); \
            _tests_failed++; \
            _tests_passed--; \
            return; \
        } \
    } while (0)

/**
 * Assert that a pointer is NULL
 */
#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            printf("\n  FAILED: %s:%d: Expected NULL pointer\n", \
                   __FILE__, __LINE__); \
            _tests_failed++; \
            _tests_passed--; \
            return; \
        } \
    } while (0)

/**
 * Assert that a pointer is not NULL
 */
#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            printf("\n  FAILED: %s:%d: Expected non-NULL pointer\n", \
                   __FILE__, __LINE__); \
            _tests_failed++; \
            _tests_passed--; \
            return; \
        } \
    } while (0)

/**
 * Assert that two floats are approximately equal (within epsilon)
 */
#define ASSERT_FLOAT_EQ(expected, actual, epsilon) \
    do { \
        double _diff = fabs((double)(expected) - (double)(actual)); \
        if (_diff > (epsilon)) { \
            printf("\n  FAILED: %s:%d: Expected %f, got %f (diff: %f > %f)\n", \
                   __FILE__, __LINE__, (double)(expected), (double)(actual), \
                   _diff, (double)(epsilon)); \
            _tests_failed++; \
            _tests_passed--; \
            return; \
        } \
    } while (0)

/**
 * Print test summary
 * Call this at the end of main() after all RUN_TEST() calls
 */
#define TEST_SUMMARY() \
    do { \
        printf("\n========================================\n"); \
        printf("Test Summary:\n"); \
        printf("  Total:  %d\n", _tests_run); \
        printf("  Passed: %d\n", _tests_passed); \
        printf("  Failed: %d\n", _tests_failed); \
        printf("========================================\n"); \
        if (_tests_failed > 0) { \
            printf("RESULT: FAILED\n"); \
            exit(1); \
        } else { \
            printf("RESULT: ALL PASSED\n"); \
            exit(0); \
        } \
    } while (0)

#endif /* VDB_TEST_FRAMEWORK_H */

