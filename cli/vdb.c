/**
 * vdb.c - Command-line interface for the VDB vector database
 *
 * This is the main entry point for the VDB CLI tool.
 * It provides commands for creating collections, ingesting data,
 * and querying the vector database.
 */

#include <stdio.h>
#include <string.h>

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
    printf("\n");
    printf("More commands will be added in subsequent steps:\n");
    printf("  create            Create a new collection\n");
    printf("  ingest            Ingest vectors into a collection\n");
    printf("  query             Query for similar vectors\n");
    printf("\n");
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
    } else {
        printf("Unknown command: %s\n", command);
        printf("Run '%s help' for usage information.\n", argv[0]);
        return 1;
    }

    return 0;
}

