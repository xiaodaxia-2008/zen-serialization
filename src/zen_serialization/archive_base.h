/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file archive_base.h
 * @author: Zen Shawn
 * @email: xiaozisheng2008@hotmail.com
 * @date: 21:25:15, September 17, 2025
 */
#pragma once
#include <spdlog/spdlog.h>

#include <zen_serialization_export.h>

#include <map>
#include <ranges>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <typeindex>
#include <variant>

#include <nlohmann/json.hpp>

#define PARENS ()
#define EXPAND(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...)                                                   \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...)                                        \
    macro(a1) __VA_OPT__(, FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define NVP(x) zen::make_nvp(#x, x)

#define SERIALIZE_MEMBER(...)                                                  \
    template <typename Archive>                                                \
    void serialize(Archive &ar)                                                \
    {                                                                          \
        ar(FOR_EACH(NVP, __VA_ARGS__));                                        \
    }

#define FOR_EACH_NVP(...) FOR_EACH(NVP, __VA_ARGS__)

#define REGISTER_CLASS(T)                                                      \
    namespace                                                                  \
    {                                                                          \
    static int force_register_class = [] {                                     \
        zen::ArchiveBase::RegisterClass<T>(#T);                                \
        return 1;                                                              \
    }();                                                                       \
    }

#define ZEN_THROW(message)                                                     \
    SPDLOG_CRITICAL(message);                                                  \
    throw std::runtime_error(message);

#define ZEN_EUNSURE(condition)                                                 \
    if (!(condition)) {                                                        \
        ZEN_THROW("assertion failed: " #condition);                            \
    }

namespace zen
{

class OutArchive;
class InArchive;

template <typename T>
struct NamedValuePair {
    std::string name;
    T value;
};

template <typename T>
NamedValuePair<T> &make_nvp(NamedValuePair<T> &pair)
{
    return pair;
}

template <typename T>
const NamedValuePair<T> &make_nvp(const NamedValuePair<T> &pair)
{
    return pair;
}

template <typename T>
NamedValuePair<T> make_nvp(T &&value)
{
    return NamedValuePair<T>{"", std::forward<T>(value)};
}

template <typename T>
NamedValuePair<T> make_nvp(std::string name, T &&value)
{
    return NamedValuePair<T>{std::move(name), std::forward<T>(value)};
}

class Access
{
public:
    template <typename T>
    static T *Create()
    {
        return new T;
    }
};

struct ZEN_SERIALIZATION_EXPORT ArchiveBase {
    template <typename T>
    static void RegisterClassName(const std::string &name)
    {
        RegisterClassName(std::type_index(typeid(T)), name);
    }

    static void RegisterClassName(const std::type_index &index,
                                  const std::string &name);

    template <typename T>
    static const std::string &GetClassName()
    {
        return GetClassName(std::type_index(typeid(T)));
    }

    static const std::string &GetClassName(const std::type_index &index);

    template <typename T>
    static void RegisterClass(const std::string &class_name)
    {
        RegisterClassName<T>(class_name);
        RegisterConstructor<T>();
        RegisterSerializer<T>();
        RegisterDeserializer<T>();
    }

    template <typename T>
    static void RegisterConstructor()
    {
        RegisterConstructor(GetClassName<T>(),
                            []() -> void * { return Access::Create<T>(); });
    }

    static void RegisterConstructor(const std::string &name,
                                    std::function<void *()> constructor);

    static const std::function<void *()> &
    GetConstructor(const std::string &name);

    template <typename T>
    static void RegisterSerializer()
    {
        const auto &type_name = GetClassName<T>();
        if constexpr (requires(T t) {
                          t.serialize(std::declval<OutArchive &>());
                      }) {
            RegisterSerializer(type_name, [](void *ptr, OutArchive &ar) {
                static_cast<T *>(ptr)->serialize(ar);
            });
        } else {
            static_assert(false, "type must have a serialize method");
        }
    }

    template <typename T>
    static void RegisterDeserializer()
    {
        const auto &type_name = GetClassName<T>();
        if constexpr (requires(T t) {
                          t.serialize(std::declval<InArchive &>());
                      }) {
            RegisterDeserializer(type_name, [](void *ptr, InArchive &ar) {
                static_cast<T *>(ptr)->serialize(ar);
            });
        } else {
            static_assert(false, "type must have a deserialize method");
        }
    }

    static void
    RegisterSerializer(const std::string &name,
                       std::function<void(void *, OutArchive &ar)> serializer);

    static void RegisterDeserializer(
        const std::string &name,
        std::function<void(void *, InArchive &ar)> deserializer);

    static const std::function<void(void *, OutArchive &ar)> &
    GetSerializer(const std::string &name);

    static const std::function<void(void *, InArchive &ar)> &
    GetDeserializer(const std::string &name);

    template <typename T>
    static T *Create(const std::string &name)
    {
        if (name.empty()) {
            ZEN_THROW("Class name is empty");
        }
        try {
            return static_cast<T *>(GetConstructor(name)());
        } catch (...) {
            if constexpr (std::is_default_constructible_v<T> &&
                          !std::is_polymorphic_v<T>) {
                return new T();
            }
        }

        ZEN_THROW(fmt::format("No constructor for {}", name));
    }
};

} // namespace zen