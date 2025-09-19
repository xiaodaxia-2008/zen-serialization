#include <zen_serialization/archive.h>

#include <catch.hpp>
#include <fmt/ranges.h>

using namespace zen;

template <typename TOut, typename TIn>
void test_vec_map()
{
    using T = std::vector<std::map<int, std::string>>;
    T value_in = {
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

    SPDLOG_INFO("{}", ss.str());

    InArchive iar{TIn{ss}};
    T value_out;
    iar(make_nvp("value", value_out));
}

int main() { test_vec_map<JsonSerializer, JsonDeserializer>(); }