/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file aggregate.h
 * @author: Zen Shawn
 * @email: xiaozisheng2008@hotmail.com
 * @date: 18:58:42, October 11, 2025
 */

#pragma once
#include "archive.h"
#include <boost/pfr.hpp>

namespace zen
{

template <typename T>
    requires std::is_class_v<T> && std::is_aggregate_v<T>
void serialize(T &item, auto &ar)
{
    boost::pfr::for_each_field_with_name(
        item, [&ar](std::string_view name, auto &&field) {
            ar(make_nvp(std::string{name}, field));
        });
}

} // namespace zen
