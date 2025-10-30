/**
 * types.h - Core type definitions for VDB
 * 
 * This header defines the fundamental types used throughout the VDB library:
 * - Metric types (cosine, euclidean)
 * - Status codes for err handling
 * - Vector representation (dimension + float array)
 * - ID type (fixed-size string)
 * 
 * Design choices:
 * - Fixed-size IDs of 64 bytes: To simplify storage layout, no per-ID malloc
 * - Explicit status codes: Clear error reporting withour errno
 * - Vectors own their data: Easier memory management, explicit ownership
*/

#ifndef VDB_TYPES_H
#define VDB_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Distance metric types
typedef enum {
    VDB_METRIC_COSINE = 0, /* Cosine similarity (range: [-1,1], higher = more similar) */
    VDB_METRIC_EUCLIDEAN = 1, /* Euclidean (L2) distance (range: [0, inf), lower = more similar)  */
} vdb_metric_t;

/**
 * Status codes for err handling
 * Convention: 0 = success, negative, error
 */

typedef enum {
    VDB_OK = 0, // Success
    VDB_ERROR_INVALID_ARGUMENT = -1, // Invalid func arg
    VDB_ERROR_OUT_OF_MEMORY = -2, // mem alloc failed
    VDB_ERROR_IO = -3, // file I/O error
    VDB_ERROR_NOT_FOUND = -4, // resource not found
    VDB_ERROR_ALREADY_EXISTS = -5, // resource already exists
    VDB_ERROR_CORRUPTED = -6, // data corruption detected
    VDB_ERROR_DIMENSION_MISMATCH = -7, // vector dimension mismatch
    VDB_ERROR_UNKNOWN = -99 // unknown error
} vdb_status_t;

/**
 * Max ID length (including null terminator)
 * 64 bytes allows for UUIDs, short desc names, etc.
*/
#define VDB_ID_MAX_LEN 64

/** 
 * Fixed-size ID type
 * Using a fixed size simplifies storage (no indirection) and makes serialization trivial
*/
typedef char vdb_id_t[VDB_ID_MAX_LEN];

/**
 * Vector representation
 * Contains dimension and a dynamically allocated float array
 * Caller is responsible for freeing via vdb_vector_free().
*/
typedef struct {
    uint32_t dim; // Num of dimensions
    float *data; // float arr of length dim
} vdb_vector_t;

/**
 * Get human-readable string for a metric type
 * Returns: String name (e.g., "cosine", "euclidean") or "unknown"
*/
const char* vdb_metric_to_string(vdb_metric_t metric);

/**
 * Get human-readable string for a status code
 * Returns: String description of the status
*/
const char* vdb_status_to_string(vdb_status_t status);

/**
 * Validate a metric type
 * Returns: true if metric is valid, false otherwise
*/
bool vdb_metric_is_valid(vdb_metric_t metric);

/**
 * Create a vector with the given dimension
 * Allocates memory for the float array (initialized to zero).
 *
 * Returns: VDB_OK on success, error code otherwise
 * On success, *out_vector is populated with allocated vector
 * On error, *out_vector is set to {0, NULL}
*/
vdb_status_t vdb_vector_create(uint32_t dim, vdb_vector_t *out_vector);

/**
 * Free a vector's data array
 * After calling this, the vector should not be used.
 * Safe to call with {0, NULL} or NULL pointer.
*/
void vdb_vector_free(vdb_vector_t *vector);

/**
 * Copy a vector (deep copy)
 * Allocates a new data array and copies values.
 *
 * Returns: VDB_OK on success, error code otherwise
*/
vdb_status_t vdb_vector_copy(const vdb_vector_t *src, vdb_vector_t *dst);

/** 
 * Validate an ID string
 * Checks that:
 * - id not empty
 * - id not NULL
 * - id is null-terminated withing VDB_ID_MAX_LEN
 * - id contains only printable chars
 * 
 * Returns: true if valid, false otherwise
*/
bool vdb_id_is_valid(const char *id);

/**
 * Safely copy an ID string
 * Ensures null-termination and length bounds.
 *
 * Returns: VDB_OK on success, VDB_ERROR_INVALID_ARGUMENT if src is invalid or too long
*/
vdb_status_t vdb_id_copy(const char *src, vdb_id_t dst);

#endif /* VDB_TYPES_H */