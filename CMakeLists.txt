cmake_minimum_required(VERSION 3.11)
project(LpSamplers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_CXX_COMPILER)
  if(APPLE)
    set(CMAKE_CXX_COMPILER "g++-14")
  elseif(UNIX)
    set(CMAKE_CXX_COMPILER "g++")
  endif()
endif()

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.\n")
endif()

# Add library target
add_library(lpsampling STATIC
    src/CountSketch.cpp
    src/MurmurHash3.cpp
    src/FpEstimator.cpp
)
target_include_directories(lpsampling
    PUBLIC ${PROJECT_SOURCE_DIR}/include/lp_sampling
)

add_library(cxxopts INTERFACE)
# add_library(cxxopts::cxxopts ALIAS cxxopts)
add_subdirectory(include/cxxopts)

# Example test/executable
add_executable(countsketch_test
    tests/countsketch.cpp
)
target_link_libraries(countsketch_test PRIVATE lpsampling)
target_link_libraries(countsketch_test PRIVATE cxxopts)

add_executable(f2estimate_test
    tests/f2estimate.cpp
)
target_link_libraries(f2estimate_test PRIVATE lpsampling)

add_executable(stream_generator
    tests/stream_generator.cpp
)
target_link_libraries(stream_generator PRIVATE cxxopts)