#include <catch.hpp>
#include <zen_serialization/archive.h>
#include <fmt/ranges.h>

using namespace zen;

template <typename TOut, typename TIn>
void test_nested_containers()
{
    // 测试嵌套的vector
    std::vector<std::vector<int>> nested_vector = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    
    // 测试map中的vector
    std::map<std::string, std::vector<int>> map_vector = {
        {"first", {1, 2, 3}},
        {"second", {4, 5, 6}},
        {"third", {7, 8, 9}}
    };
    
    // 测试vector中的map
    std::vector<std::map<std::string, int>> vector_map = {
        {{"a", 1}, {"b", 2}},
        {{"c", 3}, {"d", 4}},
        {{"e", 5}, {"f", 6}}
    };

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("nested_vector", nested_vector));
    oar(make_nvp("map_vector", map_vector));
    oar(make_nvp("vector_map", vector_map));
    oar.Flush();

    // 反序列化
    std::vector<std::vector<int>> nested_vector_out;
    std::map<std::string, std::vector<int>> map_vector_out;
    std::vector<std::map<std::string, int>> vector_map_out;

    InArchive iar{TIn(ss)};
    iar(make_nvp("nested_vector", nested_vector_out));
    iar(make_nvp("map_vector", map_vector_out));
    iar(make_nvp("vector_map", vector_map_out));

    // 验证结果
    CHECK(nested_vector_out == nested_vector);
    CHECK(map_vector_out == map_vector);
    CHECK(vector_map_out == vector_map);
}

template <typename TOut, typename TIn>
void test_complex_nested_structures()
{
    // 测试更复杂的嵌套结构
    std::map<std::string, std::vector<std::map<int, std::string>>> complex_structure;
    complex_structure["group1"] = {
        {{1, "one"}, {2, "two"}},
        {{3, "three"}, {4, "four"}}
    };
    complex_structure["group2"] = {
        {{5, "five"}, {6, "six"}},
        {{7, "seven"}, {8, "eight"}}
    };

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("complex_structure", complex_structure));
    oar.Flush();

    // 反序列化
    std::map<std::string, std::vector<std::map<int, std::string>>> complex_structure_out;

    InArchive iar{TIn(ss)};
    iar(make_nvp("complex_structure", complex_structure_out));

    // 验证结果
    CHECK(complex_structure_out == complex_structure);
}

TEST_CASE("nested_containers", "[nested]")
{
    test_nested_containers<JsonSerializer, JsonDeserializer>();
    test_nested_containers<BinarySerializer, BinaryDeserializer>();
    test_complex_nested_structures<JsonSerializer, JsonDeserializer>();
    test_complex_nested_structures<BinarySerializer, BinaryDeserializer>();
}