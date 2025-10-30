/**
 * test_types.c - Tests for core types
*/

#include "test_framework.h"
#include "vdb/types.h"
#include "vdb/collection.h"
#include <string.h>

/**
 * Test metric to string conversion
*/
TEST(metric_to_string) {
    ASSERT_STR_EQ("cosine", vdb_metric_to_string(VDB_METRIC_COSINE));
    ASSERT_STR_EQ("euclidean", vdb_metric_to_string(VDB_METRIC_EUCLIDEAN));
    ASSERT_STR_EQ("unknown", vdb_metric_to_string((vdb_metric_t)999));
}

/**
 * Test status to string conversion
*/
TEST(status_to_string) {
    ASSERT_STR_EQ("OK", vdb_status_to_string(VDB_OK));
    ASSERT_STR_EQ("Invalid argument", vdb_status_to_string(VDB_ERROR_INVALID_ARGUMENT));
    ASSERT_STR_EQ("Out of memory", vdb_status_to_string(VDB_ERROR_OUT_OF_MEMORY));
}

/**
 * Test metric validation
*/
TEST(metric_validation) {
    ASSERT_TRUE(vdb_metric_is_valid(VDB_METRIC_COSINE));
    ASSERT_TRUE(vdb_metric_is_valid(VDB_METRIC_EUCLIDEAN));
    ASSERT_FALSE(vdb_metric_is_valid((vdb_metric_t)999));
    ASSERT_FALSE(vdb_metric_is_valid((vdb_metric_t)-1));
}

/**
 * Test vector creation and freeing
 */
TEST(vector_create_free) {
    vdb_vector_t vec;

    //valid creation
    vdb_status_t status = vdb_vector_create(128, &vec);
    ASSERT_EQ(VDB_OK, status);
    ASSERT_EQ(128, vec.dim);
    ASSERT_NOT_NULL(vec.data);

    // data should be zero-initialized
    for (uint32_t i = 0; i < vec.dim; i++) {
        ASSERT_FLOAT_EQ(0.0f, vec.data[i], 1e-9f);
    }

    // free it
    vdb_vector_free(&vec);
    ASSERT_EQ(0, vec.dim);
    ASSERT_NULL(vec.data);

    // safe to free twice
    vdb_vector_free(&vec);
}

/**
 * Test vector creation with invalid parameters
 */
TEST(vector_create_invalid) {
    vdb_vector_t vec;
    
    // Zero dimension
    vdb_status_t status = vdb_vector_create(0, &vec);
    ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
    ASSERT_NULL(vec.data);
    
    // Too large dimension
    status = vdb_vector_create(VDB_COLLECTION_MAX_DIM + 1, &vec);
    ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
    ASSERT_NULL(vec.data);
    
    // NULL output pointer
    status = vdb_vector_create(128, NULL);
    ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
}

/**
 * Test vector copy
 */
TEST(vector_copy) {
    vdb_vector_t src, dst;
    
    // Create source vector
    vdb_status_t status = vdb_vector_create(3, &src);
    ASSERT_EQ(VDB_OK, status);
    
    // Fill with test data
    src.data[0] = 1.0f;
    src.data[1] = 2.0f;
    src.data[2] = 3.0f;
    
    // Copy it
    status = vdb_vector_copy(&src, &dst);
    ASSERT_EQ(VDB_OK, status);
    ASSERT_EQ(src.dim, dst.dim);
    ASSERT_NOT_NULL(dst.data);
    
    // Verify data was copied
    ASSERT_FLOAT_EQ(1.0f, dst.data[0], 1e-9f);
    ASSERT_FLOAT_EQ(2.0f, dst.data[1], 1e-9f);
    ASSERT_FLOAT_EQ(3.0f, dst.data[2], 1e-9f);
    
    // Verify it's a deep copy (different pointers)
    ASSERT_TRUE(src.data != dst.data);
    
    // Clean up
    vdb_vector_free(&src);
    vdb_vector_free(&dst);
}

/**
 * Test ID validation
 */
TEST(id_validation) {
    // Valid IDs
    ASSERT_TRUE(vdb_id_is_valid("test"));
    ASSERT_TRUE(vdb_id_is_valid("my-vector-123"));
    ASSERT_TRUE(vdb_id_is_valid("UUID-1234-5678"));
    
    // Invalid IDs
    ASSERT_FALSE(vdb_id_is_valid(NULL));
    ASSERT_FALSE(vdb_id_is_valid(""));
    
    // ID that's too long (>= 64 chars, no null terminator)
    char long_id[VDB_ID_MAX_LEN + 10];
    memset(long_id, 'a', sizeof(long_id));
    long_id[sizeof(long_id) - 1] = '\0';
    ASSERT_FALSE(vdb_id_is_valid(long_id));
    
    // ID with non-printable character
    char bad_id[10] = "test";
    bad_id[2] = '\n';  // Non-printable
    ASSERT_FALSE(vdb_id_is_valid(bad_id));
}

/**
 * Test ID copy
 */
TEST(id_copy) {
    vdb_id_t dst;
    
    // Valid copy
    vdb_status_t status = vdb_id_copy("test-id-123", dst);
    ASSERT_EQ(VDB_OK, status);
    ASSERT_STR_EQ("test-id-123", dst);
    
    // Invalid source
    status = vdb_id_copy(NULL, dst);
    ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
    
    status = vdb_id_copy("", dst);
    ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
    
    // Long ID that needs truncation
    char long_src[VDB_ID_MAX_LEN + 10];
    memset(long_src, 'x', VDB_ID_MAX_LEN - 1);
    long_src[VDB_ID_MAX_LEN - 1] = '\0';
    
    status = vdb_id_copy(long_src, dst);
    ASSERT_EQ(VDB_OK, status);
    // Should be truncated and null-terminated
    ASSERT_EQ('\0', dst[VDB_ID_MAX_LEN - 1]);
}