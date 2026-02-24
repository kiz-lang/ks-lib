#include "decimal.hpp"
#include <cctype>
#include <string>
#include <algorithm>

namespace ks {

// ========== 私有辅助函数 ==========

void Decimal::normalize() {
    if (mantissa_.is_zero()) {
        exponent_ = 0;
        return;
    }
    BigInt ten(10);
    while (mantissa_ % ten == BigInt(0)) {
        mantissa_ = mantissa_ / ten;
        exponent_ += 1;
    }
}

auto Decimal::abs_less(const Decimal& other) const -> bool {
    // 比较绝对值
    BigInt a_mant, b_mant;
    int exp = align_exponent(*this, other, a_mant, b_mant);
    return a_mant.abs() < b_mant.abs();
}

auto Decimal::align_exponent(const Decimal& a, const Decimal& b,
                             BigInt& a_mant, BigInt& b_mant) -> int {
    int common_exp = std::min(a.exponent_, b.exponent_);
    int a_scale = a.exponent_ - common_exp;
    
    int b_scale = b.exponent_ - common_exp;

    BigInt ten(10);
    a_mant = a.mantissa_;
    if (a_scale > 0) {
         a_mant = a_mant * BigInt::fast_pow_unsigned(ten, BigInt(a_scale)).value(); // 假设成功
    }
    b_mant = b.mantissa_;
    if (b_scale > 0) {
        b_mant = b_mant * BigInt::fast_pow_unsigned(ten, BigInt(b_scale)).value();
    }
    return common_exp;
}

// ========== 构造函数 ==========

Decimal::Decimal(const BigInt& mantissa) : mantissa_(mantissa), exponent_(0) {
    normalize();
}

auto Decimal::from_string(const char* str) -> Result<Decimal, String> {
    if (!str || *str == '\0') {
        return err<Decimal>("empty string");
    }

    const char* p = str;
    bool negative = false;
    if (*p == '-') {
        negative = true;
        ++p;
        if (*p == '\0') return err<Decimal>("sign only");
    } else if (*p == '+') {
        ++p;
        if (*p == '\0') return err<Decimal>("sign only");
    }

    // 查找指数部分 e/E
    const char* exp_start = nullptr;
    const char* end = p;
    while (*end) {
        if (*end == 'e' || *end == 'E') {
            exp_start = end;
            break;
        }
        ++end;
    }

    std::string mant_str;
    std::string exp_str;

    if (exp_start) {
        mant_str.assign(p, exp_start - p);
        exp_str = exp_start + 1;
        // 检查指数部分
        if (exp_str.empty()) return err<Decimal>("exponent missing");
        bool exp_neg = false;
        size_t exp_i = 0;
        if (exp_str[0] == '-') {
            exp_neg = true;
            exp_i = 1;
        } else if (exp_str[0] == '+') {
            exp_i = 1;
        }
        if (exp_i >= exp_str.size()) return err<Decimal>("exponent sign only");
        for (; exp_i < exp_str.size(); ++exp_i) {
            if (!std::isdigit(exp_str[exp_i]))
                return err<Decimal>("invalid exponent digit");
        }
    } else {
        mant_str = p;
    }

    // 解析尾数部分
    if (mant_str.empty()) return err<Decimal>("no digits");

    size_t dot_pos = mant_str.find('.');
    std::string int_part, frac_part;

    if (dot_pos != std::string::npos) {
        // 不能有多个小数点
        if (mant_str.find('.', dot_pos + 1) != std::string::npos)
            return err<Decimal>("multiple decimal points");

        int_part = mant_str.substr(0, dot_pos);
        frac_part = mant_str.substr(dot_pos + 1);

        // 整数部分可以为空（如 ".123"）
        for (char c : int_part) {
            if (!std::isdigit(c)) return err<Decimal>("invalid integer digit");
        }
        if (frac_part.empty()) return err<Decimal>("decimal point without fractional digits");
        for (char c : frac_part) {
            if (!std::isdigit(c)) return err<Decimal>("invalid fractional digit");
        }
    } else {
        // 无小数点
        int_part = mant_str;
        for (char c : int_part) {
            if (!std::isdigit(c)) return err<Decimal>("invalid digit");
        }
    }

    // 解析指数
    int exp_val = 0;
    if (!exp_str.empty()) {
        char* endptr;
        long e = std::strtol(exp_str.c_str(), &endptr, 10);
        if (*endptr != '\0') return err<Decimal>("invalid exponent");
        exp_val = static_cast<int>(e);
    }

    // 构造尾数
    BigInt mant;
    if (dot_pos == std::string::npos) {
        mant = BigInt::from_string(int_part).value();
    } else {
        std::string combined = (int_part.empty() ? "0" : int_part) + frac_part;
        mant = BigInt::from_string(combined).value();
        exp_val -= static_cast<int>(frac_part.size());
    }

    if (negative) {
        mant = -mant;
    }

    Decimal result(mant, exp_val);
    result.normalize();
    return ok(result);
}

// ========== 基本访问 ==========

auto Decimal::abs() const -> Decimal {
    Decimal res = *this;
    res.mantissa_ = res.mantissa_.abs();
    return res;
}

auto Decimal::integer_part() const -> BigInt {
    if (exponent_ >= 0) {
        // 尾数 * 10^exponent
        BigInt ten(10);
        return mantissa_ * BigInt::fast_pow_unsigned(ten, BigInt(exponent_)).value();
    } else {
        // 尾数 / 10^(-exponent)
        BigInt ten(10);
        BigInt divisor = BigInt::fast_pow_unsigned(ten, BigInt(-exponent_)).value();
        return mantissa_ / divisor;
    }
}

auto Decimal::is_zero() const -> bool {
    return mantissa_.is_zero();
}

auto Decimal::to_string() const -> std::string {
    if (mantissa_.is_zero()) return "0";

    std::string mant_str = mantissa_.abs().to_string();
    bool neg = mantissa_.sign() < 0;
    int exp = exponent_;

    std::string res;
    if (exp >= 0) {
        // 指数非负：尾数后加 exp 个零
        res = mant_str + std::string(exp, '0');
    } else {
        // 指数为负：需要插入小数点
        int abs_exp = -exp;
        if (abs_exp >= static_cast<int>(mant_str.size())) {
            // 小数点在最前面，前面补零
            res = "0." + std::string(abs_exp - mant_str.size(), '0') + mant_str;
        } else {
            // 小数点在中间
            size_t split = mant_str.size() - abs_exp;
            res = mant_str.substr(0, split) + "." + mant_str.substr(split);
        }
    }

    // 移除末尾多余的零和小数点
    if (res.find('.') != std::string::npos) {
        res.erase(res.find_last_not_of('0') + 1, std::string::npos);
        if (res.back() == '.') res.pop_back();
    }

    if (neg && res != "0") {
        res = "-" + res;
    }
    return res;
}

auto Decimal::to_ks_string() const -> String {
    return String(to_string());
}

auto Decimal::hash() const -> BigInt {
    // 简单的哈希：将 mantissa 的哈希和 exponent 组合
    // 使用 BigInt 作为哈希值
    std::hash<std::string> hasher;
    size_t mant_hash = hasher(mantissa_.to_string());
    size_t exp_hash = hasher(std::to_string(exponent_));

    BigInt mant_big(static_cast<uint64_t>(mant_hash));
    BigInt exp_big(static_cast<uint64_t>(exp_hash));
    return mant_big * BigInt(static_cast<uint64_t>(1ULL << 32)) + exp_big;
}

// ========== 比较运算符 ==========

auto Decimal::operator==(const Decimal& other) const -> bool {
    if (exponent_ == other.exponent_) {
        return mantissa_ == other.mantissa_;
    }
    BigInt a_mant, b_mant;
    align_exponent(*this, other, a_mant, b_mant);
    return a_mant == b_mant;
}

auto Decimal::operator!=(const Decimal& other) const -> bool {
    return !(*this == other);
}

auto Decimal::operator<(const Decimal& other) const -> bool {
    BigInt a_mant, b_mant;
    align_exponent(*this, other, a_mant, b_mant);
    return a_mant < b_mant;
}

auto Decimal::operator<=(const Decimal& other) const -> bool {
    return *this < other || *this == other;
}

auto Decimal::operator>(const Decimal& other) const -> bool {
    return other < *this;
}

auto Decimal::operator>=(const Decimal& other) const -> bool {
    return !(*this < other);
}

// ========== 算术运算符 ==========

auto Decimal::operator-() const -> Decimal {
    Decimal res = *this;
    res.mantissa_ = -res.mantissa_;
    return res;
}

auto Decimal::operator+(const Decimal& other) const -> Decimal {
    BigInt a_mant, b_mant;
    int exp = align_exponent(*this, other, a_mant, b_mant);
    BigInt sum = a_mant + b_mant;
    Decimal res(sum, exp);
    res.normalize();
    return res;
}

auto Decimal::operator-(const Decimal& other) const -> Decimal {
    BigInt a_mant, b_mant;
    int exp = align_exponent(*this, other, a_mant, b_mant);
    BigInt diff = a_mant - b_mant;
    Decimal res(diff, exp);
    res.normalize();
    return res;
}

auto Decimal::operator*(const Decimal& other) const -> Decimal {
    BigInt prod = mantissa_ * other.mantissa_;
    Decimal res(prod, exponent_ + other.exponent_);
    res.normalize();
    return res;
}

auto Decimal::operator/(const Decimal& other) const -> Decimal {
    return div(other, 10);
}

auto Decimal::div(const Decimal& other, int n) const -> Decimal {
    if (other.is_zero()) {
        check(false, "division by zero in Decimal");
    }
    if (n < 0) {
        check(false, "negative precision in Decimal::div");
    }

    BigInt a_mant, b_mant;
    align_exponent(*this, other, a_mant, b_mant);  // a_mant / b_mant = 原值

    // 扩展被除数以保留 n 位小数
    BigInt ten(10);
    BigInt scale = BigInt::fast_pow_unsigned(ten, BigInt(n)).value();  // 10^n
    BigInt dividend = a_mant * scale;
    BigInt quotient = dividend / b_mant;  // 整数除法

    Decimal res(quotient, -n);
    res.normalize();
    return res;
}

auto Decimal::operator+=(const Decimal& other) -> Decimal& {
    *this = *this + other;
    return *this;
}

auto Decimal::operator-=(const Decimal& other) -> Decimal& {
    *this = *this - other;
    return *this;
}

auto Decimal::operator*=(const Decimal& other) -> Decimal& {
    *this = *this * other;
    return *this;
}

auto Decimal::operator/=(const Decimal& other) -> Decimal& {
    *this = *this / other;
    return *this;
}

// ========== 与 BigInt 的混合运算 ==========

auto Decimal::operator+(const BigInt& other) const -> Decimal {
    return *this + Decimal(other);
}

auto Decimal::operator-(const BigInt& other) const -> Decimal {
    return *this - Decimal(other);
}

auto Decimal::operator*(const BigInt& other) const -> Decimal {
    return *this * Decimal(other);
}

auto Decimal::operator/(const BigInt& other) const -> Decimal {
    return *this / Decimal(other);
}

// ========== 其他实用方法 ==========

auto Decimal::pow(const BigInt& exponent) const -> Result<Decimal, String> {
    if (exponent.sign() < 0) {
        return err<Decimal>("negative exponent not supported");
    }
    if (exponent.is_zero()) {
        return ok(Decimal(BigInt(1)));
    }
    // 尾数部分进行幂运算
    auto mant_pow_res = mantissa_.abs().pow(exponent);

    auto mant_pow_res = mantissa_.abs().pow(exponent);
    if (mant_pow_res.is_err()) {
        return err<Decimal>(mant_pow_res.error()); // 直接返回错误
    }
    BigInt mant_pow = mant_pow_res.value();

    // 确定符号：底数为负且指数为奇数时结果为负
    if (mantissa_.sign() < 0 && (exponent % BigInt(2) == BigInt(1))) {
        mant_pow = -mant_pow;
    }

    // 指数部分：exponent_ * exponent
    // exponent_ 是 int，需要转换为 BigInt 后相乘，再转回 int（可能溢出）
    BigInt exp_big(exponent_);
    BigInt new_exp_big = exp_big * exponent;
    // 检查是否超出 int 范围
    if (new_exp_big > BigInt(std::numeric_limits<int>::max()) ||
        new_exp_big < BigInt(std::numeric_limits<int>::min())) {
        return err<Decimal>("exponent overflow in Decimal::pow");
    }
    int new_exp = static_cast<int>(new_exp_big.to_int64().value());

    Decimal result(mant_pow, new_exp);
    result.normalize();
    return ok(result);
}

auto Decimal::div_round(const Decimal& other, int n) const -> Decimal {
    if (other.is_zero()) {
        check(false, "division by zero in Decimal::div_round");
    }
    if (n < 0) {
        check(false, "negative precision in Decimal::div_round");
    }

    // 计算到 n+1 位小数
    Decimal temp = div(other, n + 1);
    std::string str = temp.to_string();

    // 找到小数点
    size_t dot_pos = str.find('.');
    if (dot_pos == std::string::npos) {
        return temp;  // 没有小数部分，直接返回
    }

    std::string int_part = str.substr(0, dot_pos);
    std::string frac_part = str.substr(dot_pos + 1);

    // 确保有 n+1 位小数
    if (frac_part.size() < static_cast<size_t>(n + 1)) {
        frac_part.append(n + 1 - frac_part.size(), '0');
    }

    // 检查第 n+1 位
    if (frac_part[n] >= '5') {
        // 需要进位
        std::string new_frac = (n > 0) ? frac_part.substr(0, n) : "";
        bool carry = true;
        // 对前 n 位进行进位
        for (int i = n - 1; i >= 0 && carry; --i) {
            if (new_frac[i] == '9') {
                new_frac[i] = '0';
            } else {
                new_frac[i] += 1;
                carry = false;
            }
        }

        // 处理整数部分的进位
        BigInt int_val = BigInt::from_string(int_part).value();
        if (carry) {
            if (int_part[0] == '-') {
                int_val = int_val - BigInt(1);
            } else {
                int_val = int_val + BigInt(1);
            }
        }
        int_part = int_val.to_string();

        // 构建结果字符串
        std::string result_str = int_part;
        if (n > 0 && new_frac != std::string(n, '0')) {
            result_str += "." + new_frac;
        }
        return Decimal::from_string(result_str).value();
    } else {
        // 直接截断
        std::string result_str = int_part;
        if (n > 0) {
            result_str += "." + frac_part.substr(0, n);
        }
        return Decimal::from_string(result_str).value();
    }
}

auto Decimal::decimal_weekeq(const Decimal& other, int n) const -> bool {
    if (n < 0) return false;
    if (*this == other) return true;

    BigInt this_int = this->integer_part();
    BigInt other_int = other.integer_part();
    if (this_int != other_int) return false;

    // 计算 10^n
    BigInt ten(10);
    BigInt scale = BigInt::fast_pow_unsigned(ten, BigInt(n)).value();

    // 分离小数部分
    Decimal this_frac = *this - Decimal(this_int);
    Decimal other_frac = other - Decimal(other_int);

    // 缩放后取整
    BigInt this_scaled = (this_frac * Decimal(scale)).integer_part();
    BigInt other_scaled = (other_frac * Decimal(scale)).integer_part();

    return this_scaled == other_scaled;
}

} // namespace ks