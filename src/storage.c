/**
 * storage.c - Persistent storage implementation
 * 
 * This implements append-only storage with WAL for crash recov
 * uses POSIX APIs (open, write, fsync, mmap) for file ops
*/

#include "vdb/storage.h"
#include "vdb/collection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

/* Max path len */
#define MAX_PATH 1024

/* WAL record types */
#define WAL_RECORD_APPEND 1

/* WAL record header */
typedef struct {
    uint8_t type;
    uint32_t id_len; // record type
    uint32_t vector_dim; // vector dimension
    uint32_t metadata_len; // len of metadata (0 if none)
} __attribute__((packed)) wal_record_header_t;

/**
 * Storage structure (opaque to users)
*/
struct vdb_storage {
    char base_dir[MAX_PATH];
    char name[VDB_COLLECTION_NAME_MAX_LEN];
    uint32_t dim;
    vdb_metric_t metric;
    uint64_t count;

    /* File descriptors */
    int meta_fd;
    int embeddings_fd;
    int ids_fd;
    int metadata_fd;
    int wal_fd;
};

/**
 * Build path to collection directory
*/
static void build_collection_path(const char *base_dir, const char *name, char *out_path) {
    snprintf(out_path, MAX_PATH, "%s/%s", base_dir, name);
}

/** 
 * Build path to a file within collection
*/
static void build_file_path(const char *base_dir, const char *name,
    const char *filename, char *out_path) {
        sprintf(out_path, MAX_PATH, "%s/%s/%s", base_dir, name, filename);
}

/** 
 * Create directory (and parents if needed)
*/
static vdb_status_t create_directory(const char *path) {
    struct stat st;

    /* Check if already exists */
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return VDB_OK; /* Already exists */
        } else {
            return VDB_ERROR_ALREADY_EXISTS; /* exists but is not a dir */
        }
    }

    /* try to create */
    if (mkdir(path, 0755) == 0) {
        return VDB_OK;
    }

    /* Check if parent exists */
    if (errno == ENOENT) { // ENOENT - no such file or dir
        /* Tryto create parent */
        char parent[MAX_PATH];
        strncpy(parent, path, MAX_PATH - 1);
        parent[MAX_PATH-1] = '\0';

        char *last_slash = strrchr(parent, '/');
        if (last_slash != NULL && last_slash != parent) {
            *last_slash = '\0';
            vdb_status_t status = create_directory(parent);
            if (status != VDB_OK) {
                return status;
            }
            /* try again */
            if (mkdir(path, 0755) == 0) {
                return VDB_OK;
            }
        }
    }

    return VDB_ERROR_IO;
}

/**
 * Write collection metadata file
*/
static vdb_status_t write_collection_meta(const char *path, uint32_t dim, 
                                        vdb_metric_t metric, u_int64_t count) {
    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        return VDB_ERROR_IO;
    }
    fprintf(fp, "dimension=%u\n", dim);
    fprintf(fp, "metric=%d\n", (int)metric);
    fprintf(fp, "count=%lu\n", (unsigned long)count);

    if (fclose(fp) != 0) {
        return VDB_ERROR_IO;
    }

    return VDB_OK;
}

/** 
 * Read collection metadata file
*/
static vdb_status_t read_collection_meta(const char *path, uint32_t *dim, 
                                        vdb_metric_t *metric, uint64_t *count) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return VDB_ERROR_NOT_FOUND;
    }

    int metric_int;
    unsigned long count_ul;

    int n = fscanf(fp, "dimension=%u\nmetric=%d\ncount=%lu\n",
                    dim, &metric_int, &count_ul);
    
    fclose(fp);

    if (n != 3) {
        return VDB_ERROR_CORRUPTED;
    }

    *metric = (vdb_metric_t)metric_int;
    *count = (uint64_t)count_ul;

    /* Validate */
    if (*dim == 0 || *dim > VDB_COLLECTION_MAX_DIM || !vdb_metric_is_valid(*metric)) {
        return VDB_ERROR_CORRUPTED;
    }

    return VDB_OK;
}

/** 
 * Open segment files for writing - append mode
*/
static vdb_status_t open_segment_files(vdb_storage_t *storage) {
    char path[MAX_PATH];

    /* Open embeddings segment */
    build_file_path(storage->base_dir, storage->name, "embeddings.seg", path);
    storage->embeddings_fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (storage->embeddings_fd < 0) {
        return VDB_ERROR_IO;
    }

    /* Open IDs segment */
    build_file_path(storage->base_dir, storage->name, "ids.seg", path);
    storage->ids_fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (storage->ids_fd < 0) {
        close(storage->embeddings_fd);
        return VDB_ERROR_IO;
    }

    /* Open metaddata segment */
    build_file_path(storage->base_dir, storage->name, "metadata.seg", path);
    storage->metadata_fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (storage->metadata_fd < 0) {
        close(storage->embeddings_fd);
        close(storage->ids_fd);
        return VDB_ERROR_IO;
    }

    /* Open WAL */
    build_file_path(storage->base_dir, storage->name, "wal.log", path);
    storage->wal_fd = open(path, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (storage->wal_fd < 0) {
        close(storage->embeddings_fd);
        close(storage->ids_fd);
        close(storage->metadata_fd);
        return VDB_ERROR_IO;
    }

    return VDB_OK;
}

/** 
 * Close segment files
*/
static void close_segment_files(vdb_storage_t *storage) {
    if (storage->embeddings_fd >= 0) {
        close(storage->embeddings_fd);
        storage->embeddings_fd = -1;
    }
    if (storage->ids_fd >= 0) {
        close(storage->ids_fd);
        storage->ids_fd = -1;
    }
    if (storage->metadata_fd >= 0) {
        close(storage->metadata_fd);
        storage->metadata_fd = -1;
    }
    if (storage->wal_fd >= 0) {
        close(storage->wal_fd);
        storage->wal_fd = -1;
    }
}

/** 
 * Recover from WAL (replay uncommitted records)
 * for now, ill implement a simple truncate-on success strat:
 * if WAL is non-empty on open, we assume crash and replay
*/

static vdb_status_t recover_from_wal(vdb_storage_t *storage) {
    /* get wal size */
    off_t wal_size = lseek(storage->wal_fd, 0, SEEK_END);
    if (wal_size < 0) {
        return VDB_ERROR_IO;
    }

    if (wal_size == 0) {
        return VDB_OK; // no recov needed
    }

    /** for this simple impl, we'll just truncate the WAL
     * after investigatin, in a real system we'd replay records
     * Since we fsync after each append, partial records indicate crash
     * during write, so we conservatively truncate
    */
   if (ftruncate(storage->wal_fd, 0) != 0) {
    return VDB_ERROR_IO;
   }

   return VDB_OK;
}

/** 
 * Create a new collection on disk
*/
vdb_status_t vdb_storage_create(
    const char *base_dir,
    const char *name,
    uint32_t dim,
    vdb_metric_t metric, 
    vdb_storage_t **out_storage
) {
    if (base_dir == NULL | name == NULL || out_storage == NULL) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    /* Validata params */
    vdb_status_t status = vdb_collection_validate_params(name, dim, metric);
    if (status != VDB_OK) {
        return status;
    }

    /* check if collection already exists */
    char coll_path[MAX_PATH];
    build_collection_path(base_dir, name, coll_path);

    struct stat st;
    if (stat(coll_path, &st) == 0) {
        return VDB_ERROR_ALREADY_EXISTS;
    }

    /* create base dir */
    status = create_directory(base_dir);
    if (status != VDB_OK && status != VDB_ERROR_ALREADY_EXISTS) {
        return status;
    }

    /* create collection dir */
    status = create_directory(coll_path);
    if (status != VDB_OK) {
        return status;
    }

    /* write metadata file */
    char meta_path[MAX_PATH];
    build_file_path(base_dir, name, "collection.meta", meta_path);
    status = write_collection_meta(meta_path, dim, metric, 0);
    if (status != VDB_OK) {
        return status;
    }

    /* Allocate storage structure */
    vdb_storage_t *storage = (vdb_storage_t*)calloc(1, sizeof(vdb_storage_t));
    if (storage == NULL) {
        return VDB_ERROR_OUT_OF_MEMORY;
    }

    /* init fields */
    strncpy(storage->base_dir, base_dir, MAX_PATH - 1);
    storage->base_dir[MAX_PATH - 1] = '\0';
    strncpy(storage->name, name, VDB_COLLECTION_NAME_MAX_LEN - 1);
    storage->name[VDB_COLLECTION_NAME_MAX_LEN - 1] = '\0';
    storage->dim = dim;
    storage->metric = metric;
    storage->count = 0;
    storage->embeddings_fd = -1;
    storage->ids_fd = -1;
    storage->metadata_fd = -1;
    storage->wal_fd = -1;

    /* open segment files */
    status = open_segment_files(storage);
    if (status != VDB_OK) {
        free(storage);
        return status;
    }

    /* recover from WAL if needed */
    status = recover_from_wal(storage);
    if (status != VDB_OK) {
        close_segment_files(storage);
        free(storage);
        return status;
    }

    *out_storage = storage;
    return VDB_OK;
}

/** 
 * Close storage
*/
void vdb_storage_close(vdb_storage_t **storage) {
    if (storage == NULL || *storage == NULL) {
        return;
    }

    vdb_storage_t *s = *storage;

    /* flush and close files */
    close_segment_files(s);

    /* update metadata file with final cnt */
    char meta_path[MAX_PATH];
    build_file_path(s->base_dir, s->name, "collection.meta", meta_path);
    write_collection_meta(meta_path, s->dim, s->metric, s->count);

    free(s);
    *storage = NULL;
}

/** 
 * Write WAL record
*/
static vdb_status_t write_wal_record(vdb_storage_t *storage, const vdb_item_t *item) {
    /* build WAL record */
    wal_record_header_t header;
    header.type = WAL_RECORD_APPEND;
    header.id_len = (uint32_t)strlen(item->id);
    header.vector_dim = item->vector.dim;
    header.metadata_len = item->metadata ? (uint32_t)strlen(item->metadata) : 0;

    // write header
    if (write(storage->wal_fd, &header, sizeof(header)) != sizeof(header)) {
        return VDB_ERROR_IO;
    }

    // write ID
    if (write(storage->wal_fd, item->id, header.id_len) != (ssize_t)header.id_len) {
        return VDB_ERROR_IO;
    }

    // write vector
    size_t vector_bytes = item->vector.dim * sizeof(float);
    if (write(storage->wal_fd, item->vector.data, vector_bytes) != (ssize_t)vector_bytes) {
        return VDB_ERROR_IO;
    }

    // write metadata if present
    if (header.metadata_len > 0) {
        if (write(storage->wal_fd, item->metadata, header.metadata_len) != (ssize_t)header.metadata_len) {
            return VDB_ERROR_IO;
        }
    }

    // fsync WAL (for durability)
    if (fsync(storage->wal_fd) != 0) {
        return VDB_ERROR_IO;
    }

    return VDB_OK;
}

/** 
 * Append item to storage
*/
vdb_status_t vdb_storage_append(
    vdb_storage_t *storage,
    const vdb_item_t *item
) {
    if (storage == NULL || item == NULL) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    // validate dimension
    if (item->vector.dim != storage->dim) {
        return VDB_ERROR_DIMENSION_MISMATCH;
    }

    // validate ID
    if (!vdb_id_is_valid(item->id)) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    // step1: write to WAL and fsync
    vdb_status_t status = write_wal_record(storage, item);
    if (status != VDB_OK) {
        return status;;
    }

    // step2: write to embeddings segment
    size_t vector_bytes = storage->dim * sizeof(float);
    if (write(storage->embeddings_fd, item->vector.data, vector_bytes) != (ssize_t)vector_bytes) {
        return VDB_ERROR_IO;
    }

    // step 3: write to IDs semgent - fixed 64 bytes
    vdb_id_t padded_id;
    memset(padded_id, 0, VDB_ID_MAX_LEN);
    vdb_id_copy(item->id, padded_id);
    if (write(storage->ids_fd, padded_id, VDB_ID_MAX_LEN) != VDB_ID_MAX_LEN) {
        return VDB_ERROR_IO;
    }

    // step4 : write to metadata segment (length-prefixed)
    uint32_t metadata_len = item->metadata ? (uint32_t)strlen(item->metadata) : 0;
    if (write(storage->metadata_fd, &metadata_len, sizeof(metadata_len)) != sizeof(metadata_len)) {
        return VDB_ERROR_IO;
    }

    if (metadata_len > 0) {
        if (write(storage->metadata_fd, item->metadata, metadata_len) != (ssize_t)metadata_len) {
            return VDB_ERROR_IO;
        }
    }

    // step5 - fsync all segments
    if (fsync(storage->embeddings_fd) != 0 ||
        fsync(storage->ids_fd) != 0 ||
        fsync(storage->metadata_fd) != 0) {
        return VDB_ERROR_IO;
    }

    // step6: increement count
    storage->count++;

    // step7 - truncate WAL (record is committed now)
    if (ftruncate(storage->wal_fd, 0) != 0) {
        // non fatal, we can recover on next open
    }
    return VDB_OK;
}