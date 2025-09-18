find_package(Catch2 CONFIG REQUIRED)

link_libraries(Catch2::Catch2WithMain zen_serialization)

add_executable(tests
    test_enum.cpp
    test_struct.cpp
)

include(CTest)
include(Catch)
catch_discover_tests(tests)
