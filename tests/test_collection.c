/**
 * test_collection.c - Tests for collection management
 */

 #include "test_framework.h"
 #include "vdb/collection.h"
 #include <string.h>
 
 /**
  * Test parameter validation
  */
 TEST(collection_validate_params) {
     // Valid parameters
     vdb_status_t status = vdb_collection_validate_params("test", 128, VDB_METRIC_COSINE);
     ASSERT_EQ(VDB_OK, status);
     
     status = vdb_collection_validate_params("my-collection", 512, VDB_METRIC_EUCLIDEAN);
     ASSERT_EQ(VDB_OK, status);
     
     // Invalid name
     status = vdb_collection_validate_params(NULL, 128, VDB_METRIC_COSINE);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
     
     status = vdb_collection_validate_params("", 128, VDB_METRIC_COSINE);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
     
     // Invalid dimension
     status = vdb_collection_validate_params("test", 0, VDB_METRIC_COSINE);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
     
     status = vdb_collection_validate_params("test", VDB_COLLECTION_MAX_DIM + 1, VDB_METRIC_COSINE);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
     
     // Invalid metric
     status = vdb_collection_validate_params("test", 128, (vdb_metric_t)999);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
 }
 
 /**
  * Test collection creation and closing
  */
 TEST(collection_create_close) {
     vdb_collection_t *coll = NULL;
     
     // Create a valid collection
     vdb_status_t status = vdb_collection_create("test-collection", 128, VDB_METRIC_COSINE, &coll);
     ASSERT_EQ(VDB_OK, status);
     ASSERT_NOT_NULL(coll);
     
     // Get info
     vdb_collection_info_t info;
     status = vdb_collection_get_info(coll, &info);
     ASSERT_EQ(VDB_OK, status);
     ASSERT_STR_EQ("test-collection", info.name);
     ASSERT_EQ(128, info.dim);
     ASSERT_EQ(VDB_METRIC_COSINE, info.metric);
     ASSERT_EQ(0, info.num_vectors);
     
     // Close it
     vdb_collection_close(&coll);
     ASSERT_NULL(coll);
     
     // Safe to close again
     vdb_collection_close(&coll);
 }
 
 /**
  * Test collection with different metrics
  */
 TEST(collection_metrics) {
     vdb_collection_t *coll_cosine = NULL;
     vdb_collection_t *coll_euclidean = NULL;
     
     // Create cosine collection
     vdb_status_t status = vdb_collection_create("cosine-coll", 256, VDB_METRIC_COSINE, &coll_cosine);
     ASSERT_EQ(VDB_OK, status);
     
     // Create euclidean collection
     status = vdb_collection_create("euclidean-coll", 512, VDB_METRIC_EUCLIDEAN, &coll_euclidean);
     ASSERT_EQ(VDB_OK, status);
     
     // Verify metrics
     vdb_collection_info_t info;
     
     status = vdb_collection_get_info(coll_cosine, &info);
     ASSERT_EQ(VDB_OK, status);
     ASSERT_EQ(VDB_METRIC_COSINE, info.metric);
     ASSERT_EQ(256, info.dim);
     
     status = vdb_collection_get_info(coll_euclidean, &info);
     ASSERT_EQ(VDB_OK, status);
     ASSERT_EQ(VDB_METRIC_EUCLIDEAN, info.metric);
     ASSERT_EQ(512, info.dim);
     
     // Clean up
     vdb_collection_close(&coll_cosine);
     vdb_collection_close(&coll_euclidean);
 }
 
 /**
  * Test collection creation with invalid parameters
  */
 TEST(collection_create_invalid) {
     vdb_collection_t *coll = NULL;
     
     // NULL output pointer
     vdb_status_t status = vdb_collection_create("test", 128, VDB_METRIC_COSINE, NULL);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
     
     // Invalid name
     status = vdb_collection_create(NULL, 128, VDB_METRIC_COSINE, &coll);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
     ASSERT_NULL(coll);
     
     status = vdb_collection_create("", 128, VDB_METRIC_COSINE, &coll);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
     ASSERT_NULL(coll);
     
     // Invalid dimension
     status = vdb_collection_create("test", 0, VDB_METRIC_COSINE, &coll);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
     ASSERT_NULL(coll);
     
     // Invalid metric
     status = vdb_collection_create("test", 128, (vdb_metric_t)999, &coll);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
     ASSERT_NULL(coll);
 }
 
 /**
  * Test get_info with invalid parameters
  */
 TEST(collection_get_info_invalid) {
     vdb_collection_t *coll = NULL;
     vdb_collection_info_t info;
     
     // Create a valid collection first
     vdb_status_t status = vdb_collection_create("test", 128, VDB_METRIC_COSINE, &coll);
     ASSERT_EQ(VDB_OK, status);
     
     // NULL collection
     status = vdb_collection_get_info(NULL, &info);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
     
     // NULL output
     status = vdb_collection_get_info(coll, NULL);
     ASSERT_EQ(VDB_ERROR_INVALID_ARGUMENT, status);
     
     // Clean up
     vdb_collection_close(&coll);
 }
 
 /**
  * Test long collection name (boundary)
  */
 TEST(collection_long_name) {
     vdb_collection_t *coll = NULL;
     
     // Create a name that's exactly at the limit (63 chars + null)
     char long_name[VDB_COLLECTION_NAME_MAX_LEN];
     memset(long_name, 'a', VDB_COLLECTION_NAME_MAX_LEN - 1);
     long_name[VDB_COLLECTION_NAME_MAX_LEN - 1] = '\0';
     
     vdb_status_t status = vdb_collection_create(long_name, 128, VDB_METRIC_COSINE, &coll);
     ASSERT_EQ(VDB_OK, status);
     
     vdb_collection_info_t info;
     status = vdb_collection_get_info(coll, &info);
     ASSERT_EQ(VDB_OK, status);
     
     // Name should be truncated and null-terminated
     ASSERT_EQ('\0', info.name[VDB_COLLECTION_NAME_MAX_LEN - 1]);
     
     vdb_collection_close(&coll);
 }