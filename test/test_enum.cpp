#include <catch.hpp>
#include <zen_serialization/archive.h>

using namespace zen;

enum class Enum { A, B, C };

template <typename TOut, typename TIn>
void test_enum()
{
    const Enum value = Enum::B;

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("value", value));
    oar.Flush();

    Enum value1;
    InArchive iar{TIn(ss)};
    iar(make_nvp("value", value1));
    CHECK(value == value1);
}

TEST_CASE("enum", "[enum]")
{
    test_enum<JsonSerializer, JsonDeserializer>();
    test_enum<BinarySerializer, BinaryDeserializer>();
}
