/**
 * test_main.c - Test runner for VDB
 *
 * This file contains the main test runner and basic sanity tests
 * to verify the test framework itself is working correctly.
 */

#include "test_framework.h"

/* Declare test functions from other files */
/* From test_types.c */
extern void test_metric_to_string(void);
extern void test_status_to_string(void);
extern void test_metric_validation(void);
extern void test_vector_create_free(void);
extern void test_vector_create_invalid(void);
extern void test_vector_copy(void);
extern void test_id_validation(void);
extern void test_id_copy(void);

/* From test_collection.c */
extern void test_collection_validate_params(void);
extern void test_collection_create_close(void);
extern void test_collection_metrics(void);
extern void test_collection_create_invalid(void);
extern void test_collection_get_info_invalid(void);
extern void test_collection_long_name(void);

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
    printf("VDB Test Suite - Step 2\n");
    printf("===========================================\n\n");
    
    /* Sanity tests */
    printf("--- Sanity Tests ---\n");
    RUN_TEST(sanity_arithmetic);
    
    /* Type tests */
    printf("\n--- Type Tests ---\n");
    RUN_TEST(metric_to_string);
    RUN_TEST(status_to_string);
    RUN_TEST(metric_validation);
    RUN_TEST(vector_create_free);
    RUN_TEST(vector_create_invalid);
    RUN_TEST(vector_copy);
    RUN_TEST(id_validation);
    RUN_TEST(id_copy);
    
    /* Collection tests */
    printf("\n--- Collection Tests ---\n");
    RUN_TEST(collection_validate_params);
    RUN_TEST(collection_create_close);
    RUN_TEST(collection_metrics);
    RUN_TEST(collection_create_invalid);
    RUN_TEST(collection_get_info_invalid);
    RUN_TEST(collection_long_name);
    
    /* Print summary and exit */
    TEST_SUMMARY();
    
    return 0;
}