/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file range_size.h
 * @author: Zen Shawn
 * @email: xiaozisheng2008@hotmail.com
 * @date: 18:54:25, September 18, 2025
 */
#pragma once
#include <ranges>

namespace zen
{
struct RangeSize {
    explicit RangeSize(std::size_t size = 0) : size(static_cast<uint64_t>(size))
    {
    }

    template <std::ranges::range Rng>
    explicit RangeSize(Rng &&r)
        : size(static_cast<uint64_t>(std::ranges::distance(r)))
    {
    }
    uint64_t size;
};
} // namespace zen