/**
 * storage.h - Persistent storage API for VDB collection
 * 
 * This module handles:
 * - Writing vectors to disk (append-only segment)
 * - Write-ahead logging (WAL) for crash recovery
 * - Memory-mapped reads for performance
 * - Atomic operations via fsync
 * 
 * File layout per collection:
 *    data/<name>/collection.meta   - Metadata (dim, metric, count)
 *    data/<name>/embeddings.seq    - Float32 embeddings (dim * 4 bytes per vector)
 *    data/<name>/ids.seq           - Fixed 64-byte IDs
 *    data/<name>/metadata.seq      - Length-prefixed JSON strings
 *    data/<name>/wal.log           - Write-ahead log
 * 
 * Design:
 * - Append-only: Never modify existing data (simplifies concurrency)
 * - WAL-first: Write to WAL + fsync before touching segments
 * -mmap reads: OS manages caching, fast sequential/random access
*/

#ifndef VDB_STORAGE_H
#define VDB_STORAGE_H

#include "types.h"
#include "collection.h"
#include <stdint.h>

/** 
 * Storage handle (opaque)
 * Represents an open collection on disk
*/
typedef struct vdb_storage vdb_storage_t;

/**
 * Item to store (ID + vector + metadata)
*/
typedef struct {
    vdb_id_t id; // unique id
    vdb_vector_t vector; // embedding vector
    const char *metadata; // JSON metadata string (nullable)
} vdb_item_t;

/**
 * Iterator callback for scanning stored items
 * Called once per item in storage order
 * 
 * Parameters:
 * - item: The stored item
 * - user_data: User-provided context pointer
 * 
 * Return: 0 to continue, non-zero to stop iteration
*/
typedef int (*vdb_storage_iter_fn)(const vdb_item_t *item, void *user_data);

/**
 * Create a new collection on disk
 * 
 * Creates directory structure and writes initial metadata
 * The collection is empty initially
 * 
 * Parameters:
 * - base_dir: Base directory for data (e.g. "./data")
 * - name: Collection name
 * - dim: Vector dimension
 * - metric: Distance metric
 * - out_store: Receives storage handle on success
 * 
 * Returns:
 * - VDB_OK: Success
 * - VDB_ERROR_ALREADY_EXISTS: Collection already exists
 * - VDB_ERROR_INVALID_ARGUMENT: Invalid parameters
 * - VDB_ERROR_IO: Failed to create files
*/
vdb_status_t vdb_storage_create(
    const char *base_dir,
    const char *name,
    uint32_t dim,
    vdb_metric_t metric,
    vdb_storage_t **out_storage
);

/**
 * Open an existing collection from disk
 * 
 * Loads metadata and recovers from WAL if needed
 * 
 * Parameters:
 * - base_dir: Base directory for data
 * - name: Collection name
 * - out_storage: Receives storage handle on success
 * 
 * Returns:
 * - VDB_OK: Success
 * - VDB_ERROR_NOT_FOUND: Collection doesn't exist
 * - VDB_ERROR_IO: I/O error
 * - VDB_ERROR_CORRUPTED: Metadata corrupted
*/
vdb_status_t vdb_storage_open(
    const char *base_dir,
    const char *name,
    vdb_storage_t **out_storage
);

/**
 * Close storage and release resources
 * Flushes any pending writes, unmaps memory, closes files
 * Safe to call with NULL. Sets *storage to NULL
*/
void vdb_storage_close(vdb_storage_t **storage);

/**
 * Append an item to storage
 * 1. Validate item (dimension matches collection)
 * 2. Write to WAL + fsync
 * 3. Append to segment files
 * 4. Update metadata (increment count)
 * 5. Truncate WAL
 * 
 * Parameters:
 * - storage: Storage handle
 * - item: Item to append (ID must be unique - not checking for now)
 * 
 * Returns:
 * - VDB_OK: Success
 * - VDB_ERROR_INVALID_ARGUMENT: Null params or dimension mismatch
 * - VDB_ERROR_IO: Write failed
*/
vdb_status_t vdb_storage_append(
    vdb_storage_t *storage,
    const vdb_item_t *item
);

/**
 * Iterate over all stored items
 * 
 * Reads from segment files (not WAL)
 * Items are return in insertion order
 * 
 * Parameters:
 * - storage: Storage handle
 * - callback: function called for each item
 * - user_data: Passed to callback
 * 
 * Returns: 
 * - VDB_OK: Success (all items processed or callb stopped iter)
 * - VDB_ERROR_INVALID_ARGUMENT: Null params
 * - VDB_ERROR_IO: Read error
*/
vdb_status_t vdb_storage_iterate(
    vdb_storage_t *storage,
    vdb_storage_iter_fn callback,
    void *user_data
);

/**
 * Get collection info from storage
 * 
 * Returns:
 * - VDB_OK: Success
 * - VDB_ERROR_INVALID_ARGUMENT: Null params
*/
vdb_status_t vdb_storage_get_info(
    const vdb_storage_t *storage,
    vdb_collection_info_t *out_info
);

/**
 * Get num of items in storage
*/
uint64_t vdb_storage_count(const vdb_storage_t *storage);

#endif /* VDB_STORAGE_H */