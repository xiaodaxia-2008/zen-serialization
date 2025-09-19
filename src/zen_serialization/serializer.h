/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file serializer.h
 * @author: Zen Shawn
 * @email: xiaozisheng2008@hotmail.com
 * @date: 19:00:06, September 18, 2025
 */
#pragma once

#include "binary_serializer.h"
#include "json_serializer.h"

#include <variant>

namespace zen
{

namespace detail
{

template <typename... Ts>
struct Serializer {
    std::variant<Ts...> ser;

    template <typename... Args>
    Serializer(Args &&...ts) : ser(std::forward<Args>(ts)...)
    {
    }

    bool IsBinary() const
    {
        return std::visit([](auto &s) { return s.IsBinary(); }, ser);
    }

    void Flush()
    {
        std::visit([](auto &s) { s.Flush(); }, ser);
    }

    template <typename... Ts>
    void operator()(Ts &&...args)
    {
        std::visit([&](auto &s) { s(args...); }, ser);
    }

    void SetNextName(std::string_view name)
    {
        std::visit([name](auto &s) { s.SetNextName(name); }, ser);
    }

    void NewObject()
    {
        std::visit([](auto &s) { s.NewObject(); }, ser);
    }

    void FinishObject()
    {
        std::visit([](auto &s) { s.FinishObject(); }, ser);
    }

    void NewArray()
    {
        std::visit([](auto &s) { s.NewArray(); }, ser);
    }

    void FinishArray()
    {
        std::visit([](auto &s) { s.FinishArray(); }, ser);
    };
};
} // namespace detail

#ifndef ZEN_SERIALIZATION_OUT_SERIALIZER
using OutSerializer = detail::Serializer<JsonSerializer, BinarySerializer>;
#else
using OutSerializer = detail::Serializer<ZEN_SERIALIZATION_OUT_SERIALIZER>;
#endif

#ifndef ZEN_SERIALIZATION_IN_DESERIALIZER
using InDeserializer = detail::Serializer<JsonDeserializer, BinaryDeserializer>;
#else
using InDeserializer = detail::Serializer<ZEN_SERIALIZATION_IN_DESERIALIZER>;
#endif
} // namespace zen