# zen-serlialization

A simple and easy to use serialization library with default json/binary support.

# Why not use existing libraries like cereal, boost serialization etc ?

The motivate for this library is to provide complete cross dll support while implementing the serialization function and polymorphic registration could still stay in cpp files instead of header files only. 

Assuming a scenegraph structure like the following:
```cpp
// BaseNode.h
class BASE_LIB_EXPORT BaseNode {
    int m_int;
    std::string m_string;
    std::vector<std::shared_ptr<BaseNode>> m_children;
    std::weak_ptr<BaseNode> m_parent;

    public:
    virtual ~BaseNode() = default;

    void save(OutArchive &oar) const;
    void load(InArchive &iar);
};
```

The serialization implementation could be done in cpp files like the following:

```cpp
// BaseNode.cpp
void BaseNode::save(OutArchive &oar) const
{
    ar(NVP(m_int), NVP(m_string), NVP(m_children), NVP(m_parent))
}
void BaseNode::load(InArchive &iar)
{
    ar(NVP(m_int), NVP(m_string), NVP(m_children), NVP(m_parent))
}

// need to register the node as it's polymorphic type
REGISTER_NODE(BaseNode)
```

The `BaseNode.cpp` is compiled into a library named `BASE_LIB`.

And there is a derived class like the following:
```cpp
// DerivedNode.h
class DERIVED_LIB_EXPORT DerivedNode : public BaseNode {
    int m_int2;
    public:
    void save(OutArchive &oar) const;
    void load(InArchive &iar);
}
```
```cpp
// DerivedNode.cpp
void DerivedNode::save(OutArchive &oar) const
{
    ar(make_nvp("base", BaseClass<BaseNode>(this)), NVP(m_int2));
}

void DerivedNode::load(InArchive &oar) 
{
    ar(make_nvp("base", BaseClass<BaseNode>(this)), NVP(m_int2));
}

// need to register the node as it's polymorphic type
REGISTER_NODE(DerivedNode)
```

The `DerivedNode.cpp` is compiled into a library named `DERIVED_LIB`.

Both the serialization implementation and polymorphic types registration could be done in cpp files which could be impossible with cereal or boost serialization.


# Features

- good readability
- support json/binary serialization
- stl containers support
- support for variant,tuple,pair, optional etc.
- raw pointer, smart pointer support
- inheritance and polymorphic support
- not restricted to implement the serialization function in header files, see [scene example](./example/scene/scene.cpp)

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

    ZEN_ENSURE(person_in == person_out)
}
```

the expected output is 
```json
{"john":{"age":40,"name":"John","weight":80.8}}
```


- advanced usage, [person example](./example/person.cpp) 

- more advanced usage of a scenegraph structure, [scene example](./example/scene/scene.cpp) 