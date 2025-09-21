#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <stacktrace>
#include <string>

#include <zen_serialization/archive.h>

struct Person {
    std::string name{""};
    int age{0};
    double weight{0};

    auto operator<=>(const Person &other) const = default;

    SERIALIZE_MEMBER(name, age, weight)
};

int main()
{
    using namespace zen;

    Person value_in{.name = "John", .age = 40, .weight = 80.8}, value_out;

    std::stringstream ss;
    OutArchive oar{JsonSerializer(ss)};
    oar(make_nvp("value", value_in));
    oar.Flush();

    SPDLOG_INFO("Serialized: {}", ss.str());

    InArchive iar{JsonDeserializer{ss}};
    iar(make_nvp("value", value_out));

    ZEN_ENSURE(value_in == value_out)
}