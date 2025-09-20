# zen-serialization

A simple and easy to use serialization library with default json/binary support.

# Why not use existing libraries like cereal, boost serialization etc ?

The motivation for this library is to provide complete cross dll support while allowing serialization functions and polymorphic registration to stay in cpp files instead of header files only. 

Assuming a scenegraph structure like the following:
```cpp
// base_node.h
class BASE_LIB_EXPORT BaseNode {
    int m_int;
    std::string m_string;
    std::vector<std::shared_ptr<BaseNode>> m_children;
    std::weak_ptr<BaseNode> m_parent;

    public:
    virtual ~BaseNode() = default;

    void serialize(OutArchive &oar) const;
    void serialize(InArchive &iar);
};
```

The serialization implementation could be done in cpp files like the following:

```cpp
// base_node.cpp
void BaseNode::serialize(OutArchive &oar) const
{
    ar(NVP(m_int), NVP(m_string), NVP(m_children), NVP(m_parent));
}
void BaseNode::serialize(InArchive &iar)
{
    ar(NVP(m_int), NVP(m_string), NVP(m_children), NVP(m_parent));
}

// need to register the node as it's polymorphic type
REGISTER_NODE(BaseNode)
```

The `base_node.cpp` is compiled into a library named `base_lib`.

And there is a derived class like the following:
```cpp
// derived_node.h
class DERIVED_LIB_EXPORT DerivedNode : public BaseNode {
    int m_int2;
    public:
    void serialize(OutArchive &oar) const;
    void serialize(InArchive &iar);
};
```
```cpp
// derived_node.cpp
void DerivedNode::serialize(OutArchive &oar) const
{
    ar(make_nvp("base", BaseClass<BaseNode>(this)), NVP(m_int2));
}

void DerivedNode::serialize(InArchive &oar) 
{
    ar(make_nvp("base", BaseClass<BaseNode>(this)), NVP(m_int2));
}

// need to register the node as it's polymorphic type
REGISTER_NODE(DerivedNode)
```

The `derived_node.cpp` is compiled into a library named `derived_lib`.

Both the serialization implementation and polymorphic types registration could be done in cpp files which is impossible with cereal or boost serialization.


# Features

- Good readability
- Support for JSON/binary serialization
- STL containers support
- Support for variant, tuple, pair, optional, and expected etc
- Raw pointer and smart pointer support
- Inheritance and polymorphic support
- Serialization functions can be implemented in source files, not restricted to headers (see [scene example](./example/scene/main.cpp))

# Usage

- Basic usage: [simple person example](./example/simple.cpp)

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

    ZEN_ENSURE(person_in == person_out)
}
```

The expected output is:
```json
{"john":{"age":40,"name":"John","weight":80.8}}
```

- Advanced usage: [advanced person example](./example/advanced.cpp) 

- More advanced usage of a scenegraph structure: [scene example](./example/scene/scene.cpp) 
