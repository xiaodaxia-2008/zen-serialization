#include <catch.hpp>
#include <zen_serialization/archive.h>
#include <unordered_set>
#include <unordered_map>

using namespace zen;

template <typename TOut, typename TIn>
void test_unordered_set()
{
    std::unordered_set<int> set = {1, 2, 3, 4, 5};
    
    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("set", set));
    oar.Flush();

    std::unordered_set<int> set_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("set", set_out));

    REQUIRE(set_out.size() == set.size());
    CHECK(std::ranges::all_of(set, [&set_out](const auto& item) { 
        return set_out.contains(item); 
    }));
}

template <typename TOut, typename TIn>
void test_unordered_multiset()
{
    std::unordered_multiset<int> multiset = {1, 2, 2, 3, 3, 3};
    
    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("multiset", multiset));
    oar.Flush();

    std::unordered_multiset<int> multiset_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("multiset", multiset_out));

    REQUIRE(multiset_out.size() == multiset.size());
    // 检查每个元素的出现次数
    for (const auto& item : multiset) {
        CHECK(std::count(multiset_out.begin(), multiset_out.end(), item) 
              == std::count(multiset.begin(), multiset.end(), item));
    }
}

template <typename TOut, typename TIn>
void test_unordered_map()
{
    std::unordered_map<std::string, int> map = {
        {"one", 1},
        {"two", 2},
        {"three", 3}
    };
    
    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("map", map));
    oar.Flush();

    std::unordered_map<std::string, int> map_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("map", map_out));

    REQUIRE(map_out.size() == map.size());
    for (const auto& [key, value] : map) {
        CHECK(map_out.at(key) == value);
    }
}

template <typename TOut, typename TIn>
void test_unordered_multimap()
{
    std::unordered_multimap<std::string, int> multimap = {
        {"key", 1},
        {"key", 2},
        {"other_key", 3},
        {"key", 4}
    };
    
    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("multimap", multimap));
    oar.Flush();

    std::unordered_multimap<std::string, int> multimap_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("multimap", multimap_out));

    REQUIRE(multimap_out.size() == multimap.size());
    
    // 检查每个键值对是否都存在
    for (const auto& [key, value] : multimap) {
        auto range_original = multimap.equal_range(key);
        auto range_restored = multimap_out.equal_range(key);
        
        std::vector<int> original_values;
        std::vector<int> restored_values;
        
        for (auto it = range_original.first; it != range_original.second; ++it) {
            original_values.push_back(it->second);
        }
        
        for (auto it = range_restored.first; it != range_restored.second; ++it) {
            restored_values.push_back(it->second);
        }
        
        CHECK(original_values.size() == restored_values.size());
        // 排序后比较
        std::sort(original_values.begin(), original_values.end());
        std::sort(restored_values.begin(), restored_values.end());
        CHECK(original_values == restored_values);
    }
}

TEST_CASE("unordered_associative", "[unordered]")
{
    test_unordered_set<JsonSerializer, JsonDeserializer>();
    test_unordered_set<BinarySerializer, BinaryDeserializer>();
    
    test_unordered_multiset<JsonSerializer, JsonDeserializer>();
    test_unordered_multiset<BinarySerializer, BinaryDeserializer>();
    
    test_unordered_map<JsonSerializer, JsonDeserializer>();
    test_unordered_map<BinarySerializer, BinaryDeserializer>();
    
    test_unordered_multimap<JsonSerializer, JsonDeserializer>();
    test_unordered_multimap<BinarySerializer, BinaryDeserializer>();
}