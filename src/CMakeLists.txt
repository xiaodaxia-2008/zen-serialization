include(GenerateExportHeader)

add_library(zen_serialization SHARED archive.cpp)
generate_export_header(zen_serialization)

target_link_libraries(zen_serialization PUBLIC spdlog::spdlog nlohmann_json::nlohmann_json)
target_include_directories(zen_serialization
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_compile_options(zen_serialization PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)