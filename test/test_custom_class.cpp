#include <catch.hpp>
#include <zen_serialization/archive.h>

using namespace zen;

// 定义一个自定义序列化的类
class CustomClass {
public:
    int id;
    std::string name;
    std::vector<double> values;

    CustomClass() = default;
    CustomClass(int i, std::string n, std::vector<double> v)
        : id(i), name(std::move(n)), values(std::move(v)) {}

    template<typename Archive>
    void serialize(Archive& ar) {
        ar(make_nvp("id", id));
        ar(make_nvp("name", name));
        ar(make_nvp("values", values));
    }

    bool operator==(const CustomClass& other) const {
        return id == other.id && name == other.name && values == other.values;
    }
};

// 定义另一个使用SERIALIZE_MEMBER宏的类
struct Product {
    std::string code;
    double price;
    int quantity;
    
    SERIALIZE_MEMBER(code, price, quantity)
    
    bool operator==(const Product& other) const {
        return code == other.code && price == other.price && quantity == other.quantity;
    }
};

template <typename TOut, typename TIn>
void test_custom_class()
{
    CustomClass obj(42, "Test Object", {1.1, 2.2, 3.3});
    
    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("custom_obj", obj));
    oar.Flush();

    CustomClass obj_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("custom_obj", obj_out));

    CHECK(obj_out == obj);
}

template <typename TOut, typename TIn>
void test_product_class()
{
    Product product{"ABC123", 99.99, 5};
    
    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("product", product));
    oar.Flush();

    Product product_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("product", product_out));

    CHECK(product_out == product);
}

TEST_CASE("custom_class", "[custom]")
{
    test_custom_class<JsonSerializer, JsonDeserializer>();
    test_custom_class<BinarySerializer, BinaryDeserializer>();
    test_product_class<JsonSerializer, JsonDeserializer>();
    test_product_class<BinarySerializer, BinaryDeserializer>();
}