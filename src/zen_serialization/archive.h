/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file out_archive.h
 * @author: Zen Shawn
 * @email: xiaozisheng2008@hotmail.com
 * @date: 21:24:50, September 17, 2025
 */
#pragma once
#include "archive_base.h"
#include "base64.h"
#include "binary_serializer.h"
#include "json_serializer.h"
#include "serializer.h"

namespace zen
{
class Scope
{
    std::function<void()> f;

public:
    Scope(std::function<void()> f) : f(std::move(f)) {}

    ~Scope()
    {
        if (f) {
            f();
        }
    }
};

template <bool IsArray, typename TSerializer>
struct NewObjectScope {
    TSerializer &serializer;
    Scope scope;

    NewObjectScope(TSerializer &ser)
        : serializer(ser), scope([&] {
              if constexpr (IsArray) {
                  serializer.FinishObject();
              } else {
                  serializer.FinishArray();
              }
          })
    {
        if constexpr (IsArray) {
            serializer.NewArray();
        } else {
            serializer.NewObject();
        }
    }
};

class OutArchive : public ArchiveBase
{
    std::set<std::uintptr_t> m_pointers;

    OutSerializer m_serializer;

public:
    using TSerializer = OutSerializer;

    static constexpr bool is_input = false;

    OutArchive(OutSerializer serializer) : m_serializer(std::move(serializer))
    {
    }

    void Flush() { m_serializer.Flush(); }

    void operator()(auto &&item1, auto &&...items)
    {
        trySerialize(make_nvp(item1));
        if constexpr (sizeof...(items) > 0) {
            (*this)(items...);
        }
    }

private:
    template <typename T>
    void trySerialize(const NamedValuePair<T> &item)
    {
        m_serializer.SetNextName(item.name);
        trySerialize(item.value);
    }

    template <typename T>
    void trySerialize(const T &item)
    {
        constexpr bool is_range = std::ranges::range<T>;
        if constexpr (requires(const T item) { item.save(*this); }) {
            NewObjectScope<is_range, TSerializer> scope(m_serializer);
            item.save(*this);
        } else if constexpr (requires(const T item) { save(item, *this); }) {
            NewObjectScope<is_range, TSerializer> scope(m_serializer);
            save(item, *this);
        } else if constexpr (requires(T item) { item.serialize(*this); }) {
            NewObjectScope<is_range, TSerializer> scope(m_serializer);
            const_cast<T &>(item).serialize(*this);
        } else if constexpr (requires(T item) { serialize(item, *this); }) {
            NewObjectScope<is_range, TSerializer> scope(m_serializer);
            serialize(const_cast<T &>(item), *this);
        } else if constexpr (requires(T item) { serialize(item); }) {
            serialize(item);
        } else {
            static_assert(false, "Unsupported type");
        }
    }

    template <typename T>
        requires std::is_enum_v<T>
    void serialize(const T &item)
    {
        serialize(static_cast<std::underlying_type_t<T>>(item));
    }

    template <typename T>
        requires std::is_arithmetic_v<T>
    void serialize(const T &item)
    {
        m_serializer(item);
    }

    void serialize(const RangeSize &item) { m_serializer(item.size); }

    void serialize(const std::string &item) { m_serializer(item); }

    template <std::ranges::range Rng>
    void serialize(const Rng &items)
    {
        using T = std::ranges::range_value_t<Rng>;
        constexpr bool save_binary =
            std::ranges::contiguous_range<Rng> && std::is_arithmetic_v<T>;

        std::size_t n;
        if constexpr (std::ranges::sized_range<Rng>) {
            n = std::ranges::size(items);
        } else {
            n = std::distance(std::ranges::begin(items),
                              std::ranges::end(items));
        }

        m_serializer(RangeSize(n));
        if constexpr (save_binary) {
            if (m_serializer.IsBinary()) {
                auto ptr = reinterpret_cast<const char *>(items.data());
                std::span<const char> bytes(ptr, n * sizeof(T));
                m_serializer(bytes);
                return;
            }
        }

        NewObjectScope<true, TSerializer> scope(m_serializer);
        for (const auto &i : items) {
            trySerialize(i);
        }
    }

    template <typename T>
    void serialize(const std::unique_ptr<T> &item)
    {
        serialize(item.get());
    }

    template <typename T>
    void serialize(const std::weak_ptr<T> &item)
    {
        serialize(item.lock());
    }

    template <typename T>
    void serialize(const std::shared_ptr<T> &item)
    {
        serialize(item.get());
    }

    template <typename T>
        requires std::is_pointer_v<T>
    void serialize(const T &item)
    {
        auto id = reinterpret_cast<std::uintptr_t>(item);
        NewObjectScope<false, TSerializer> scope(m_serializer);
        trySerialize(NVP(id));
        if (id == 0) {
            return;
        }

        if (m_pointers.contains(id)) {
            return;
        }
        constexpr bool is_polymorphic =
            std::is_polymorphic_v<std::remove_pointer_t<T>>;

        m_pointers.insert(id);
        if constexpr (!is_polymorphic) {
            trySerialize(make_nvp("data", *item));
            return;
        } else {
            const auto &type_name = GetClassName(typeid(*item));
            trySerialize(make_nvp("type_name", type_name));
            const auto &serializer = GetSerializer(type_name);
            m_serializer.SetNextName("data");
            NewObjectScope<false, TSerializer> scope2(m_serializer);
            serializer(item, *this);
        }
    }
};

class InArchive : public ArchiveBase
{
    std::map<void *, std::shared_ptr<void>> m_shared_pointers;
    std::map<std::uintptr_t, void *> m_raw_pointers;

    InDeserializer m_serializer;

public:
    using TSerializer = InDeserializer;

    constexpr static bool is_input = true;

    InArchive(InDeserializer deserializer)
        : m_serializer(std::move(deserializer))
    {
    }

    void operator()(auto &&item1, auto &&...items)
    {
        trySerialize(make_nvp(item1));
        if constexpr (sizeof...(items) > 0) {
            (*this)(items...);
        }
    }

private:
    template <typename T>
        requires std::is_reference_v<T>
    void trySerialize(NamedValuePair<T> &&item)
    {
        m_serializer.SetNextName(item.name);
        trySerialize(item.value);
    }

    template <typename T>
        requires std::is_reference_v<T>
    void trySerialize(NamedValuePair<T> &item)
    {
        m_serializer.SetNextName(item.name);
        trySerialize(item.value);
    }

    template <typename T>
    void trySerialize(T &item)
    {
        constexpr bool is_range = std::ranges::range<T>;
        if constexpr (requires(T item) { item.load(*this); }) {
            NewObjectScope<is_range, TSerializer> scope(m_serializer);
            item.load(*this);
        } else if constexpr (requires(T item) { load(item, *this); }) {
            NewObjectScope<is_range, TSerializer> scope(m_serializer);
            load(item, *this);
        } else if constexpr (requires(T item) { item.serialize(*this); }) {
            NewObjectScope<is_range, TSerializer> scope(m_serializer);
            item.serialize(*this);
        } else if constexpr (requires(T item) { serialize(item, *this); }) {
            NewObjectScope<is_range, TSerializer> scope(m_serializer);
            serialize(item, *this);
        } else if constexpr (requires { serialize(item); }) {
            // the defaul serialization function for basic types
            serialize(item);
        } else {
            static_assert(false,
                          "Unsupported type, define free or member save/load "
                          "function; or serialize function for it.");
        }
    }

    template <typename T>
        requires std::is_arithmetic_v<T>
    void serialize(T &item)
    {
        m_serializer(item);
    }

    template <typename T>
        requires std::is_enum_v<T>
    void serialize(T &item)
    {
        std::underlying_type_t<T> value;
        serialize(value);
        item = static_cast<T>(value);
    }

    void serialize(std::string &item) { m_serializer(item); }

    void serialize(RangeSize &item) { m_serializer(item); }

    template <std::ranges::range Rng>
    void serialize(Rng &items)
    {
        using T = std::ranges::range_value_t<Rng>;
        constexpr bool save_binary =
            std::ranges::contiguous_range<Rng> && std::is_arithmetic_v<T>;

        RangeSize sn(0);
        if constexpr (save_binary) {
            if (m_serializer.IsBinary()) {
                m_serializer(sn);
                std::size_t n = sn.size;

                if constexpr (requires { items.resize(n); }) {
                    items.resize(n);
                }
                auto ptr = reinterpret_cast<char *>(items.data());
                std::span<char> span(ptr, n * sizeof(T));
                m_serializer(span);
                return;
            }
        }

        // for really range types, we need to start NewJsonScope first to get
        // the current array size
        NewObjectScope<true, TSerializer> scope(m_serializer);

        m_serializer(sn);
        std::size_t n = sn.size;

        if constexpr (requires { items.resize(n); }) {
            items.resize(n);
        }
        if constexpr (requires { typename Rng::mapped_type; }) {
            for (std::size_t i = 0; i < n; ++i) {
                typename std::pair<typename Rng::key_type,
                                   typename Rng::mapped_type>
                    item;
                trySerialize(item);
                items.emplace(std::move(item));
            }
        } else {
            for (auto &i : items) {
                trySerialize(i);
            }
        }
    }

    template <typename T>
    void serialize(std::unique_ptr<T> &item)
    {
        T *ptr;
        serialize(ptr, false);
        item = std::unique_ptr<T>(ptr);
    }

    template <typename T>
    void serialize(std::weak_ptr<T> &item)
    {
        std::shared_ptr<T> shared;
        serialize(shared);
        item = shared;
    }

    template <typename T>
    void serialize(std::shared_ptr<T> &item)
    {
        T *ptr;
        serialize(ptr, true);
        if (ptr == nullptr) {
            item.reset();
            return;
        }

        if (m_shared_pointers.contains(ptr)) {
            item = std::static_pointer_cast<T>(m_shared_pointers[ptr]);
            return;
        } else {
            ZEN_THROW("shared_ptr not found");
        }
    }

    template <typename T>
        requires std::is_pointer_v<T>
    void serialize(T &ptr, bool shared = false)
    {
        std::uintptr_t id;
        NewObjectScope<false, TSerializer> scope(m_serializer);
        auto nvp = make_nvp("id", id);
        trySerialize(nvp);
        if (id == 0) {
            ptr = nullptr;
            return;
        }
        if (m_raw_pointers.contains(id)) {
            ptr = static_cast<T>(m_raw_pointers[id]);
            return;
        }

        using TVal = std::remove_pointer_t<T>;

        if constexpr (!std::is_polymorphic_v<TVal>) {
            ptr = Access::Create<TVal>();
            m_raw_pointers[id] = ptr;
            if (shared) {
                m_shared_pointers[ptr] = std::shared_ptr<TVal>(ptr);
            }
            trySerialize(make_nvp("data", *ptr));
        } else {
            std::string type_name;
            trySerialize(make_nvp("type_name", type_name));
            ptr = Create<TVal>(type_name);
            m_raw_pointers[id] = ptr;
            if (shared) {
                m_shared_pointers[ptr] = std::shared_ptr<TVal>(ptr);
            }
            const auto &deserializer = GetDeserializer(type_name);
            m_serializer.SetNextName("data");
            NewObjectScope<false, TSerializer> scope2(m_serializer);
            deserializer(ptr, *this);
        }
    }
};
} // namespace zen

// namespace zen
// {
// class ZEN_SERIALIZATION_EXPORT OutArchive
// {
// public:
//     std::variant<GeneralOutArchive<JsonSerializer>,
//                  GeneralOutArchive<BinarySerializer>>
//         archive;

//     void Flush()
//     {
//         std::visit([](auto &ar) { ar.Flush(); }, archive);
//     }

//     void operator()(auto &&...args)
//     {
//         std::visit(
//             [&](auto &a) { return a(std::forward<decltype(args)>(args)...);
//             }, archive);
//     }
// };

// class ZEN_SERIALIZATION_EXPORT InArchive
// {
// public:
//     std::variant<GeneralInArchive<JsonDeserializer>,
//                  GeneralInArchive<BinaryDeserializer>>
//         archive;

//     void operator()(auto &&...args)
//     {
//         std::visit(
//             [&](auto &a) { return a(std::forward<decltype(args)>(args)...);
//             }, archive);
//     }
// };
// } // namespace zen
/////////// variant serialization ////////////////

#include <variant>
namespace zen
{

template <typename... Args>
void save(const std::variant<Args...> &variant, OutArchive &ar)
{
    ar(make_nvp("index", variant.index()));
    std::visit([&ar](auto &&arg) { ar(make_nvp("value", arg)); }, variant);
}

template <std::size_t N = 0, typename... Args>
std::variant<Args...> create_variant(std::size_t index)
{
    if (index == N) {
        return std::variant<Args...>(std::in_place_index<N>);
    } else {
        if constexpr (N + 1 < sizeof...(Args)) {
            return create_variant<N + 1, Args...>(index);
        } else {
            throw std::out_of_range(std::format("Index out of range {}/{}",
                                                index, sizeof...(Args)));
        }
    }
}

template <typename... Args>
void load(std::variant<Args...> &variant, InArchive &ar)
{
    std::size_t index;
    ar(make_nvp("index", index));
    variant = create_variant<0, Args...>(index);
    std::visit([&ar](auto &&arg) { ar(make_nvp("value", arg)); }, variant);
}

inline void save(const std::monostate &, OutArchive &ar) {}

inline void load(std::monostate &, InArchive &ar) {}
} // namespace zen

//// stack serialization ///////

#include <stack>
namespace zen
{

namespace detail
{
template <typename T>
concept is_stack = requires(T t) {
    t.top();
    t.pop();
    t.push(t.top());
    typename T::container_type;
};

//! Allows access to the protected container in stack
template <is_stack S>
typename S::container_type const &container(const S &stack)
{
    struct H : public S {
        static typename S::container_type const &get(const S &s)
        {
            return s.*(&H::c);
        }
    };

    return H::get(stack);
}
} // namespace detail

template <detail::is_stack S>
void save(const S &s, OutArchive &ar)
{
    ar(make_nvp("container", detail::container(s)));
}

template <detail::is_stack S>
void load(S &s, InArchive &ar)
{
    typename S::container_type c;
    ar(make_nvp("container", c));
    s = S(std::move(c));
}
} // namespace zen

namespace zen
{
template <typename T1, typename T2>
void save(const std::pair<T1, T2> &pair, OutArchive &ar)
{
    ar(make_nvp("first", pair.first), make_nvp("second", pair.second));
}

template <typename T1, typename T2>
void load(std::pair<T1, T2> &pair, InArchive &ar)
{
    ar(make_nvp("first", pair.first), make_nvp("second", pair.second));
}

// set deserializer
template <typename T, typename Comp, typename Alloc>
void load(std::set<T, Comp, Alloc> &set, InArchive &ar)
{
    RangeSize range_size(0);
    ar(range_size);
    for (std::size_t i = 0; i < range_size.size; ++i) {
        T value;
        ar(value);
        set.insert(std::move(value));
    }
}

} // namespace zen

namespace zen
{
// serialization for tuple

template <typename... Args>
void save(const std::tuple<Args...> &tuple, OutArchive &ar)
{
    // std::apply([&](auto &&...elems) { ar(elems...); }, tuple);
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (ar(make_nvp(std::to_string(I), std::get<I>(tuple))), ...);
    }(std::make_index_sequence<sizeof...(Args)>{});
}

template <typename... Args>
void load(std::tuple<Args...> &tuple, InArchive &ar)
{
    // std::apply([&](auto &&...elems) { ar(elems...); }, tuple);
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (ar(make_nvp(std::to_string(I), std::get<I>(tuple))), ...);
    }(std::make_index_sequence<sizeof...(Args)>{});
}
} // namespace zen

namespace zen
{
namespace detail
{
template <typename T>
concept is_bitset = requires(T t) {
    t.to_string();
    t.to_ullong();
    t.flip();
};
} // namespace detail

template <detail::is_bitset T>
void save(const T &t, OutArchive &ar)
{
    ar(make_nvp("value", t.to_string()));
}

template <detail::is_bitset T>
void load(T &t, InArchive &ar)
{
    std::string str;
    ar(make_nvp("value", str));
    t = T(str);
}

} // namespace zen

// namespace zen