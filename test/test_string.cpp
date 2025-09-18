#include <zen_serialization/archive.h>

#include <catch.hpp>

using namespace zen;

template <typename TOut, typename TIn>
void test_str()
{
    std::string value = "hello 你好 こんにちは !";
    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("value", value));
    oar.Flush();

    std::string value1;
    InArchive iar{TIn(ss)};
    iar(make_nvp("value", value1));
    CHECK(value1 == value);
}

TEST_CASE("string", "[string]")
{
    test_str<JsonSerializer, JsonDeserializer>();
    test_str<BinarySerializer, BinaryDeserializer>();
}