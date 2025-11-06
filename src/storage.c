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