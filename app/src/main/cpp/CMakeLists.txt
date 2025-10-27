# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name.
project("openmicstream")

# Find the Oboe package using the CONFIG mode provided by Prefab.
find_package(oboe REQUIRED CONFIG)

# Define our library and list ALL of its source files in one command.
add_library(
        # Sets the name of the library.
        openmicstream

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to all source files.
        native-lib.cpp
        AudioEngine.cpp)

# Link our library against Oboe and Android's logging library.
target_link_libraries(
        # Specifies the target library.
        openmicstream

        # Links the Oboe library to our library.
        oboe::oboe

        # Links the log library for native logging (logcat).
        log)