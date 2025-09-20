cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(ZEN_SERIALIZATION_BUILD_TEST "Build the testing suite" ON)
option(ZEN_SERIALIZATION_BUILD_EXAMPLE "Build the examples" ON)

project(zen-serialization VERSION 0.1.0
    DESCRIPTION "simple and easy serialization library for c++")

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS pfr)

add_subdirectory(src)

if(ZEN_SERIALIZATION_BUILD_TEST)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()

if(ZEN_SERIALIZATION_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
