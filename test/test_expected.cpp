#include <zen_serialization/archive.h>

#include <catch.hpp>

#include <expected>
#include <sstream>
#include <string>
#include <vector>

using namespace zen;

template <typename TOut, typename TIn>
void test_expected(auto &&v)
{
    std::expected<std::vector<int>, std::string> value_in, value_out;
    value_in = v;

    std::stringstream ss;
    OutArchive oar{TOut{ss}};
    oar(make_nvp("value", value_in));
    oar.Flush();

    InArchive iar{TIn{ss}};
    iar(make_nvp("value", value_out));

    REQUIRE(value_out == value_in);
}

TEST_CASE("expected", "[expected][json]")
{
    test_expected<JsonSerializer, JsonDeserializer>(std::vector<int>{1, 2, 3});
    test_expected<JsonSerializer, JsonDeserializer>(
        std::unexpected<std::string>{"hello"});
}

TEST_CASE("expected", "[expected][binary]")
{
    test_expected<BinarySerializer, BinaryDeserializer>(
        std::vector<int>{1, 2, 3});
    test_expected<BinarySerializer, BinaryDeserializer>(
        std::unexpected<std::string>{"hello"});
}
