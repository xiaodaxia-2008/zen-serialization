#include <catch.hpp>
#include <zen_serialization/archive.h>

using namespace zen;

// 定义基类Animal
struct Animal {
    virtual ~Animal() = default;

    std::string name{""};
    int age{0};

    virtual std::string make_sound() const = 0;
    virtual std::string get_type() const = 0;

    SERIALIZE_MEMBER(name, age)
};

// 定义哺乳动物类
struct Mammal : public Animal {
    int num_legs{4};
    bool has_fur{true};

    std::string make_sound() const override
    {
        return "Some generic mammal sound";
    }

    std::string get_type() const override { return "Mammal"; }

    SERIALIZE_MEMBER(BaseClass<Animal>(this), num_legs, has_fur)
};

// 定义鸟类
struct Bird : public Animal {
    double wingspan{0.0};
    bool can_fly{true};

    std::string make_sound() const override { return "Chirp chirp"; }

    std::string get_type() const override { return "Bird"; }

    SERIALIZE_MEMBER(BaseClass<Animal>(this), wingspan, can_fly)
};

// 定义具体的哺乳动物：狗
struct Dog : public Mammal {
    std::string breed{""};
    bool is_pet{true};

    std::string make_sound() const override { return "Woof woof"; }

    std::string get_type() const override { return "Dog"; }

    SERIALIZE_MEMBER(BaseClass<Mammal>(this), breed, is_pet)
};

// 定义具体的鸟类：鹰
struct Eagle : public Bird {
    bool is_hunter{true};
    double hunting_range{0.0};

    std::string make_sound() const override { return "Screech"; }

    std::string get_type() const override { return "Eagle"; }

    SERIALIZE_MEMBER(BaseClass<Bird>(this), is_hunter, hunting_range)
};

// 动物园类，包含多种动物
struct Zoo {
    std::string name{""};
    std::vector<std::shared_ptr<Animal>> animals;

    SERIALIZE_MEMBER(name, animals)
};

// 注册所有类以便序列化
REGISTER_CLASS(Animal)
REGISTER_CLASS(Mammal)
REGISTER_CLASS(Bird)
REGISTER_CLASS(Dog)
REGISTER_CLASS(Eagle)
REGISTER_CLASS(Zoo)

template <typename TOut, typename TIn>
void test_inheritance()
{
    // 创建动物园
    auto zoo = std::make_shared<Zoo>();
    zoo->name = "City Zoo";

    // 创建狗
    auto dog = std::make_shared<Dog>();
    dog->name = "Buddy";
    dog->age = 3;
    dog->breed = "Golden Retriever";
    dog->is_pet = true;
    dog->num_legs = 4;
    dog->has_fur = true;

    // 创建鹰
    auto eagle = std::make_shared<Eagle>();
    eagle->name = "Freedom";
    eagle->age = 5;
    eagle->wingspan = 2.1;
    eagle->can_fly = true;
    eagle->is_hunter = true;
    eagle->hunting_range = 5.5;

    // 将动物添加到动物园
    zoo->animals.push_back(dog);
    zoo->animals.push_back(eagle);

    // 序列化
    std::stringstream ss;
    OutArchive oar{OutSerializer{TOut(ss)}};
    oar(make_nvp("zoo", zoo));
    oar.Flush();

    // 反序列化
    std::shared_ptr<Zoo> zoo_out;
    InArchive iar{InDeserializer{TIn(ss)}};
    iar(make_nvp("zoo", zoo_out));

    // 验证序列化/反序列化结果
    CHECK(zoo_out->name == zoo->name);
    CHECK(zoo_out->animals.size() == zoo->animals.size());

    // 验证狗的信息
    auto dog_out = std::static_pointer_cast<Dog>(zoo_out->animals[0]);
    CHECK(dog_out->name == dog->name);
    CHECK(dog_out->age == dog->age);
    CHECK(dog_out->breed == dog->breed);
    CHECK(dog_out->is_pet == dog->is_pet);
    CHECK(dog_out->num_legs == dog->num_legs);
    CHECK(dog_out->has_fur == dog->has_fur);

    // 验证鹰的信息
    auto eagle_out = std::static_pointer_cast<Eagle>(zoo_out->animals[1]);
    CHECK(eagle_out->name == eagle->name);
    CHECK(eagle_out->age == eagle->age);
    CHECK(eagle_out->wingspan == eagle->wingspan);
    CHECK(eagle_out->can_fly == eagle->can_fly);
    CHECK(eagle_out->is_hunter == eagle->is_hunter);
    CHECK(eagle_out->hunting_range == eagle->hunting_range);

    // 测试多态行为
    CHECK(dog_out->make_sound() == dog->make_sound());
    CHECK(eagle_out->make_sound() == eagle->make_sound());
    CHECK(dog_out->get_type() == dog->get_type());
    CHECK(eagle_out->get_type() == eagle->get_type());
}

TEST_CASE("inheritance", "[inheritance]")
{
    test_inheritance<JsonSerializer, JsonDeserializer>();
    test_inheritance<BinarySerializer, BinaryDeserializer>();
}