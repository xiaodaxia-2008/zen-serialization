#pragma once
#include <zenser_dummy_lib_base_export.h>

#include <zen_serialization/archive.h>

#include <array>
#include <bitset>
#include <deque>
#include <forward_list>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>

using namespace zen;

class ZENSER_DUMMY_LIB_BASE_EXPORT BaseNode
    : public std::enable_shared_from_this<BaseNode>
{
public:
    BaseNode(std::string name) : m_name(std::move(name))
    {
        m_id = std::make_unique<int>(rand());
        m_number = new double(rand());
        m_number1 = m_number;
        m_variant = "variant";
        m_array = {1.0, 2.0};

        m_stack.push(1.0);
        m_stack.push(2.0);
        m_list = {1.0, 2.0};
        m_bitset = 0b111011101;

        m_vector_list = {
            {1, 3},
            {1, 2, 3},
            {1, 3, 4},
        };

        m_deque = {1.0, 2.0};
        m_queue.push_range(m_deque);

        m_forward_list = {1.0, 2.0};

        m_map = {
            {"one", 1.0},
            {"two", 2.0},
        };
        m_unordered_map = {
            {"one", 1.0},
            {"two", 2.0},
        };
        m_multimap = {
            {"one", 1.0},
            {"two", 2.0},
            {"one", 3.0},
            {"two", 4.0},
        };
        m_unordered_multimap = {
            {"one", 1.0},
            {"two", 2.0},
            {"one", 3.0},
            {"two", 4.0},
        };

        m_set = {"1.0", "2.0"};
        m_multiset = {"1.0", "2.0", "1.0", "2.0"};
        m_unordered_set = {"1.0", "2.0"};
        m_unordered_multiset = {"1.0", "2.0", "1.0", "2.0"};

        m_unordered_set = {"1.0", "2.0"};
        m_pair = {1.0, 2};
        m_tuple = std::make_tuple(1.0, 2, "three");
    }

    virtual ~BaseNode();

    std::shared_ptr<BaseNode> GetParent() const { return m_parent.lock(); }

    void SetName(std::string name) { m_name = std::move(name); }

    std::string_view GetName() const { return m_name; }

    void AddChild(std::shared_ptr<BaseNode> child)
    {
        if (std::ranges::find(m_children, child) == m_children.end()) {
            child->m_parent = shared_from_this();
            m_children.push_back(std::move(child));
        }
    }

    void RemoveChild(const std::shared_ptr<BaseNode> &child)
    {
        auto it = std::ranges::find(m_children, child);
        if (it != m_children.end()) {
            m_children.erase(it);
            child->m_parent.reset();
        }
    }

    std::shared_ptr<BaseNode> GetChild(std::size_t index)
    {
        return m_children.at(index);
    }

    std::size_t GetChildrenCount() const { return m_children.size(); }

    template <typename Archive>
    void serialize(Archive &ar)
    {
        ar(NVP(m_name));
        ar(NVP(m_parent));
        ar(NVP(m_children));
        ar(NVP(m_id));
        ar(NVP(m_number));
        ar(NVP(m_number1));
        ar(NVP(m_number2));

        ar(NVP(m_vector_list));

        ar(NVP(m_array));
        ar(NVP(m_variant));
        ar(NVP(m_list));
        ar(NVP(m_forward_list));

        ar(NVP(m_stack));
        ar(NVP(m_deque));
        ar(NVP(m_queue));

        ar(NVP(m_pair));

        ar(NVP(m_map));
        ar(NVP(m_unordered_map));
        ar(NVP(m_multimap));
        ar(NVP(m_unordered_multimap));

        ar(NVP(m_set));
        ar(NVP(m_unordered_set));
        ar(NVP(m_multiset));
        ar(NVP(m_unordered_multiset));

        ar(NVP(m_tuple));
        ar(NVP(m_bitset));
    }

    virtual void format(fmt::format_context &ctx) const
    {
        fmt::format_to(ctx.out(), "{}", GetName());
        fmt::format_to(ctx.out(), ", children: [");
        for (auto child : m_children) {
            fmt::format_to(ctx.out(), "{}, ", child->m_name);
        }
        fmt::format_to(ctx.out(), "]");
        if (auto parent = GetParent()) {
            fmt::format_to(ctx.out(), ", parent: {}", parent->GetName());
        }
        if (m_id) {
            fmt::format_to(ctx.out(), ", id: {}", *m_id);
        }
        if (m_number) {
            fmt::format_to(ctx.out(), ", number: {}", *m_number);
        }
        if (m_number1) {
            fmt::format_to(ctx.out(), ", number1: {}", *m_number1);
        }
        if (m_number2) {
            fmt::format_to(ctx.out(), ", number2: {}", *m_number2);
        }
        fmt::format_to(ctx.out(), ", variant: {}", m_variant);
        fmt::format_to(ctx.out(), ", vector list: {}", m_vector_list);
        fmt::format_to(ctx.out(), ", array: {}", m_array);
        fmt::format_to(ctx.out(), ", list: {}", m_list);
        fmt::format_to(ctx.out(), ", deque: {}", m_deque);
        fmt::format_to(ctx.out(), ", queue: {}", m_queue);
        fmt::format_to(ctx.out(), ", forward_list: {}", m_forward_list);
        fmt::format_to(ctx.out(), ", stack: {}", m_stack);

        fmt::format_to(ctx.out(), ", map: {}", m_map);
        fmt::format_to(ctx.out(), ", multimap: {}", m_multimap);
        fmt::format_to(ctx.out(), ", unorderd map: {}", m_unordered_map);
        fmt::format_to(ctx.out(), ", unorderd multimap: {}",
                       m_unordered_multimap);

        fmt::format_to(ctx.out(), ", set: {}", m_set);
        fmt::format_to(ctx.out(), ", multiset: {}", m_multiset);
        fmt::format_to(ctx.out(), ", unorderd set: {}", m_unordered_set);
        fmt::format_to(ctx.out(), ", unorderd multiset: {}",
                       m_unordered_multiset);

        fmt::format_to(ctx.out(), ", bit set: {}", m_bitset);
        fmt::format_to(ctx.out(), ", pair: {}", m_pair);
        fmt::format_to(ctx.out(), ", tuple: {}", m_tuple);
    }

protected:
    BaseNode() = default;
    friend class zen::Access;

    std::string m_name{"Node"};

    std::unique_ptr<int> m_id;

    double *m_number{nullptr};
    double *m_number1{nullptr};
    double *m_number2{nullptr};

    std::vector<std::shared_ptr<BaseNode>> m_children;

    std::array<double, 2> m_array;

    std::list<double> m_list;
    std::forward_list<double> m_forward_list;

    std::vector<std::list<int>> m_vector_list;

    std::stack<double> m_stack;
    std::queue<double> m_queue;
    std::deque<double> m_deque;

    std::map<std::string, double> m_map;
    std::unordered_map<std::string, double> m_unordered_map;
    std::multimap<std::string, double> m_multimap;
    std::unordered_multimap<std::string, double> m_unordered_multimap;

    std::set<std::string> m_set;
    std::unordered_set<std::string> m_unordered_set;
    std::multiset<std::string> m_multiset;
    std::unordered_multiset<std::string> m_unordered_multiset;

    std::bitset<8> m_bitset;
    std::pair<double, int> m_pair;
    std::tuple<double, int, std::string> m_tuple;

    std::weak_ptr<BaseNode> m_parent;

    std::variant<float, double, std::string> m_variant;
};

template <std::derived_from<BaseNode> T>
struct fmt::formatter<T> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

    auto format(const BaseNode &b, format_context &ctx) const
    {
        b.format(ctx);
        return ctx.out();
    }
};