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

#include <iterator>
#include <optional>

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
                  serializer.FinishArray();
              } else {
                  serializer.FinishObject();
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

    static constexpr bool IsInput() { return false; }

    template <typename... Args>
    OutArchive(Args &&...args) : m_serializer(std::forward<Args>(args)...)
    {
    }

    void Flush() { m_serializer.Flush(); }

    bool IsBinary() const { return m_serializer.IsBinary(); }

    void operator()(auto &&item1, auto &&...items)
    {
        process(make_nvp(item1));
        if constexpr (sizeof...(items) > 0) {
            (*this)(items...);
        }
    }

private:
    template <typename T>
    void process(const NamedValuePair<T> &item)
    {
        m_serializer.SetNextName(item.name);
        process(item.value);
    }

    template <typename T>
    void process(const T &item)
    {
        if constexpr (std::is_arithmetic_v<T>) {
            m_serializer(item);
        } else if constexpr (std::is_enum_v<T>) {
            m_serializer(std::to_underlying(item));
        } else if constexpr (requires(T t) { t.serialize(*this); }) {
            NewObjectScope<false, TSerializer> scope(m_serializer);
            const_cast<T &>(item).serialize(*this);
        } else if constexpr (requires(T t) { serialize(t, *this); }) {
            NewObjectScope<false, TSerializer> scope(m_serializer);
            serialize(const_cast<T &>(item), *this);
        } else if constexpr (std::ranges::range<T>) {
            processRange(item);
        } else {
            static_assert(
                false,
                "Unsupported type, define free or member serialize function");
        }
    }

    template <typename T>
    void process(const BaseClass<T> &item)
    {
        process(*item.ptr);
    }

    void process(const RangeSize &item) { m_serializer(item.size); }

    void process(const std::string &item) { m_serializer(item); }

    template <std::ranges::range Rng>
    void processRange(const Rng &items)
    {
        using T = std::ranges::range_value_t<Rng>;
        constexpr bool save_binary =
            std::ranges::contiguous_range<Rng> && std::is_arithmetic_v<T>;

        std::size_t n = std::ranges::distance(items);

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
            process(i);
        }
    }

    template <typename T>
    void process(const std::unique_ptr<T> &item)
    {
        process(item.get());
    }

    template <typename T>
    void process(const std::weak_ptr<T> &item)
    {
        process(item.lock());
    }

    template <typename T>
    void process(const std::shared_ptr<T> &item)
    {
        process(item.get());
    }

    template <typename T>
        requires std::is_pointer_v<T>
    void process(const T &item)
    {
        auto id = reinterpret_cast<std::uintptr_t>(item);
        NewObjectScope<false, TSerializer> scope(m_serializer);
        process(NVP(id));
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
            process(make_nvp("data", *item));
            return;
        } else {
            const auto &type_name = GetClassName(typeid(*item));
            process(make_nvp("type_name", type_name));
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

    InArchive(const InArchive &) = delete;

public:
    using TSerializer = InDeserializer;

    static constexpr bool IsInput() { return true; }

    template <typename... Args>
    InArchive(Args &&...args) : m_serializer(std::forward<Args>(args)...)
    {
    }

    bool IsBinary() const { return m_serializer.IsBinary(); }

    void operator()(auto &&item1, auto &&...items)
    {
        process(make_nvp(item1));
        if constexpr (sizeof...(items) > 0) {
            (*this)(items...);
        }
    }

private:
    template <typename T>
    void process(NamedValuePair<T> &&item)
    {
        m_serializer.SetNextName(item.name);
        process(item.value);
    }

    template <typename T>
    void process(NamedValuePair<T> &item)
    {
        m_serializer.SetNextName(item.name);
        process(item.value);
    }

    template <typename T>
    void process(T &item)
    {
        constexpr bool is_range = std::ranges::range<T>;
        if constexpr (std::is_arithmetic_v<T>) {
            m_serializer(item);
        } else if constexpr (std::is_enum_v<T>) {
            std::underlying_type_t<T> value;
            m_serializer(value);
            item = static_cast<T>(value);
        } else if constexpr (std::same_as<T, RangeSize>) {
            m_serializer(item);
        } else if constexpr (std::is_pointer_v<T>) {
            processPointer(item);
        } else if constexpr (requires(T t) { t.serialize(*this); }) {
            NewObjectScope<is_range, TSerializer> scope(m_serializer);
            item.serialize(*this);
        } else if constexpr (requires(T t) { serialize(t, *this); }) {
            NewObjectScope<is_range, TSerializer> scope(m_serializer);
            serialize(item, *this);
        } else if constexpr (is_range) {
            processRange(item);
        }

        else {
            static_assert(false, "Unsupported type, define free or member "
                                 "load/serialize function");
        }
    }

    void process(std::string &item) { m_serializer(item); }

    template <typename T>
    void process(BaseClass<T> &item)
    {
        process(const_cast<T &>(*item.ptr));
    }

    template <std::ranges::range Rng>
    void processRange(Rng &items)
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

        // for really range types, we need to start NewObjectScope first to get
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
                process(item);
                items.emplace(std::move(item));
            }
        } else if constexpr (requires {
                                 requires !std::output_iterator<
                                     typename Rng::iterator, T>;
                                 items.emplace(std::declval<T>());
                             }) {
            for (std::size_t i = 0; i < n; ++i) {
                typename Rng::value_type item;
                process(item);
                items.emplace(std::move(item));
            }
        } else if constexpr (requires {
                                 requires std::output_iterator<
                                     typename Rng::iterator, T>;
                             }) {
            for (auto &i : items) {
                process(i);
            }
        } else {
            static_assert(false, "this range type is not supported, please "
                                 "provide a specific serialize function");
        }
    }

    template <typename T>
    void process(std::unique_ptr<T> &item)
    {
        T *ptr;
        processPointer<std::add_pointer_t<T>, false>(ptr);
        item = std::unique_ptr<T>(ptr);
    }

    template <typename T>
    void process(std::weak_ptr<T> &item)
    {
        std::shared_ptr<T> shared;
        process(shared);
        item = shared;
    }

    template <typename T>
    void process(std::shared_ptr<T> &item)
    {
        T *ptr;
        processPointer<std::add_pointer_t<T>, true>(ptr);
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

    template <typename T, bool IsShared = false>
        requires std::is_pointer_v<T>
    void processPointer(T &ptr)
    {
        std::uintptr_t id;
        NewObjectScope<false, TSerializer> scope(m_serializer);
        auto nvp = make_nvp("id", id);
        process(nvp);
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
            if constexpr (IsShared) {
                m_shared_pointers[ptr] = std::shared_ptr<TVal>(ptr);
            }
            process(make_nvp("data", *ptr));
        } else {
            std::string type_name;
            process(make_nvp("type_name", type_name));
            ptr = Create<TVal>(type_name);
            m_raw_pointers[id] = ptr;
            if constexpr (IsShared) {
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

namespace zen
{
// serialization for comple
template <typename T, typename TArchive>
    requires requires(T t) {
        t.real();
        t.imag();
    }
void serialize(T &item, TArchive &ar)
{
    if constexpr (ar.IsInput()) {
        using value_type = typename T::value_type;
        value_type real, imag;
        ar(make_nvp("real", real));
        ar(make_nvp("imag", imag));
        item = T(real, imag);
    } else {
        ar(make_nvp("real", item.real()));
        ar(make_nvp("imag", item.imag()));
    }
}

} // namespace zen

/////////// variant serialization ////////////////

#include <variant>
namespace zen
{

template <typename... Args>
void serialize(const std::variant<Args...> &variant, OutArchive &ar)
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
void serialize(std::variant<Args...> &variant, InArchive &ar)
{
    std::size_t index;
    ar(make_nvp("index", index));
    variant = create_variant<0, Args...>(index);
    std::visit([&ar](auto &&arg) { ar(make_nvp("value", arg)); }, variant);
}

template <typename TArchive>
inline void serialize(std::monostate &, TArchive &ar)
{
}

template <typename TArchive>
void serialize(std::filesystem::path &item, TArchive &ar)
{
    if constexpr (ar.IsInput()) {
        std::string str;
        ar(make_nvp("path", str));
        std::u8string path{str.begin(), str.end()};
        item = std::filesystem::path{path};
    } else {
        std::u8string path = item.u8string();
        std::string str{path.begin(), path.end()};
        ar(make_nvp("path", str));
    }
}

template <typename T, typename TArchive>
void serialize(std::optional<T> &item, TArchive &ar)
{
    bool has_value = item.has_value();
    ar(make_nvp("has_value", has_value));
    if (has_value) {
        if constexpr (ar.IsInput()) {
            // construct item before serialize
            item.emplace();
        }
        ar(make_nvp("value", *item));
    }
}

} // namespace zen

//// stack serialization ///////

#include <stack>
namespace zen
{

namespace detail
{
template <typename T>
concept have_inner_container = requires(T t) {
    // stack or queue
    typename T::container_type;
    // not a range
    requires !std::ranges::range<T>;
};

//! Allows access to the protected container in stack
template <have_inner_container S>
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

template <detail::have_inner_container S>
void serialize(const S &s, OutArchive &ar)
{
    ar(make_nvp("container", detail::container(s)));
}

template <detail::have_inner_container S>
void serialize(S &s, InArchive &ar)
{
    typename S::container_type c;
    ar(make_nvp("container", c));
    if constexpr (std::is_constructible_v<S, typename S::container_type>) {
        s = S(std::move(c));
    } else {
        // priority_queue specific case
        s = S(std::begin(c), std::end(c));
    }
}
} // namespace zen

namespace zen
{

/// serialization for pair
template <typename T1, typename T2, typename TArchive>
void serialize(std::pair<T1, T2> &pair, TArchive &ar)
{
    ar(make_nvp("first", pair.first), make_nvp("second", pair.second));
}

/// serialization for tuple
template <typename TArchive, typename... Args>
void serialize(std::tuple<Args...> &tuple, TArchive &ar)
{
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
void serialize(const T &t, OutArchive &ar)
{
    ar(make_nvp("value", t.to_string()));
}

template <detail::is_bitset T>
void serialize(T &t, InArchive &ar)
{
    std::string str;
    ar(make_nvp("value", str));
    t = T(str);
}

} // namespace zen

#if __has_include(<expected>)
#include <expected>
namespace zen
{
template <typename U, typename E, typename TArchive>
void serialize(std::expected<U, E> &item, TArchive &ar)
{
    bool has_value = item.has_value();
    ar(make_nvp("has_value", has_value));
    if constexpr (ar.IsInput()) {
        if (has_value) {
            U value;
            ar(make_nvp("value", value));
            item = std::move(value);
        } else {
            E error;
            ar(make_nvp("error", error));
            item = std::unexpected(std::move(error));
        }
    } else {
        if (has_value) {
            ar(make_nvp("value", item.value()));
        } else {
            ar(make_nvp("error", item.error()));
        }
    }
}
} // namespace zen
#endif