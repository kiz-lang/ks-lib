#pragma once

#include "string.hpp"
#include <cstdio>
#include <cstdlib>

namespace ks {

/// 如果 expr 为 false，打印错误消息并终止程序
inline auto check(bool expr, const String& msg) -> void {
    if (!expr) {
        std::fprintf(stderr, "ks::check failed: %s\n", msg.c_str());
        std::abort();
    }
}

} // namespace ks