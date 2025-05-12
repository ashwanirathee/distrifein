cmake_minimum_required(VERSION 3.14)
project(distrifein VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionally set a flag to build examples/tests
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTS "Build tests" ON)

# Get all .cpp files in src/
file(GLOB SOURCES "src/*.cc")

# Add them to the library
add_library(distrifein ${SOURCES})

# Public headers
target_include_directories(distrifein
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# ========================================
# Examples
# ========================================
if(BUILD_EXAMPLES)
    # Get all .cpp files in src/
    file(GLOB SOURCES "apps/*.cc")

    add_executable(main ${SOURCES})
    target_link_libraries(main PRIVATE distrifein)
endif()

# ========================================
# Tests
# ========================================
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
