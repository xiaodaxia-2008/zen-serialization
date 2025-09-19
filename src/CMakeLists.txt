include(GenerateExportHeader)

add_library(zen_serialization SHARED archive.cpp)
generate_export_header(zen_serialization)

target_sources(zen_serialization
    PUBLIC
    FILE_SET headers TYPE HEADERS
    FILES
    zen_serialization/archive.h
    zen_serialization/archive_base.h
    zen_serialization/base64.h
    zen_serialization/binary_serializer.h
    zen_serialization/json_serializer.h
    zen_serialization/range_size.h
    zen_serialization/serializer.h
)

target_link_libraries(zen_serialization
    PUBLIC
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    Boost::pfr
)

target_include_directories(zen_serialization
    PUBLIC
    # $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_compile_options(zen_serialization PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)

install(TARGETS zen_serialization
    EXPORT zen_serialization-config
    FILE_SET headers
)

install(EXPORT zen_serialization-config
    DESTINATION cmake
    NAMESPACE zen::
)