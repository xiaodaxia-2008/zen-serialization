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

    Person person_in{.name = "John", .age = 40, .weight = 80.8};

    std::stringstream ss;
    OutArchive oar{JsonSerializer(ss)};
    oar(make_nvp("john", person_in));
    oar.Flush();

    SPDLOG_INFO("Serialized: {}", ss.str());

    Person person_out;
    InArchive iar{JsonDeserializer{ss}};
    iar(make_nvp("john", person_out));

    ZEN_ENSURE(person_in == person_out)
}