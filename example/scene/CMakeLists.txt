include(GenerateExportHeader)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(base_lib SHARED base_node.cpp)
generate_export_header(base_lib)
target_link_libraries(base_lib PUBLIC zen_serialization)

add_library(derived_lib SHARED derived_node.cpp)
generate_export_header(derived_lib)
target_link_libraries(derived_lib PUBLIC base_lib)

add_executable(scene main.cpp)
target_link_libraries(scene PRIVATE derived_lib)
target_compile_features(scene PRIVATE cxx_std_23)

if(ZEN_SERIALIZATION_BUILD_TEST)
    add_test(NAME SceneTest COMMAND scene)
endif()