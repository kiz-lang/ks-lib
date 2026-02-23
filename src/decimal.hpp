#pragma once

#include "bigint.hpp"
#include "result.hpp"
#include "string.hpp"
#include "check.hpp"
#include <string>
#include <cstdint>

namespace ks {

/// 高精度十进制小数，使用尾数 * 10^指数表示
class Decimal {
public:
    // ========== 构造函数 ==========

    /// 默认构造为 0
    Decimal() : mantissa_(0), exponent_(0) {}

    /// 从 BigInt 构造（指数为 0）
    explicit Decimal(const BigInt& mantissa);

    /// 从内置整数构造
    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    explicit Decimal(T val) : Decimal(BigInt(val)) {}

    /// 从 C 字符串解析
    static auto from_string(const char* str) -> Result<Decimal, String>;

    /// 从 std::string 解析
    static auto from_string(const std::string& str) -> Result<Decimal, String> {
        return from_string(str.c_str());
    }

    /// 从 ks::String 解析
    static auto from_string(const String& str) -> Result<Decimal, String> {
        return from_string(str.c_str());
    }

    // ========== 拷贝/移动 ==========
    Decimal(const Decimal& other) = default;
    Decimal(Decimal&& other) noexcept = default;
    auto operator=(const Decimal& other) -> Decimal& = default;
    auto operator=(Decimal&& other) noexcept -> Decimal& = default;

    // ========== 基本访问 ==========

    /// 返回绝对值
    auto abs() const -> Decimal;

    /// 返回整数部分（截断小数部分）
    auto integer_part() const -> BigInt;

    /// 判断是否为零
    auto is_zero() const -> bool;

    /// 转换为十进制字符串
    auto to_string() const -> std::string;

    /// 转换为 ks::String
    auto to_ks_string() const -> String;

    /// 哈希值（用于字典）
    auto hash() const -> BigInt;

    // ========== 比较运算符 ==========

    auto operator==(const Decimal& other) const -> bool;
    auto operator!=(const Decimal& other) const -> bool;
    auto operator<(const Decimal& other) const -> bool;
    auto operator<=(const Decimal& other) const -> bool;
    auto operator>(const Decimal& other) const -> bool;
    auto operator>=(const Decimal& other) const -> bool;

    // ========== 算术运算符 ==========

    auto operator+() const -> const Decimal& { return *this; }
    auto operator-() const -> Decimal;

    auto operator+(const Decimal& other) const -> Decimal;
    auto operator-(const Decimal& other) const -> Decimal;
    auto operator*(const Decimal& other) const -> Decimal;
    auto operator/(const Decimal& other) const -> Decimal;  // 默认保留10位小数

    /// 指定小数位数的除法
    auto div(const Decimal& other, int n) const -> Decimal;

    auto operator+=(const Decimal& other) -> Decimal&;
    auto operator-=(const Decimal& other) -> Decimal&;
    auto operator*=(const Decimal& other) -> Decimal&;
    auto operator/=(const Decimal& other) -> Decimal&;

    // ========== 与 BigInt 的混合运算 ==========

    auto operator+(const BigInt& other) const -> Decimal;
    auto operator-(const BigInt& other) const -> Decimal;
    auto operator*(const BigInt& other) const -> Decimal;
    auto operator/(const BigInt& other) const -> Decimal;

    // ========== 其他实用方法 ==========

    /// 幂运算（指数为非负整数）
    auto pow(const BigInt& exponent) const -> Result<Decimal, String>;

    /// 带舍入的除法（四舍五入到指定小数位数）
    auto div_round(const Decimal& other, int n = 10) const -> Decimal;

    /// 比较小数部分到指定位数是否相等
    auto decimal_weekeq(const Decimal& other, int n) const -> bool;

private:
    BigInt mantissa_;   // 尾数（包含符号，归一化后无末尾零）
    int exponent_;      // 指数：value = mantissa_ * 10^exponent_

    /// 私有构造函数，直接设置尾数和指数（不自动归一化）
    Decimal(const BigInt& mant, int exp) : mantissa_(mant), exponent_(exp) {}

    /// 归一化：去除尾数末尾的零，调整指数
    void normalize();

    /// 比较绝对值：返回 *this 的绝对值是否小于 other 的绝对值
    auto abs_less(const Decimal& other) const -> bool;

    /// 对齐两个 Decimal 的指数到较小者，返回公共指数，并输出对齐后的尾数
    static auto align_exponent(const Decimal& a, const Decimal& b,
                               BigInt& a_mant, BigInt& b_mant) -> int;
};

// ========== 全局运算符（BigInt 与 Decimal） ==========

inline auto operator+(const BigInt& lhs, const Decimal& rhs) -> Decimal {
    return rhs + lhs;
}

inline auto operator-(const BigInt& lhs, const Decimal& rhs) -> Decimal {
    return Decimal(lhs) - rhs;
}

inline auto operator*(const BigInt& lhs, const Decimal& rhs) -> Decimal {
    return rhs * lhs;
}

inline auto operator/(const BigInt& lhs, const Decimal& rhs) -> Decimal {
    return Decimal(lhs) / rhs;
}

} // namespace ks