#include <catch.hpp>
#include <zen_serialization/archive.h>

#include <filesystem>

using namespace zen;
namespace fs = std::filesystem;

template <typename TOut, typename TIn>
void test_filesystem_path(auto &&ini)
{
    fs::path value_in, value_out;
    value_in = ini;

    std::stringstream ss;
    OutArchive oar{TOut{ss}};
    oar(make_nvp("value", value_in));
    oar.Flush();

    InArchive iar{TIn{ss}};
    iar(make_nvp("value", value_out));

    REQUIRE(value_in == value_out);
}

TEST_CASE("FileSystemPath", "[FileSystemPath][json]")
{
    test_filesystem_path<JsonSerializer, JsonDeserializer>(fs::current_path());
    test_filesystem_path<JsonSerializer, JsonDeserializer>(
        fs::temp_directory_path());
}

TEST_CASE("FileSystemPath", "[FileSystemPath][binary]")
{
    test_filesystem_path<BinarySerializer, BinaryDeserializer>(
        fs::current_path());
    test_filesystem_path<BinarySerializer, BinaryDeserializer>(
        fs::temp_directory_path());
}
