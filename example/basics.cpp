#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <stacktrace>
#include <string>

#define ZEN_SERIALIZATION_OUT_SERIALIZER zen::BinarySerializer
#define ZEN_SERIALIZATION_IN_DESERIALIZER zen::BinaryDeserializer
#include <zen_serialization/archive.h>

struct Person {
    std::string name{""};
    int age{0};
    double weight{0};

    SERIALIZE_MEMBER(name, age, weight)
};

template <>
struct fmt::formatter<Person> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

    auto format(const Person &p, format_context &ctx) const
    {
        return format_to(ctx.out(), "Person: {{name: {}, age: {}, weight: {}}}",
                         p.name, p.age, p.weight);
    }
};

using namespace zen;

template <typename TOut, typename TIn>
void test_str()
{
    std::string name = "JikeMa";
    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(NVP(name));
    oar.Flush();
    auto str = ss.str();
    std::vector<char> data{str.begin(), str.end()};
    SPDLOG_INFO("Serialized: {}", data);
    SPDLOG_INFO("{}", name);

    name.clear();

    InArchive iar{TIn(ss)};
    iar(NVP(name));
    SPDLOG_INFO("{}", name);
}

template <typename TOut, typename TIn>
void test()
{
    auto john = std::shared_ptr<Person>(
        new Person{.name = "John", .age = 40, .weight = 80.8});

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(NVP(john));
    oar.Flush();

    auto str = ss.str();
    std::vector<char> data{str.begin(), str.end()};
    SPDLOG_INFO("Serialized: {}", data);

    SPDLOG_INFO("{}", *john);

    john.reset();
    InArchive iar{TIn(ss)};
    iar(NVP(john));
    SPDLOG_INFO("{}", *john);
}

int main()
{
    std::set_terminate([] {
        auto curexp = std::current_exception();
        if (curexp) {
            try {
                spdlog::error("{}", fmt::streamed(std::stacktrace::current()));
                std::rethrow_exception(curexp);
            } catch (std::exception &e) {
                spdlog::error("{}", e.what());
            }
        }
    });

    // test<JsonSerializer, JsonDeserializer>();
    test<BinarySerializer, BinaryDeserializer>();
    // test_str<JsonSerializer, JsonDeserializer>();
    test_str<BinarySerializer, BinaryDeserializer>();
}