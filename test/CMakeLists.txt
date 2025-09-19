find_package(Catch2 CONFIG REQUIRED)

link_libraries(Catch2::Catch2WithMain zen_serialization)

add_executable(tests
    test_enum.cpp
    test_string.cpp
    test_struct.cpp
    test_containers.cpp
    test_associative.cpp
    test_optional.cpp
    test_basic_types.cpp
    test_smart_pointers.cpp
    test_nested_containers.cpp
    test_custom_class.cpp
    test_unordered_associative.cpp
)