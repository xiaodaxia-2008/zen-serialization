#include <fmt/ostream.h>
#include <stacktrace>
#include <string>
#include <zen_serialization/archive.h>

struct Person {
    std::string name{""};
    int age{0};
    double weight{0};
    std::weak_ptr<Person> father;

    std::vector<std::shared_ptr<Person>> children;
    std::shared_ptr<Person> first_child;

    SERIALIZE_MEMBER(name, age, weight, father, first_child, children)
    // SERIALIZE_MEMBER(name, age);
};

template <>
struct fmt::formatter<Person> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

    auto format(const Person &p, format_context &ctx) const
    {
        return format_to(ctx.out(),
                         "Person: {{name: {}, age: {}, weight: {}, father: {}, "
                         "first_child: {}, children: {}}}",
                         p.name, p.age, p.weight,
                         p.father.lock() ? p.father.lock()->name : "null",
                         p.first_child ? p.first_child->name : "null",
                         p.children.size());
    }
};

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
    using namespace zen;
    auto john = std::shared_ptr<Person>(
        new Person{.name = "John", .age = 40, .weight = 80.8});

    auto mike = std::shared_ptr<Person>(
        new Person{.name = "Mike", .age = 22, .weight = 56.8, .father = john});
    auto nike = std::shared_ptr<Person>(
        new Person{.name = "Nike", .age = 20, .weight = 58.8, .father = john});

    john->children.push_back(mike);
    john->children.push_back(nike);
    john->first_child = mike;

    std::stringstream ss;
    {
        OutArchive oar(ss);
        oar(NVP(john));
    }
    SPDLOG_INFO("Serialized: {}", ss.str());

    SPDLOG_INFO("{}", *john);

    john.reset();
    {
        InArchive iar(ss);
        iar(NVP(john));
    }
    SPDLOG_INFO("{}", *john);
}