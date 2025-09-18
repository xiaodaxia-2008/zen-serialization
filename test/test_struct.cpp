#include <catch.hpp>
#include <zen_serialization/archive.h>

using namespace zen;
using namespace Catch::Matchers;

struct Person {
    std::string name{""};
    int age{0};
    double weight{0};
    std::weak_ptr<Person> father;

    std::vector<std::shared_ptr<Person>> children;
    std::shared_ptr<Person> first_child;

    SERIALIZE_MEMBER(name, age, weight, father, first_child, children)
};

TEST_CASE("Struct")
{
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
    SPDLOG_DEBUG("Serialized: {}", ss.str());

    SPDLOG_DEBUG("{}", *john);

    auto john1 = std::move(john);
    {
        InArchive iar(ss);
        iar(NVP(john));
    }
    SPDLOG_DEBUG("{}", *john);
    CHECK(john);
    CHECK(john->age == john1->age);
    CHECK(john->name == john1->name);
    CHECK_THAT(john->weight, WithinAbs(john1->weight, 0.0001));
    CHECK(john->father.lock() == nullptr);
    CHECK(john->children.size() == 2);
    CHECK(john->children[0] == john->first_child);
    CHECK(john->children[0]->name == mike->name);
    CHECK(john->children[1]->name == nike->name);
}