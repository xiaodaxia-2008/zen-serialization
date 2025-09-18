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

namespace zen
{
struct RangeSize {
    explicit RangeSize(std::size_t size) : size(size) {}
    std::size_t size;
};

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

class JsonSerializer
{
    std::ostream &m_stream;
    std::vector<nlohmann::json> m_objects;
    std::size_t m_idx = 0;
    std::vector<std::string> m_next_names;

public:
    static constexpr bool is_binary = false;

    JsonSerializer(std::ostream &stream) : m_stream(stream)
    {
        m_objects.emplace_back(nlohmann::json::object());
    }

    void Flush(int indent = 4) { m_stream << Json().dump(indent); }

    ~JsonSerializer() { Flush(); }

    const nlohmann::json &Json() const
    {
        assert(m_objects.size() == 1);
        return m_objects.back();
    }

    void SetNextName(std::string_view name) { m_next_names.emplace_back(name); }

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

    void NewObject() { m_objects.emplace_back(nlohmann::json::object()); }

    void FinishObject()
    {
        auto current = std::move(m_objects.back());
        m_objects.pop_back();
        auto &parent = m_objects.back();
        if (parent.is_array()) {
            parent.push_back(std::move(current));
        } else {
            const auto &key = NextName();
            parent[key] = std::move(current);
        }
    }

    void NewArray() { m_objects.emplace_back(nlohmann::json::array()); }

    void operator()(const RangeSize &size) {}

    void operator()(std::span<const std::uint8_t> bytes)
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
            throw std::runtime_error("Invalid json type");
        }
    }
};

class JsonDeserializer
{
    std::istream &m_stream;
    nlohmann::json m_json;
    std::vector<nlohmann::json> m_objects;
    std::size_t m_idx = 0;

    std::vector<std::string> m_next_names;

    std::stack<std::size_t, std::vector<std::size_t>> m_arr_idxes;

public:
    static constexpr bool is_binary = false;

    JsonDeserializer(std::istream &stream) : m_stream(stream)
    {
        m_stream >> m_json;
        m_objects.emplace_back(m_json);
    }

    const nlohmann::json &Json() const { return m_objects.back(); }

    void SetNextName(std::string_view name) { m_next_names.emplace_back(name); }

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

    void NewObject()
    {
        auto &current = m_objects.back();
        if (current.is_object()) {
            m_objects.emplace_back(current[NextName()]);
        } else if (current.is_array()) {
            m_objects.emplace_back(current[m_arr_idxes.top()++]);
        } else {
            ZEN_THROW("cannot obtain new object");
        }
    }

    void FinishObject()
    {
        if (m_objects.back().is_array()) {
            m_arr_idxes.pop();
        }
        m_objects.pop_back();
    }

    void NewArray()
    {
        m_objects.emplace_back(m_objects.back()[NextName()]);
        m_arr_idxes.push(0);
    }

    void operator()(RangeSize &size)
    {
        auto &current = m_objects.back();
        if (current.is_array()) {
            size.size = current.size();
        } else {
            ZEN_THROW("Current object is not an array");
        }
    }

    void operator()(std::span<std::uint8_t> bytes)
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
        auto &current = m_objects.back();
        if (current.is_object()) {
            current[NextName()].get_to(t);
        } else if (current.is_array()) {
            current[m_arr_idxes.top()++].get_to(t);
        } else {
            ZEN_THROW(fmt::format("Invalid json type {}", current.dump()));
        }
    }
};

} // namespace zen
namespace zen
{

template <bool IsArray, typename TSerializer>
struct NewJsonScope {
    TSerializer &serializer;
    Scope scope;

    NewJsonScope(TSerializer &ser)
        : serializer(ser), scope([&] { serializer.FinishObject(); })
    {
        if constexpr (IsArray) {
            serializer.NewArray();
        } else {
            serializer.NewObject();
        }
    }
};

class ZEN_SERIALIZATION_EXPORT OutArchive : public ArchiveBase
{
public:
    using TSerializer = JsonSerializer;

private:
    std::set<std::uintptr_t> m_pointers;

    TSerializer m_serializer;

public:
    constexpr bool IsOutput() const { return true; }

    OutArchive(std::ostream &stream) : m_serializer(stream) {}

    ~OutArchive() {}

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
            NewJsonScope<is_range, TSerializer> scope(m_serializer);
            item.save(*this);
        } else if constexpr (requires(const T item) { save(item, *this); }) {
            NewJsonScope<is_range, TSerializer> scope(m_serializer);
            save(item, *this);
        } else if constexpr (requires(T item) { item.serialize(*this); }) {
            NewJsonScope<is_range, TSerializer> scope(m_serializer);
            const_cast<T &>(item).serialize(*this);
        } else if constexpr (requires(T item) { serialize(item, *this); }) {
            NewJsonScope<is_range, TSerializer> scope(m_serializer);
            serialize(const_cast<T &>(item), *this);
        } else if constexpr (requires(T item) { serialize(item); }) {
            serialize(item);
        } else {
            static_assert(false, "Unsupported type");
        }
    }

    template <typename T>
        requires std::is_arithmetic_v<T>
    void serialize(const T &item)
    {
        m_serializer(item);
    }

    template <typename T>
        requires std::is_enum_v<T>
    void serialize(const T &item)
    {
        serialize(static_cast<std::underlying_type_t<T>>(item));
    }

    void serialize(const std::string &item) { m_serializer(item); }

    void serialize(const RangeSize &item) { m_serializer(item); }

    template <std::ranges::range Rng>
    void serialize(const Rng &items)
    {
        using T = std::ranges::range_value_t<Rng>;
        constexpr bool save_binary = TSerializer::is_binary &&
                                     std::ranges::contiguous_range<Rng> &&
                                     std::is_arithmetic_v<T>;

        std::size_t n;
        if constexpr (std::ranges::sized_range<Rng>) {
            n = std::ranges::size(items);
        } else {
            n = std::distance(std::ranges::begin(items),
                              std::ranges::end(items));
        }

        serialize(RangeSize(n));
        if constexpr (save_binary) {
            auto ptr = reinterpret_cast<const uint8_t *>(items.data());
            std::span<const std::uint8_t> bytes(ptr, n * sizeof(T));
            m_serializer(bytes);
        } else {
            NewJsonScope<true, TSerializer> scope(m_serializer);
            for (const auto &i : items) {
                trySerialize(i);
            }
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
        serialize(item.lock().get());
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
        NewJsonScope<false, TSerializer> scope(m_serializer);
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
            NewJsonScope<false, TSerializer> scope2(m_serializer);
            serializer(item, *this);
        }
    }
};

class ZEN_SERIALIZATION_EXPORT InArchive : public ArchiveBase
{
public:
    using TSerializer = JsonDeserializer;

private:
    std::map<void *, std::shared_ptr<void>> m_shared_pointers;
    std::map<std::uintptr_t, void *> m_raw_pointers;

    TSerializer m_serializer;

public:
    InArchive(std::istream &stream) : m_serializer(stream) {}

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
            NewJsonScope<is_range, TSerializer> scope(m_serializer);
            item.load(*this);
        } else if constexpr (requires(T item) { load(item, *this); }) {
            NewJsonScope<is_range, TSerializer> scope(m_serializer);
            load(item, *this);
        } else if constexpr (requires(T item) { item.serialize(*this); }) {
            NewJsonScope<is_range, TSerializer> scope(m_serializer);
            item.serialize(*this);
        } else if constexpr (requires(T item) { serialize(item, *this); }) {
            NewJsonScope<is_range, TSerializer> scope(m_serializer);
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
        constexpr bool save_binary = TSerializer::is_binary &&
                                     std::ranges::contiguous_range<Rng> &&
                                     std::is_arithmetic_v<T>;

        RangeSize sn(0);
        if constexpr (save_binary) {
            serialize(sn);
            std::size_t n = sn.size;

            if constexpr (requires { items.resize(n); }) {
                items.resize(n);
            }
            auto ptr = reinterpret_cast<uint8_t *>(items.data());
            std::span<uint8_t> span(ptr, n * sizeof(T));
            m_serializer(span);
            return;
        }

        // for really range types, we need to start NewJsonScope first to get
        // the current array size
        NewJsonScope<true, TSerializer> scope(m_serializer);

        serialize(sn);
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
        NewJsonScope<false, TSerializer> scope(m_serializer);
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
            NewJsonScope<false, TSerializer> scope2(m_serializer);
            deserializer(ptr, *this);
        }
    }
};
} // namespace zen

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

void save(const std::monostate &, OutArchive &ar) {}

void load(std::monostate &, InArchive &ar) {}
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
