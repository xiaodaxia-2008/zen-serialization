/**
 * Copyright © 2025 Zen Shawn. All rights reserved.
 *
 * @file base64.h
 * @author: Zen Shawn
 * @email: xiaozisheng2008@hotmail.com
 * @date: 22:24:34, September 17, 2025
 */
#pragma once
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace zen
{

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

inline std::string base64_encode(std::span<const char> input)
{
    std::string output;
    int val = 0;
    int valb = -6;
    for (auto c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            output.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        output.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (output.size() % 4) {
        output.push_back('=');
    }
    return output;
}

inline std::vector<char> base64_decode(std::string_view input)
{
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) {
        T[base64_chars[i]] = i;
    }

    std::vector<char> output;
    int val = 0;
    int valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) {
            break;
        }
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            output.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return output;
}

} // namespace zen