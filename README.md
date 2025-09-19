# zen-serlialization

A simple and easy to use serialization library with default json/binary support.

# Features

- good readability
- support json/binary serialization
- stl containers support
- support for variant,tuple,pair, etc.
- raw pointer, smart pointer support
- inheritance support

# Usage

- basic usage,  [simple example](./example/simple.cpp)

```cpp
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

    ZEN_EUNSURE(person_in == person_out)
}
```

the expected output is 
```json
{"john":{"age":40,"name":"John","weight":80.8}}
```


- advanced usage, [person example](./example/person.cpp) 

- more advanced usage of a scenegraph structure, [scene example](./example/scene/scene.cpp) 