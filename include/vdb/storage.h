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

#endif /* VDB_STORAGE_H */