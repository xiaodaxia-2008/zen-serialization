/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file serializer.h
 * @author: Zen Shawn
 * @email: xiaozisheng2008@hotmail.com
 * @date: 18:50:48, September 18, 2025
 */
#pragma once
#include "range_size.h"
#include <zen_serialization_export.h>

#include <iostream>
#include <span>
#include <string_view>
#include <vector>

namespace zen
{

class JsonSerializer
{
    std::ostream &m_stream;
    std::vector<nlohmann::json> m_objects;
    std::vector<std::string> m_next_names;
    std::size_t m_idx = 0;
    int m_indentation{-1};

    const nlohmann::json &Json() const
    {
        assert(m_objects.size() == 1);
        return m_objects.back();
    }

    std::string NextName()
    {
        if (m_next_names.empty()) {
            return std::format("value{}", m_idx++);
        } else {
            auto name = std::move(m_next_names.back());
            m_next_names.pop_back();
            if (name.empty()) {
                return std::format("value{}", m_idx++);
            } else {
                return std::move(name);
            }
        }
    }

public:
    JsonSerializer(std::ostream &stream, int indentation = -1)
        : m_stream(stream), m_indentation(indentation)
    {
        m_objects.emplace_back(nlohmann::json::object());
    }

    static constexpr bool IsBinary() { return false; }

    void Flush() { m_stream << Json().dump(m_indentation); }

    void SetNextName(std::string_view name) { m_next_names.emplace_back(name); }

    void NewObject() { m_objects.emplace_back(nlohmann::json::object()); }

    void FinishObject()
    {
        auto current = std::move(m_objects.back());
        m_objects.pop_back();
        auto &parent = m_objects.back();
        if (parent.is_array()) {
            parent.push_back(std::move(current));
        } else if (parent.is_object()) {
            const auto &key = NextName();
            parent[key] = std::move(current);
        } else {
            ZEN_THROW(fmt::format("parent is not an array or object {}",
                                  parent.dump()));
        }
    }

    void NewArray() { m_objects.emplace_back(nlohmann::json::array()); }

    void FinishArray() { FinishObject(); }

    void operator()([[maybe_unused]] const RangeSize &size)
    {
        // it's not necessary to serialize the size with json format
    }

    void operator()(std::span<const char> bytes)
    {
        (*this)(base64_encode(bytes));
    }

    template <typename T>
        requires std::is_arithmetic_v<T> || std::is_same_v<T, std::string>
    void operator()(const T &t)
    {
        auto &current = m_objects.back();
        if (current.is_object()) {
            current[NextName()] = t;
        } else if (current.is_array()) {
            current.push_back(t);
        } else {
            ZEN_THROW(fmt::format("Invalid json type {}", typeid(t).name()));
        }
    }
};

class JsonDeserializer
{
    using json = nlohmann::json;

    json m_json;
    std::vector<std::reference_wrapper<json>> m_objects;
    std::size_t m_idx = 0;

    std::vector<std::string> m_next_names;

    std::vector<std::size_t> m_arr_idxes;

    std::string NextName()
    {
        if (m_next_names.empty()) {
            return std::format("value{}", m_idx++);
        } else {
            std::string name = std::move(m_next_names.back());
            m_next_names.pop_back();
            return std::move(name);
        }
    }

public:
    JsonDeserializer(std::istream &stream)
    {
        stream >> m_json;
        m_objects.push_back(m_json);
    }

    JsonDeserializer(JsonDeserializer &&deser)
    {
        ZEN_ENSURE_WITH_MSG(
            deser.m_objects.empty() ||
                (deser.m_objects.size() == 1 &&
                 &deser.m_objects.front().get() == &deser.m_json),
            "Only untouched deserializers can be moved")
        m_json = std::move(deser.m_json);
        m_objects.push_back(m_json);
    }

    static constexpr bool IsBinary() { return false; }

    void Flush() {}

    void SetNextName(std::string_view name) { m_next_names.emplace_back(name); }

    void NewObject()
    {
        json &current = m_objects.back();
        if (current.is_object()) {
            m_objects.emplace_back(current[NextName()]);
        } else if (current.is_array()) {
            m_objects.emplace_back(current[m_arr_idxes.back()++]);
        } else {
            ZEN_THROW(fmt::format("cannot obtain new object, current is {}",
                                  current.type_name()));
        }
    }

    void FinishObject()
    {
        json &current = m_objects.back();
        if (current.is_array()) {
            m_arr_idxes.pop_back();
        }
        m_objects.pop_back();
    }

    void NewArray()
    {
        json &current = m_objects.back();
        if (current.is_object()) {
            m_objects.emplace_back(current[NextName()]);
            m_arr_idxes.emplace_back(0);
        } else if (current.is_array()) {
            m_objects.emplace_back(current[m_arr_idxes.back()++]);
            m_arr_idxes.emplace_back(0);
        } else {
            ZEN_THROW("cannot obtain new array");
        }
    }
    void FinishArray() { FinishObject(); }

    void operator()(RangeSize &size)
    {
        json &current = m_objects.back();
        if (current.is_array()) {
            size.size = current.size();
        } else {
            ZEN_THROW("Current object is not an array");
        }
    }

    void operator()(std::span<char> bytes)
    {
        std::string encoded;
        (*this)(encoded);
        auto decoded = base64_decode(encoded);
        std::copy_n(decoded.begin(), bytes.size(), bytes.begin());
    }

    template <typename T>
        requires std::is_arithmetic_v<T> || std::is_same_v<T, std::string>
    void operator()(T &t)
    {
        json &current = m_objects.back();
        if (current.is_object()) {
            current[NextName()].get_to(t);
        } else if (current.is_array()) {
            current[m_arr_idxes.back()++].get_to(t);
        } else {
            ZEN_THROW(fmt::format("Invalid json type {}", current.type_name()));
        }
    }
};

} // namespace zen