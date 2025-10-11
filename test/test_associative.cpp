#include <zen_serialization/archive.h>

#include <catch.hpp>

using namespace zen;

template <typename TOut, typename TIn>
void test_aggregate()
{
    std::set<std::string> set = {"a", "b", "c"};
    auto &v = *set.begin();
    std::map<std::string, int> map = {{"a", 1}, {"b", 2}, {"c", 3}};
    auto &v2 = *map.begin();
    using value_type = std::map<std::string, int>::value_type;
    if constexpr (requires { typename std::tuple_element_t<0, float>; }) {
        SPDLOG_INFO("not a pair");
    }

    if constexpr (requires {
                      requires !std::is_const_v<
                          typename std::tuple_element_t<0, value_type>>;
                  }) {
        using key_type = std::tuple_element_t<0, value_type>;
        SPDLOG_INFO("is a pair");
    }
}

TEST_CASE("associative", "[map]")
{
    test_aggregate<JsonSerializer, JsonDeserializer>();
    test_aggregate<BinarySerializer, BinaryDeserializer>();
}