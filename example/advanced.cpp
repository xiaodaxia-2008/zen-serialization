#include <fmt/ostream.h>
#include <stacktrace>
#include <string>
#include <zen_serialization/archive.h>

using namespace zen;

enum class Gender { Male, Female };

struct Person {
    virtual ~Person() = default;

    std::string name{""};
    int age{0};
    double weight{0};
    Gender gender{Gender::Male};

    SERIALIZE_MEMBER(name, age, weight, gender)
};

struct Child : public Person {
    std::weak_ptr<Person> father;

    SERIALIZE_MEMBER(BaseClass<Person>(this), father)
};

struct Father : public Person {
    std::vector<std::shared_ptr<Person>> children;

    SERIALIZE_MEMBER(BaseClass<Person>(this), children)
};

REGISTER_CLASS(Person)
REGISTER_CLASS(Child)
REGISTER_CLASS(Father)

template <>
struct fmt::formatter<Person> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

    auto format(const Person &p, format_context &ctx) const
    {
        return format_to(ctx.out(),
                         "Person name: {}, age: {}, weight: {}, gender: {}",
                         p.name, p.age, p.weight, std::to_underlying(p.gender));
    }
};

int main()
{
    using TOut = JsonSerializer;
    using TIn = JsonDeserializer;

    auto father = std::make_shared<Father>();
    father->name = "John";
    father->age = 50;
    father->weight = 80.5;
    father->gender = Gender::Male;

    auto child1 = std::make_shared<Child>();
    child1->name = "Mike";
    child1->age = 18;
    child1->weight = 50.5;
    child1->gender = Gender::Male;
    child1->father = father;

    father->children.push_back(child1);

    std::stringstream ss;
    OutArchive oar{OutSerializer{TOut(ss, 2)}};
    oar(make_nvp("John", father));
    oar.Flush();
    SPDLOG_INFO("Serialized: {}", ss.str());

    std::shared_ptr<Person> person_out;
    InArchive iar{InDeserializer{TIn(ss)}};
    iar(make_nvp("John", person_out));

    ZEN_ENSURE(person_out->name == father->name);
    ZEN_ENSURE(person_out->age == father->age);
    ZEN_ENSURE(person_out->weight == father->weight);

    auto father_out = std::static_pointer_cast<Father>(person_out);
    ZEN_ENSURE(father_out->children.size() == father->children.size());
    auto child1_out = std::static_pointer_cast<Child>(father_out->children[0]);
    ZEN_ENSURE(child1_out->name == child1->name);
    ZEN_ENSURE(child1_out->age == child1->age);
    ZEN_ENSURE(child1_out->weight == child1->weight);
    ZEN_ENSURE(child1_out->gender == child1->gender);
    ZEN_ENSURE(child1_out->father.lock() == father_out);
}
