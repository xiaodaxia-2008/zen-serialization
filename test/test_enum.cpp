#include <catch.hpp>
#include <zen_serialization/archive.h>

using namespace zen;

enum class Alphabet { A, B, C };


TEST_CASE("enum", "[enum]")
{
    Alphabet a = Alphabet::B;

    std::stringstream ss;

    {
        OutArchive oar(ss);
        oar(make_nvp("a", a));
    }

    SPDLOG_INFO("serialized: {}", ss.str());

    Alphabet a1;
    {
        InArchive iar(ss);
        iar(make_nvp("a", a1));
    }

    CHECK(a == a1);
}