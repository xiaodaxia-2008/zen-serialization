#include <catch.hpp>
#include <zen_serialization/archive.h>
#include <queue>
#include <vector>
#include <functional>

using namespace zen;

template <typename TOut, typename TIn>
void test_priority_queue()
{
    std::priority_queue<int> pq;
    pq.push(10);
    pq.push(30);
    pq.push(20);
    pq.push(5);
    pq.push(40);

    // 保存优先队列的预期顺序（从高到低）
    std::vector<int> expected_order;
    auto temp_pq = pq; // 复制一份用于提取顺序
    while (!temp_pq.empty()) {
        expected_order.push_back(temp_pq.top());
        temp_pq.pop();
    }

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("priority_queue", pq));
    oar.Flush();

    std::priority_queue<int> pq_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("priority_queue", pq_out));

    // 验证还原的优先队列
    REQUIRE(pq_out.size() == pq.size());
    
    std::vector<int> restored_order;
    while (!pq_out.empty()) {
        restored_order.push_back(pq_out.top());
        pq_out.pop();
    }

    CHECK(expected_order == restored_order);
}

template <typename TOut, typename TIn>
void test_priority_queue_with_custom_comparator()
{
    // 使用greater<int>作为比较器，创建最小堆
    std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
    pq.push(10);
    pq.push(30);
    pq.push(20);
    pq.push(5);
    pq.push(40);

    // 保存优先队列的预期顺序（从低到高）
    std::vector<int> expected_order;
    auto temp_pq = pq; // 复制一份用于提取顺序
    while (!temp_pq.empty()) {
        expected_order.push_back(temp_pq.top());
        temp_pq.pop();
    }

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("priority_queue_min", pq));
    oar.Flush();

    std::priority_queue<int, std::vector<int>, std::greater<int>> pq_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("priority_queue_min", pq_out));

    // 验证还原的优先队列
    REQUIRE(pq_out.size() == pq.size());
    
    std::vector<int> restored_order;
    while (!pq_out.empty()) {
        restored_order.push_back(pq_out.top());
        pq_out.pop();
    }

    CHECK(expected_order == restored_order);
}

template <typename TOut, typename TIn>
void test_priority_queue_with_complex_type()
{
    // 测试带有自定义类型的优先队列
    std::priority_queue<std::pair<int, std::string>> pq;
    pq.push({10, "ten"});
    pq.push({30, "thirty"});
    pq.push({20, "twenty"});
    pq.push({5, "five"});
    pq.push({40, "forty"});

    // 保存优先队列的预期顺序
    std::vector<std::pair<int, std::string>> expected_order;
    auto temp_pq = pq; // 复制一份用于提取顺序
    while (!temp_pq.empty()) {
        expected_order.push_back(temp_pq.top());
        temp_pq.pop();
    }

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("priority_queue_pair", pq));
    oar.Flush();

    std::priority_queue<std::pair<int, std::string>> pq_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("priority_queue_pair", pq_out));

    // 验证还原的优先队列
    REQUIRE(pq_out.size() == pq.size());
    
    std::vector<std::pair<int, std::string>> restored_order;
    while (!pq_out.empty()) {
        restored_order.push_back(pq_out.top());
        pq_out.pop();
    }

    CHECK(expected_order == restored_order);
}

TEST_CASE("priority_queue", "[container]")
{
    test_priority_queue<JsonSerializer, JsonDeserializer>();
    test_priority_queue<BinarySerializer, BinaryDeserializer>();
    
    test_priority_queue_with_custom_comparator<JsonSerializer, JsonDeserializer>();
    test_priority_queue_with_custom_comparator<BinarySerializer, BinaryDeserializer>();
    
    test_priority_queue_with_complex_type<JsonSerializer, JsonDeserializer>();
    test_priority_queue_with_complex_type<BinarySerializer, BinaryDeserializer>();
}