/**
 * collection.c - Collection management implementation
*/

#include "vdb/collection.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// internal collection structure (opaque to users)
struct vdb_collection {
    char name[VDB_COLLECTION_NAME_MAX_LEN];
    uint32_t dim;
    vdb_metric_t metric;
    uint64_t num_vectors;
};

// validate collection parameters
vdb_status_t vdb_collection_validate_params(
    const char *name,
    uint32_t dim,
    vdb_metric_t metric
) {
    // validate name
    if (name == NULL || name[0] == '\0') {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    size_t name_len = strnlen(name, VDB_COLLECTION_NAME_MAX_LEN);
    if (name_len >= VDB_COLLECTION_NAME_MAX_LEN) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    // check printable chars only
    for (size_t i = 0; i < name_len; i++) {
        if (!isprint((unsigned char)name[i])) {
            return VDB_ERROR_INVALID_ARGUMENT;
        }
    }

    // validate dimension
    if (dim == 0 || dim > VDB_COLLECTION_MAX_DIM) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    // validate metric
    if (!vdb_metric_is_valid(metric)) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    return VDB_OK;
}

// Create new collection
vdb_status_t vdb_collection_create(
    const char *name,
    uint32_t dim,
    vdb_metric_t metric,
    vdb_collection_t **out_collection
) {
    if (out_collection == NULL) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    // validate params
    vdb_status_t status = vdb_collection_validate_params(name, dim, metric);
    if (status != VDB_OK) {
        *out_collection = NULL;
        return status;
    }

    // allocate collection
    vdb_collection_t *coll = (vdb_collection_t*)malloc(sizeof(vdb_collection_t));
    if (coll == NULL) {
        *out_collection = NULL;
        return VDB_ERROR_OUT_OF_MEMORY;
    }

    // init fields
    strncpy(coll->name, name, VDB_COLLECTION_NAME_MAX_LEN - 1);
    coll->name[VDB_COLLECTION_NAME_MAX_LEN - 1] = '\0';
    coll->dim = dim;
    coll->metric = metric;
    coll->num_vectors = 0;

    *out_collection = coll;
    return VDB_OK;
}

// close and free a collection
void vdb_collection_close(vdb_collection_t **collection) {
    if (collection == NULL || *collection == NULL) {
        return;
    }

    free(*collection);
    *collection = NULL;
}

// get collection info
vdb_status_t vdb_collection_get_info(
    const vdb_collection_t *collection,
    vdb_collection_info_t *out_info
) {
    if (collection == NULL || out_info == NULL) {
        return VDB_ERROR_INVALID_ARGUMENT;
    }

    // copy data to info struc
    strncpy(out_info->name, collection->name, VDB_COLLECTION_NAME_MAX_LEN);
    out_info->name[VDB_COLLECTION_NAME_MAX_LEN - 1] = '\0';
    out_info->dim = collection->dim;
    out_info->metric = collection->metric;
    out_info->num_vectors = collection->num_vectors;

    return VDB_OK;
}