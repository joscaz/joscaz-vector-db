cmake_minimum_required(VERSION 3.15)
project(joscaz_vector_db VERSION 0.1.0 LANGUAGES C)

# Set C17 standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Enable warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# POSIX feature macros for macOS/Linux
add_definitions(-D_POSIX_C_SOURCE=200809L)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files for the library (empty for now, but ready for src/*.c)
file(GLOB_RECURSE VDB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.c")

# Create vdb static library
# For now, we'll add a placeholder so the library target exists
# We'll populate it with actual source files in Step 2
if(VDB_SOURCES)
    add_library(vdb STATIC ${VDB_SOURCES})
    target_include_directories(vdb PUBLIC ${CMAKE_SOURCE_DIR}/include)
else()
    # Create a dummy library for now
    add_library(vdb INTERFACE)
    target_include_directories(vdb INTERFACE ${CMAKE_SOURCE_DIR}/include)
endif()

# CLI executable
add_executable(vdb_cli cli/vdb.c)
if(VDB_SOURCES)
    target_link_libraries(vdb_cli vdb)
endif()

# Test executable
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.c")
add_executable(vdb_tests ${TEST_SOURCES})
if(VDB_SOURCES)
    target_link_libraries(vdb_tests vdb)
endif()
target_include_directories(vdb_tests PRIVATE ${CMAKE_SOURCE_DIR}/tests)

# Enable testing
enable_testing()
add_test(NAME vdb_tests COMMAND vdb_tests)

