/**
 * types.c - Implementation of core type utilities
*/

#include "vdb/types.h"
#include "vdb/collection.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Convert metric enum to string
*/
const char* vdb_metric_to_string(vdb_metric_t metric) {
    switch (metric) {
        case VDB_METRIC_COSINE:
            return "cosine";
        case VDB_METRIC_EUCLIDEAN:
            return "euclidean";
        default:
            return "unknown";
    }
}

/**
 * Convert status code to string
*/
const char* vdb_status_to_string(vdb_status_t status) {
    switch (status) {
        case VDB_OK:
            return "OK";
        case VDB_ERROR_INVALID_ARGUMENT:
            return "Invalid argument";
        case VDB_ERROR_OUT_OF_MEMORY:
            return "Out of memory";
        case VDB_ERROR_IO:
            return "I/O error";
        case VDB_ERROR_NOT_FOUND:
            return "Not found";
        case VDB_ERROR_ALREADY_EXISTS:
            return "Already exists";
        case VDB_ERROR_CORRUPTED:
            return "Data corrupted";
        case VDB_ERROR_DIMENSION_MISMATCH:
            return "Dimension mismatch";
        case VDB_ERROR_UNKNOWN:
        default:
            return "Unknown error";
    }
}

// validate metric type
bool vdb_metric_is_valid(vdb_metric_t metric) {
    return (metric == VDB_METRIC_COSINE || metric == VDB_METRIC_EUCLIDEAN);
}

// create a vector
vdb_status_t vdb_vector_create(uint32_t dim, vdb_vector_t *out_vector) {
    if (out_vector == NULL) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }
    
    if (dim == 0 || dim > VDB_COLLECTION_MAX_DIM) {
        out_vector->dim = 0;
        out_vector->data = NULL;
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    // allocate and zero-initialize
    float *data = (float*)calloc(dim, sizeof(float));
    if (data == NULL) {
        out_vector->dim = 0;
        out_vector->data = NULL;
        return VDB_ERROR_OUT_OF_MEMORY;
    }

    out_vector->dim = dim;
    out_vector->data = data;
    return VDB_OK;
}

// free a vector
void vdb_vector_free(vdb_vector_t *vector) {
    if (vector == NULL) {
        return;
    }

    if (vector->data != NULL) {
        free(vector->data);
        vector->data = NULL;
    }
    vector->dim = 0;
}

// copy a vector (deep copy)
vdb_status_t vdb_vector_copy(const vdb_vector_t *src, vdb_vector_t *dst) {
    if (src == NULL || dst == NULL) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    if (src->data == NULL || src->dim == 0) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    // create dest vector
    vdb_status_t status = vdb_vector_create(src->dim, dst);
    if (status != VDB_OK) {
        return status;
    }

    // copy data
    memcpy(dst->data, src->data, src->dim * sizeof(float));
    return VDB_OK;
}

// validate ID string
bool vdb_id_is_valid(const char *id) {
    if (id == NULL || id[0] == '\0') {
        return false;
    }

    // check length and that it's null-terminated within bounds
    size_t len = 0;
    for (len = 0; len < VDB_ID_MAX_LEN; len++) {
        if (id[len] == '\0') {
            break;
        }

        // ensure all chars printable
        if (!isprint((unsigned char)id[len])) {
            return false;
        }
    }

    // must have found null-terminator before VDB_ID_MAX_LEN
    if (len >= VDB_ID_MAX_LEN) {
        return false;
    }
    return true;
}

// safely copy an ID
vdb_status_t vdb_id_copy(const char *src, vdb_id_t dst) {
    if (!vdb_id_is_valid(src)) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    // safe copy with explicit null termination
    strncpy(dst, src, VDB_ID_MAX_LEN - 1);
    dst[VDB_ID_MAX_LEN-1] = '\0';

    return VDB_OK;
}