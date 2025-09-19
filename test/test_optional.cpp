#include <catch.hpp>
#include <zen_serialization/archive.h>
#include <optional>

using namespace zen;

template <typename TOut, typename TIn>
void test_optional()
{
    std::optional<int> opt_value = 42;
    std::optional<int> opt_empty;

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("opt_value", opt_value));
    oar(make_nvp("opt_empty", opt_empty));
    oar.Flush();

    std::optional<int> opt_value_out;
    std::optional<int> opt_empty_out;

    InArchive iar{TIn(ss)};
    iar(make_nvp("opt_value", opt_value_out));
    iar(make_nvp("opt_empty", opt_empty_out));

    REQUIRE(opt_value_out.has_value());
    CHECK(opt_value_out.value() == 42);
    CHECK_FALSE(opt_empty_out.has_value());
}

template <typename TOut, typename TIn>
void test_optional_complex()
{
    std::optional<std::vector<int>> opt_vec = std::vector<int>{1, 2, 3, 4, 5};
    std::optional<std::vector<int>> opt_vec_empty;

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("opt_vec", opt_vec));
    oar(make_nvp("opt_vec_empty", opt_vec_empty));
    oar.Flush();

    std::optional<std::vector<int>> opt_vec_out;
    std::optional<std::vector<int>> opt_vec_empty_out;

    InArchive iar{TIn(ss)};
    iar(make_nvp("opt_vec", opt_vec_out));
    iar(make_nvp("opt_vec_empty", opt_vec_empty_out));

    REQUIRE(opt_vec_out.has_value());
    CHECK(opt_vec_out.value() == std::vector<int>{1, 2, 3, 4, 5});
    CHECK_FALSE(opt_vec_empty_out.has_value());
}

TEST_CASE("optional", "[optional]")
{
    test_optional<JsonSerializer, JsonDeserializer>();
    test_optional<BinarySerializer, BinaryDeserializer>();
    test_optional_complex<JsonSerializer, JsonDeserializer>();
    test_optional_complex<BinarySerializer, BinaryDeserializer>();
}