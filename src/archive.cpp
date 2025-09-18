/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file Archive.cpp
 * @author: Zen Shawn
 * @email: xiaozisheng2008@hotmail.com
 * @date: 11:13:18, September 16, 2025
 */
#include <zen_serialization/archive.h>
#include <zen_serialization/archive_base.h>
#include <zen_serialization/base64.h>
#include <zen_serialization/binary_serializer.h>
#include <zen_serialization/json_serializer.h>
#include <zen_serialization/range_size.h>
#include <zen_serialization/serializer.h>

namespace zen
{
static std::map<std::string, std::function<void *()>> g_constructors;
static std::map<std::type_index, std::string> g_type_names;

static std::map<std::string, std::function<void(void *, OutArchive &ar)>>
    g_serializers;
static std::map<std::string, std::function<void(void *, InArchive &ar)>>
    g_deserializers;

void ArchiveBase::RegisterClassName(const std::type_index &index,
                                    const std::string &name)
{
    g_type_names[index] = name;
}

const std::string &ArchiveBase::GetClassName(const std::type_index &index)
{
    return g_type_names.at(index);
}

void ArchiveBase::RegisterConstructor(const std::string &name,
                                      std::function<void *()> constructor)
{
    g_constructors[name] = std::move(constructor);
}

const std::function<void *()> &
ArchiveBase::GetConstructor(const std::string &name)
{
    return g_constructors.at(name);
}

void ArchiveBase::RegisterSerializer(
    const std::string &name,
    std::function<void(void *, OutArchive &ar)> serializer)
{
    g_serializers[name] = std::move(serializer);
}

void ArchiveBase::RegisterDeserializer(
    const std::string &name,
    std::function<void(void *, InArchive &ar)> deserializer)
{
    g_deserializers[name] = std::move(deserializer);
}

const std::function<void(void *, OutArchive &ar)> &
ArchiveBase::GetSerializer(const std::string &name)
{
    return g_serializers.at(name);
}

const std::function<void(void *, InArchive &ar)> &
ArchiveBase::GetDeserializer(const std::string &name)
{
    return g_deserializers.at(name);
}

} // namespace zen