#pragma once

#include "string.hpp"
#include <cstdio>
#include <string>
#include <sstream>
#include <utility>

namespace ks {

namespace detail {

/// 将任意类型转换为 ks::String 的内部辅助函数
template<typename T>
auto to_string_impl(T&& value) -> String {
    using RawType = std::decay_t<T>;
    if constexpr (std::is_arithmetic_v<RawType>) {
        // 算术类型使用 std::to_string
        return String(std::to_string(std::forward<T>(value)));
    } else if constexpr (std::is_same_v<RawType, String>) {
        // 已经是 ks::String，直接转发
        return std::forward<T>(value);
    } else if constexpr (std::is_same_v<RawType, std::string>) {
        // std::string 转换为 ks::String
        return String(std::forward<T>(value));
    } else if constexpr (std::is_same_v<RawType, const char*>) {
        // C 字符串
        return String(std::forward<T>(value));
    } else if constexpr (std::is_same_v<RawType, char>) {
        // 单个字符
        return String(1, value);
    } else {
        // 其他类型尝试使用输出流（假设不抛出异常）
        std::ostringstream oss;
        oss << value;
        return String(oss.str());
    }
}

} // namespace detail

/// 内部格式化函数：将格式字符串 fmt 中的 {} 依次替换为 args 的字符串表示
template<typename... Args>
auto format_to_string(const String& fmt, Args&&... args) -> String {
    std::array<String, sizeof...(args)> arg_strings = {
        detail::to_string_impl(std::forward<Args>(args))...
    };
    std::string result;
    const char* f = fmt.c_str();
    size_t f_len = fmt.len();
    size_t arg_index = 0;

    for (size_t i = 0; i < f_len; ) {
        if (f[i] == '{') {
            if (i + 1 < f_len && f[i + 1] == '{') {
                result += '{';
                i += 2;
                continue;
            } else if (i + 1 < f_len && f[i + 1] == '}') {
                ks::check(arg_index < arg_strings.size(),
                          "format error: too few arguments");
                result.append(arg_strings[arg_index].c_str(),
                              arg_strings[arg_index].len());
                ++arg_index;
                i += 2;
                continue;
            }
        } else if (f[i] == '}') {
            if (i + 1 < f_len && f[i + 1] == '}') {
                result += '}';
                i += 2;
                continue;
            }
        }
        result += f[i];
        ++i;
    }

    ks::check(arg_index == arg_strings.size(),
              "format error: too many arguments");
    return String(std::move(result));
}

/// 打印格式化字符串到标准输出
template<typename... Args>
auto print(const String& fmt, Args&&... args) -> void {
    String output = format_to_string(fmt, std::forward<Args>(args)...);
    std::fwrite(output.c_str(), 1, output.len(), stdout);
}

/// 打印格式化字符串并换行
template<typename... Args>
auto println(const String& fmt, Args&&... args) -> void {
    print(fmt, std::forward<Args>(args)...);
    std::fwrite("\n", 1, 1, stdout);
}

/// 重载：接受 C 字符串格式
template<typename... Args>
auto print(const char* fmt, Args&&... args) -> void {
    print(String(fmt), std::forward<Args>(args)...);
}

/// 重载：接受 C 字符串格式并换行
template<typename... Args>
auto println(const char* fmt, Args&&... args) -> void {
    println(String(fmt), std::forward<Args>(args)...);
}

} // namespace ks