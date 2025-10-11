#include <zen_serialization/archive.h>

#include <catch.hpp>
#include <fmt/ranges.h>

#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>

using namespace zen;

template <std::ranges::range T>
void Equal(T &&a, T &&b)
{
    using value_type = std::ranges::range_value_t<T>;
    REQUIRE(std::ranges::distance(a) == std::ranges::distance(b));
    for (auto &&[e1, e2] : std::views::zip(a, b)) {
        if constexpr (std::ranges::range<value_type> &&
                      !std::equality_comparable<value_type>) {
            Equal(e1, e2);
        } else {
            REQUIRE(e1 == e2);
        }
    }
}

template <typename TOut, typename TIn>
void test_array()
{
    std::string name = "array";
    using T = std::array<int, 3>;
    T value_in, value_out;
    value_in[0] = 1;
    value_in[1] = 2;
    value_in[2] = 3;

    std::stringstream ss;
    OutArchive oar{TOut{ss}};
    oar(make_nvp(name, value_in));
    oar.Flush();

    InArchive iar{TIn{ss}};
    iar(make_nvp(name, value_out));

    Equal(value_in, value_out);
}

TEST_CASE("containers", "[array][json]")
{
    test_array<JsonSerializer, JsonDeserializer>();
}

TEST_CASE("containers", "[array][binary]")
{
    test_array<BinarySerializer, BinaryDeserializer>();
}

template <typename TOut, typename TIn, template <typename...> typename TMap>
void test_aggregate()
{
    std::string name = "map";
    using T = TMap<std::string, int>;
    T value_in, value_out;
    value_in.emplace("one", 1);
    value_in.emplace("two", 2);
    value_in.emplace("three", 3);
    value_in.emplace("two", 4);
    value_in.emplace("three", 6);

    std::stringstream ss;
    OutArchive oar{TOut{ss}};
    oar(make_nvp(name, value_in));
    oar.Flush();

    InArchive iar{TIn{ss}};
    iar(make_nvp(name, value_out));

    Equal(value_in, value_out);
}

TEST_CASE("containers", "[map][json]")
{
    test_aggregate<JsonSerializer, JsonDeserializer, std::map>();
}

TEST_CASE("containers", "[map][binary]")
{
    test_aggregate<BinarySerializer, BinaryDeserializer, std::map>();
}

TEST_CASE("containers", "[unordered_map][json]")
{
    test_aggregate<JsonSerializer, JsonDeserializer, std::unordered_map>();
}

TEST_CASE("containers", "[unordered_map][binary]")
{
    test_aggregate<BinarySerializer, BinaryDeserializer, std::unordered_map>();
}

TEST_CASE("containers", "[multimap][json]")
{
    test_aggregate<JsonSerializer, JsonDeserializer, std::multimap>();
}

TEST_CASE("containers", "[multimap][binary]")
{
    test_aggregate<BinarySerializer, BinaryDeserializer, std::multimap>();
}

TEST_CASE("containers", "[unordered_multimap][json]")
{
    test_aggregate<JsonSerializer, JsonDeserializer, std::unordered_multimap>();
}

TEST_CASE("containers", "[unordered_multimap][binary]")
{
    test_aggregate<BinarySerializer, BinaryDeserializer, std::unordered_multimap>();
}

template <typename TOut, typename TIn, template <typename...> typename TRng>
void test_range(bool print = false)
{
    using T = TRng<std::string>;
    T value_in, value_out;

    value_in = {
        "one", "two", "three", "four", "five", "six", "one", "six",
    };

    std::stringstream ss;
    OutArchive oar{TOut{ss}};
    oar(make_nvp("value", value_in));
    oar.Flush();

    InArchive iar{TIn{ss}};
    iar(make_nvp("value", value_out));

    Equal(value_in, value_out);
}

TEST_CASE("containers", "[vector][json]")
{
    test_range<JsonSerializer, JsonDeserializer, std::vector>();
}

TEST_CASE("containers", "[vector][binary]")
{
    test_range<BinarySerializer, BinaryDeserializer, std::vector>();
}

TEST_CASE("containers", "[list][json]")
{
    test_range<JsonSerializer, JsonDeserializer, std::list>();
}

TEST_CASE("containers", "[list][binary]")
{
    test_range<BinarySerializer, BinaryDeserializer, std::list>();
}

TEST_CASE("containers", "[forward_list][json]")
{
    test_range<JsonSerializer, JsonDeserializer, std::forward_list>();
}

TEST_CASE("containers", "[forward_list][binary]")
{
    test_range<BinarySerializer, BinaryDeserializer, std::forward_list>();
}

TEST_CASE("containers", "[deque][json]")
{
    test_range<JsonSerializer, JsonDeserializer, std::deque>();
}

TEST_CASE("containers", "[deque][binary]")
{
    test_range<BinarySerializer, BinaryDeserializer, std::deque>();
}

TEST_CASE("containers", "[set][json]")
{
    test_range<JsonSerializer, JsonDeserializer, std::set>();
}

TEST_CASE("containers", "[set][binary]")
{
    test_range<BinarySerializer, BinaryDeserializer, std::set>();
}

TEST_CASE("containers", "[multiset][json]")
{
    test_range<JsonSerializer, JsonDeserializer, std::multiset>();
}

TEST_CASE("containers", "[multiset][binary]")
{
    test_range<BinarySerializer, BinaryDeserializer, std::multiset>();
}

template <typename TOut, typename TIn, template <typename...> typename TRng>
void test_stack_queue()
{
    std::string name = "stack";
    using T = TRng<int>;
    T value_in, value_out;
    value_in.emplace(1);
    value_in.emplace(2);
    value_in.emplace(3);

    std::stringstream ss;
    OutArchive oar{TOut{ss}};
    oar(make_nvp(name, value_in));
    oar.Flush();

    InArchive iar{TIn{ss}};
    iar(make_nvp(name, value_out));

    REQUIRE(value_in == value_out);
}

TEST_CASE("containers", "[stack][json]")
{
    test_stack_queue<JsonSerializer, JsonDeserializer, std::stack>();
}

TEST_CASE("containers", "[stack][binary]")
{
    test_stack_queue<BinarySerializer, BinaryDeserializer, std::stack>();
}

TEST_CASE("containers", "[queue][json]")
{
    test_stack_queue<JsonSerializer, JsonDeserializer, std::queue>();
}

TEST_CASE("containers", "[queue][binary]")
{
    test_stack_queue<BinarySerializer, BinaryDeserializer, std::queue>();
}
