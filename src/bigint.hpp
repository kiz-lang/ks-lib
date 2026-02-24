#pragma once

#include "result.hpp"
#include "string.hpp"
#include "check.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <ostream>
#include <istream>

namespace ks {

/// 无限精度整数，使用 10^9 为基数的存储，每个元素存储 0-999999999
class BigInt {
public:
    using Digit = uint32_t;
    using DoubleDigit = uint64_t;
    static const Digit BASE = 1000000000;  // 10^9
    static const int DIGIT_BITS = 9;       // 每个 digit 对应的十进制位数

    // ========== 构造函数 ==========

    /// 默认构造为 0
    BigInt() : data_{0}, negative_{false} {}

    /// 从内置无符号整数构造
    explicit BigInt(uint64_t val) : negative_{false} {
        if (val == 0) {
            data_.push_back(0);
        } else {
            while (val > 0) {
                data_.push_back(static_cast<Digit>(val % BASE));
                val /= BASE;
            }
        }
    }

    /// 从内置有符号整数构造
    explicit BigInt(int64_t val) : negative_{val < 0} {
        uint64_t abs_val = val < 0 ? -static_cast<uint64_t>(val) : static_cast<uint64_t>(val);
        if (abs_val == 0) {
            data_.push_back(0);
            negative_ = false;  // 零没有符号
        } else {
            while (abs_val > 0) {
                data_.push_back(static_cast<Digit>(abs_val % BASE));
                abs_val /= BASE;
            }
        }
    }

    /// 从 C 字符串构造，失败时返回错误（通过静态方法）
    static auto from_string(const char* str) -> Result<BigInt, String>;

    /// 从 std::string 构造
    static auto from_string(const std::string& str) -> Result<BigInt, String> {
        return from_string(str.c_str());
    }

    /// 从 ks::String 构造
    static auto from_string(const String& str) -> Result<BigInt, String> {
        return from_string(str.c_str());
    }

    /// 拷贝构造
    BigInt(const BigInt& other) = default;

    /// 移动构造
    BigInt(BigInt&& other) noexcept = default;

    /// 拷贝赋值
    auto operator=(const BigInt& other) -> BigInt& = default;

    /// 移动赋值
    auto operator=(BigInt&& other) noexcept -> BigInt& = default;

    // ========== 比较运算符 ==========

    auto operator==(const BigInt& other) const -> bool;
    auto operator!=(const BigInt& other) const -> bool;
    auto operator<(const BigInt& other) const -> bool;
    auto operator<=(const BigInt& other) const -> bool;
    auto operator>(const BigInt& other) const -> bool;
    auto operator>=(const BigInt& other) const -> bool;

    // ========== 算术运算符 ==========

    auto operator+() const -> const BigInt& { return *this; }
    auto operator-() const -> BigInt;

    auto operator+(const BigInt& other) const -> BigInt;
    auto operator-(const BigInt& other) const -> BigInt;
    auto operator*(const BigInt& other) const -> BigInt;
    auto operator/(const BigInt& other) const -> BigInt;
    auto operator%(const BigInt& other) const -> BigInt;

    auto operator+=(const BigInt& other) -> BigInt&;
    auto operator-=(const BigInt& other) -> BigInt&;
    auto operator*=(const BigInt& other) -> BigInt&;
    auto operator/=(const BigInt& other) -> BigInt&;
    auto operator%=(const BigInt& other) -> BigInt&;

    /// 幂运算（指数为非负整数）
    auto pow(const BigInt& exponent) const -> Result<BigInt, String>;

    // ========== 其他实用方法 ==========

    /// 返回绝对值
    auto abs() const -> BigInt;

    /// 返回符号（0 表示非负，1 表示负）
    auto sign() const -> int;

    /// 判断是否为零
    auto is_zero() const -> bool;

    /// 转换为十进制字符串
    auto to_string() const -> std::string;

    /// 转换为 ks::String
    auto to_ks_string() const -> String;

    /// 转换为 uint64_t（如果超出范围返回错误）
    auto to_uint64() const -> Result<uint64_t, String>;

    /// 转换为 int64_t
    auto to_int64() const -> Result<int64_t, String>;

    // ========== 输入输出友元 ==========

    friend auto operator<<(std::ostream& os, const BigInt& num) -> std::ostream&;
    friend auto operator>>(std::istream& is, BigInt& num) -> std::istream&;

private:
    std::vector<Digit> data_;  // 低位在前，每个元素 0-999999999
    bool negative_;

    // 私有辅助函数

    /// 去除前导零，并确保至少有一个零
    void trim();

    /// 比较绝对值：返回 *this 的绝对值是否小于 other 的绝对值
    auto abs_less(const BigInt& other) const -> bool;

    /// 无符号加法：|this| + |other|，结果存入 result（无符号）
    static auto unsigned_add(const BigInt& a, const BigInt& b) -> BigInt;

    /// 无符号减法：|a| - |b|，要求 |a| >= |b|，结果存入 result（无符号）
    static auto unsigned_sub(const BigInt& a, const BigInt& b) -> BigInt;

    /// 无符号乘法：|a| * |b|
    static auto unsigned_mul(const BigInt& a, const BigInt& b) -> BigInt;

    /// 无符号除法：|a| / |b|，返回 (商, 余数)，要求 b 非零
    static auto unsigned_div_mod(const BigInt& a, const BigInt& b) -> std::pair<BigInt, BigInt>;

    /// 除法，结果转换为 double（可能损失精度）
    auto div_to_double(const BigInt& other) const -> Result<double, String>;

    /// 快速幂：base^exp，exp 为非负整数
    static auto fast_pow_unsigned(const BigInt& base, const BigInt& exp) -> Result<BigInt, String>;

    /// 左移乘以 BASE^k（即乘以 10^{9k}）
    auto shift_left(size_t k) const -> BigInt;
};

} // namespace ks