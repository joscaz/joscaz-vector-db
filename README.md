# joscaz-vector-db

A minimal Chroma-like vector database implemented in C.

## Overview

This is a high-performance vector database library and CLI tool written in C17. It supports:
- Vector similarity search (cosine and euclidean distance)
- Metadata filtering
- Persistent storage with write-ahead logging
- SIMD-accelerated similarity computation
- HNSW indexing

## Project Structure

```
joscaz-vector-db/
├── CMakeLists.txt          # Build configuration
├── include/vdb/            # Public API headers
├── src/                    # Implementation files
├── cli/                    # CLI tool
│   └── vdb.c
├── tests/                  # Test suite
│   ├── test_framework.h    # Custom test framework
│   └── test_main.c         # Test runner
└── third_party/            # External dependencies
```

## Building

This project uses CMake and requires a C17-compatible compiler (GCC 7+, Clang 5+).

### Prerequisites

- CMake 3.15 or higher
- GCC or Clang compiler
- POSIX-compliant system (macOS or Linux)

### Build Instructions

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
make

# Optionally, build with verbose output
make VERBOSE=1
```

### Build Targets

- `vdb` - Static library
- `vdb_cli` - Command-line interface
- `vdb_tests` - Test suite

## Usage

### Running the CLI

```bash
# From the build directory
./vdb_cli help
./vdb_cli version
```

### Running Tests

```bash
# From the build directory
./vdb_tests

# Or use CTest
ctest --verbose
```

## Development Status

This project is under active development. Current implementation status:

- [x] Step 1: Project scaffold with CMake build system
- [ ] Step 2: Core types and collection API
- [ ] Step 3: Storage layer with WAL
- [ ] Step 4: Similarity computation and exact search
- [ ] Step 5: Metadata indexing
- [ ] Step 6: HNSW index
- [ ] Step 7: Observability and polish

## Design Principles

- **Safety First**: Comprehensive input validation and error handling
- **Clarity Over Cleverness**: Well-commented, readable code
- **Performance**: SIMD acceleration where applicable
- **Simplicity**: Start with simple, correct implementations before optimization

## License

[To be determined]

## Contributing

This is a learning/demonstration project. Contributions welcome!