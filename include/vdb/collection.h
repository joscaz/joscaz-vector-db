/**
 * collection.h - Collection management API
 *
 * A collection is a named container for vectors with a fixed dimension
 * and distance metric. All vectors in a collection must have the same
 * dimension.
 *
 * Design:
 * - Collections are identified by name (max 64 chars)
 * - Dimension and metric are immutable after creation
 * - Collections will later persist to disk
*/

#ifndef VDB_COLLECTION_H
#define VDB_COLLECTION_H

#include "types.h"

// Max collection name length (including null terminator)
#define VDB_COLLECTION_NAME_MAX_LEN 64

// Max reasonable dimension (safety check)
// 64K dimensions should be plenty for any practical use case
#define VDB_COLLECTION_MAX_DIM 65536

// Opaque collection handle
// Actual structure is defined in collection.c to hide implementation details
typedef struct vdb_collection vdb_collection_t;

// collection metadata (return by info functions)
typedef struct {
    char name[VDB_COLLECTION_NAME_MAX_LEN];
    uint32_t dim;
    vdb_metric_t metric;
    uint64_t num_vectors; // num of vectors stored
} vdb_collection_info_t;

/**
 * Create a new in-memory collection
 * 
 * Params:
 * - name: Collection name (must be valid, non-empty, < 64 chars)
 * - dim: Vector dimension (must be > 0 and <= VDB_COLLECTION_MAX_DIM)
 * - metric: Distance metric (must be valid)
 * - out_collection: Pointer to receive the created collection handle
 * 
 * Returns:
 * - VDB_OK: success, *out_collection is valid
 * - VDB_ERROR_INVALID_ARGUMENT: Invalid name, dimension, or metric
 * - VDB_ERROR_OUT_OF_MEMORY: Allocation failed
 * 
 * The caller must call vdb_collection_close() to free resources
*/
vdb_status_t vdb_collection_create(
    const char *name,
    uint32_t dim,
    vdb_metric_t metric,
    vdb_collection_t **out_collection
);

/**
 * Close a collection and free resources
 * Safe to call with NULL pointer.
 * Sets *collection to NULL after freeing.
*/
void vdb_collection_close(vdb_collection_t **collection);

/**
 * Get collection information
 *
 * Parameters:
 * - collection: Collection handle (must not be NULL)
 * - out_info: Pointer to receive collection info
 *
 * Returns:
 * - VDB_OK: Success, *out_info is populated
 * - VDB_ERROR_INVALID_ARGUMENT: collection or out_info is NULL
*/
vdb_status_t vdb_collection_get_info(
    const vdb_collection_t *collection,
    vdb_collection_info_t *out_info
);

/**
 * Validate collection parameters (helper function)
 * Checks name, dimension, and metric validity.
 *
* Returns: VDB_OK if valid, error code otherwise
 */
vdb_status_t vdb_collection_validate_params(
    const char *name,
    uint32_t dim,
    vdb_metric_t metric
);

#endif /* VDB_COLLECTION_H */