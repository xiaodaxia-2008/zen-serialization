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

#include <iostream>
#include <span>
#include <string_view>

namespace zen
{

struct BaseSerializer {
    void SetNextName(std::string_view /*name*/) {}
    void NewObject() {}
    void FinishObject() {}
    void NewArray() {}
    void FinishArray() {};
    void Flush() {}
};

class BinarySerializer : public BaseSerializer
{
    std::ostream &m_stream;

public:
    BinarySerializer(std::ostream &stream) : m_stream(stream) {}

    static constexpr bool IsBinary() { return true; }

    void operator()(const RangeSize &size) { (*this)(size.size); }

    void operator()(const std::string &str)
    {
        (*this)(static_cast<uint64_t>(str.size()));
        (*this)(std::span<const char>(str));
    }

    void operator()(std::span<const char> bytes)
    {
        auto n = static_cast<std::streamsize>(bytes.size_bytes());
        m_stream.write(bytes.data(), n);

        if (!m_stream) {
            ZEN_THROW(fmt::format("Failed to write {} bytes to stream", n));
        }
    }

    template <typename T>
        requires std::is_arithmetic_v<T>
    void operator()(const T &t)
    {
        auto ptr = reinterpret_cast<const char *>(std::addressof(t));
        auto n = static_cast<std::streamsize>(sizeof(T));
        m_stream.write(ptr, n);
        if (!m_stream) {
            ZEN_THROW(fmt::format("Failed to write {} bytes to stream", n));
        }
    }
};

class BinaryDeserializer : public BaseSerializer
{
    std::istream &m_stream;

public:
    static constexpr bool is_binary = true;

    BinaryDeserializer(std::istream &stream) : m_stream(stream) {}

    static constexpr bool IsBinary() { return true; }

    void operator()(RangeSize &size) { (*this)(size.size); }

    void operator()(std::string &str)
    {
        uint64_t size;
        (*this)(size);
        str.resize(size);
        (*this)(std::span<char>(str));
    }

    void operator()(std::span<char> bytes)
    {
        auto n = static_cast<std::streamsize>(bytes.size_bytes());
        m_stream.read(bytes.data(), n);
        if (!m_stream) {
            ZEN_THROW(fmt::format("Failed to read {} bytes from stream", n));
        }
    }

    template <typename T>
        requires std::is_arithmetic_v<T>
    void operator()(T &t)
    {
        auto ptr = reinterpret_cast<char *>(std::addressof(t));
        auto n = static_cast<std::streamsize>(sizeof(T));
        m_stream.read(ptr, n);
        if (!m_stream) {
            ZEN_THROW(fmt::format("Failed to read {} bytes from stream", n));
        }
    }
};
} // namespace zen