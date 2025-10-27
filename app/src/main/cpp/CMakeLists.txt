# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name.
project("openmicstream")

# --- Opus Configuration ---
set(OPUS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/opus)

# Force Opus to build as a basic C library without any assembly or intrinsics
set(OPUS_BUILD_SHARED_LIBRARY OFF CACHE BOOL "" FORCE)
set(OPUS_INSTALL_PKG_CONFIG_MODULE OFF CACHE BOOL "" FORCE)
set(OPUS_INSTALL_CMAKE_CONFIG_MODULE OFF CACHE BOOL "" FORCE)

# Disable all CPU-specific optimizations
set(OPUS_DISABLE_INTRINSICS ON CACHE BOOL "" FORCE)
set(OPUS_DISABLE_ASM ON CACHE BOOL "" FORCE)
set(OPUS_MAY_HAVE_NEON OFF CACHE BOOL "" FORCE)
set(OPUS_PRESUME_NEON OFF CACHE BOOL "" FORCE)
set(OPUS_X86_MAY_HAVE_SSE OFF CACHE BOOL "" FORCE)
set(OPUS_X86_MAY_HAVE_SSE2 OFF CACHE BOOL "" FORCE)
set(OPUS_X86_MAY_HAVE_SSE4_1 OFF CACHE BOOL "" FORCE)
set(OPUS_X86_MAY_HAVE_AVX OFF CACHE BOOL "" FORCE)

# Use fixed-point for mobile
set(OPUS_FIXED_POINT ON CACHE BOOL "" FORCE)
set(OPUS_FLOAT_API OFF CACHE BOOL "" FORCE)

# Build Opus
add_subdirectory(${OPUS_SRC_DIR} ${CMAKE_BINARY_DIR}/opus EXCLUDE_FROM_ALL)

# Include directories
add_library(opus_headers INTERFACE)
target_include_directories(opus_headers INTERFACE
        ${OPUS_SRC_DIR}/include
        ${CMAKE_BINARY_DIR}/opus
)
# --- End Opus Configuration ---

# Find Oboe
find_package(oboe REQUIRED CONFIG)

# Your library
add_library(
        openmicstream
        SHARED
        native-lib.cpp
        AudioEngine.cpp)

target_link_libraries(
        openmicstream
        opus
        opus_headers
        oboe::oboe
        log)