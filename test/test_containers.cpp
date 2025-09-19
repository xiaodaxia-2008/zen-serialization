#include <zen_serialization/archive.h>

#include <catch.hpp>
#include <fmt/ranges.h>

#include <queue>
#include <stack>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <forward_list>
#include <list>
#include <map>
#include <set>

using namespace zen;

template <typename TOut, typename TIn>
void test_queue()
{
    using T = std::queue<int>;
    T value_in, value_out;
    value_in.

}

template <typename TOut, typename TIn>
void test_stack()
{
    std::string name = "stack";
    using T = std::stack<std::pair<int, std::string>>;
    T value_in, value_out;
    value_in.emplace(std::pair<int, std::string>{1, "one"});
    value_in.emplace(std::pair<int, std::string>{2, "two"});
    value_in.emplace(std::pair<int, std::string>{3, "three"});

    std::stringstream ss;
    OutArchive oar{TOut{ss}};
    oar(make_nvp(name, value_in));
    oar.Flush();

    InArchive iar{TIn{ss}};
    iar(make_nvp(name, value_out));

    REQUIRE(fmt::format("{}", value_in) == fmt::format("{}", value_out));
}

template <typename TOut, typename TIn>
void test_vec_map()
{
    using T = std::vector<std::map<int, std::string>>;
    T value_in, value_out;

    value_in = {
        {
            {1, "one"},
            {2, "two"},
            {3, "three"},
        },
        {
            {4, "four"},
            {5, "five"},
            {6, "six"},
        },
    };

    std::stringstream ss;
    OutArchive oar{TOut{ss}};
    oar(make_nvp("value", value_in));
    oar.Flush();

    InArchive iar{TIn{ss}};
    iar(make_nvp("value", value_out));

    REQUIRE(fmt::format("{}", value_in) == fmt::format("{}", value_out));
}

template <typename TOut, typename TIn>
void test_vec_list()
{
    std::vector<std::list<int>> value_in, value_out;
    value_in = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8},
    };

    std::stringstream ss;
    OutArchive oar{TOut{ss}};
    oar(make_nvp("value", value_in));
    oar.Flush();

    InArchive iar{TIn{ss}};
    iar(make_nvp("value", value_out));

    REQUIRE(value_in == value_out);
}

TEST_CASE("containers", "[vectorlist][json]")
{
    test_vec_list<JsonSerializer, JsonDeserializer>();
}

TEST_CASE("containers", "[vectormap][json]")
{
    test_vec_map<JsonSerializer, JsonDeserializer>();
}

TEST_CASE("containers", "[stackpair][json]")
{
    test_stack<JsonSerializer, JsonDeserializer>();
}

TEST_CASE("containers", "[vectorlist][binary]")
{
    test_vec_list<BinarySerializer, BinaryDeserializer>();
}

TEST_CASE("containers", "[vectormap][binary]")
{
    test_vec_map<BinarySerializer, BinaryDeserializer>();
}

TEST_CASE("containers", "[stackpair][binary]")
{
    test_stack<BinarySerializer, BinaryDeserializer>();
}
