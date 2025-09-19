#include <catch.hpp>
#include <zen_serialization/archive.h>

using namespace zen;

struct Node {
    int value;
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> loop;
    std::unique_ptr<int> data;

    SERIALIZE_MEMBER(value, next, loop, data)
};

template <typename TOut, typename TIn>
void test_smart_pointers()
{
    // 创建一个简单的链表结构
    auto node1 = std::make_shared<Node>();
    node1->value = 1;
    node1->data = std::make_unique<int>(42);

    auto node2 = std::make_shared<Node>();
    node2->value = 2;
    node2->data = std::make_unique<int>(84);

    node1->next = node2;
    node2->loop = node1; // 循环引用

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("node1", node1));
    oar.Flush();

    // 反序列化
    std::shared_ptr<Node> node1_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("node1", node1_out));

    // 验证结果
    REQUIRE(node1_out != nullptr);
    CHECK(node1_out->value == 1);
    REQUIRE(node1_out->data != nullptr);
    CHECK(*(node1_out->data) == 42);

    REQUIRE(node1_out->next != nullptr);
    CHECK(node1_out->next->value == 2);
    REQUIRE(node1_out->next->data != nullptr);
    CHECK(*(node1_out->next->data) == 84);

    // 检查循环引用
    auto locked_loop = node1_out->next->loop.lock();
    REQUIRE(locked_loop != nullptr);
    CHECK(locked_loop->value == node1_out->value);
    CHECK(locked_loop == node1_out); // 应该指向同一个对象
}

TEST_CASE("smart_pointers", "[smart_ptr]")
{
    test_smart_pointers<JsonSerializer, JsonDeserializer>();
    test_smart_pointers<BinarySerializer, BinaryDeserializer>();
}