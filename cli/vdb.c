/**
 * vdb.c - Command-line interface for the VDB vector database
 *
 * This is the main entry point for the VDB CLI tool.
 * It provides commands for creating collections, ingesting data,
 * and querying the vector database.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vdb/types.h"
#include "vdb/collection.h"

#define VDB_VERSION "0.1.0"

/**
 * Print usage information
 */
static void print_usage(const char *prog_name) {
    printf("VDB - Vector Database v%s\n", VDB_VERSION);
    printf("\n");
    printf("Usage: %s <command> [options]\n", prog_name);
    printf("\n");
    printf("Commands:\n");
    printf("  help              Show this help message\n");
    printf("  version           Print version information\n");
    printf("  create <name> <dim> <metric>   Create a new collection\n");
    printf("                    - name: Collection name\n");
    printf("                    - dim: Vector dimension (1-%d)\n", VDB_COLLECTION_MAX_DIM);
    printf("                    - metric: 'cosine' or 'euclidean'\n");
    printf("\n");
    printf("Coming in Step 3:\n");
    printf("  ingest            Ingest vectors into a collection\n");
    printf("  query             Query for similar vectors\n");
    printf("\n");
}

/**
 * Parse metric from string
*/
static int parse_metric(const char *metric_str, vdb_metric_t *out_metric) {
    if (strcmp(metric_str, "cosine") == 0) {
        *out_metric = VDB_METRIC_COSINE;
        return 0;
    } else if (strcmp(metric_str, "euclidean") == 0) {
        *out_metric = VDB_METRIC_EUCLIDEAN;
        return 0;
    }
    return -1;
}

/**
 * Handle 'create' command 
*/
static int cmd_create(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Error: 'create' requires 3 arguments: <name> <dim> <metric>\n");
        fprintf(stderr, "Example: %s create my-collection 128 cosine\n", argv[0]);
        return 1;
    }

    const char *name = argv[2];
    const char *dim_str = argv[3];
    const char *metric_str = argv[4];

    // parse dimension
    char *endptr;
    long dim_long = strtol(dim_str, &endptr, 10);
    if (*endptr != '\0' || dim_long <= 0 || dim_long > VDB_COLLECTION_MAX_DIM) {
        fprintf(stderr, "Error: Invalid dimension '%s' (must be 1-%d)\n", dim_str, VDB_COLLECTION_MAX_DIM);
        return 1;
    }
    u_int32_t dim = (uint32_t)dim_long;

    // parse metric
    vdb_metric_t metric;
    if (parse_metric(metric_str, &metric) != 0) {
        fprintf(stderr, "Error, Invalid metric '%s' (must be 'cosine' or 'euclidean')\n", metric_str);
        return 1;
    }

    // create collection
    vdb_collection_t *coll = NULL;
    vdb_status_t status = vdb_collection_create(name, dim, metric, &coll);

    if (status != VDB_OK) {
        fprintf(stderr, "Error: Failed to create collection: %s\n", 
            vdb_status_to_string(status));
        return 1;
    }

    // get and display info
    vdb_collection_info_t info;
    status = vdb_collection_get_info(coll, &info);
    if (status != VDB_OK) {
        fprintf(stderr, "Error: Failed to get collection info: %s\n", 
            vdb_status_to_string(status));
        vdb_collection_close(&coll);
        return 1;
    }

    printf("  Created collection '%s'\n", info.name);
    printf("  Dimension: %u\n", info.dim);
    printf("  Metric: %s\n", vdb_metric_to_string(info.metric));
    printf("  Vectors: %lu\n", (unsigned long)info.num_vectors);
    printf("\n");
    printf("Note: Current collection is in-memory only (persistence in development)\n");

    // close collection
    vdb_collection_close(&coll);

    return 0;

}

/**
 * Main entry point
 */
int main(int argc, char *argv[]) {
    // If no arguments, show usage
    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }

    const char *command = argv[1];

    // Handle commands
    if (strcmp(command, "help") == 0 || strcmp(command, "-h") == 0 || strcmp(command, "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    } else if (strcmp(command, "version") == 0 || strcmp(command, "-v") == 0 || strcmp(command, "--version") == 0) {
        printf("VDB v%s\n", VDB_VERSION);
        return 0;
    } else if (strcmp(command, "create") == 0) {
        return cmd_create(argc, argv);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        fprintf(stderr, "Run '%s help' for usage information.\n", argv[0]);
        return 1;
    }

    return 0;
}

