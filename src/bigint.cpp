#include "bigint.hpp"
#include <cctype>
#include <limits>
#include <stdexcept>  // 仅用于辅助，不使用异常

namespace ks {

// ========== 私有辅助函数 ==========

void BigInt::trim() {
    while (data_.size() > 1 && data_.back() == 0) {
        data_.pop_back();
    }
    if (data_.empty()) {
        data_.push_back(0);
        negative_ = false;
    }
    if (data_.size() == 1 && data_[0] == 0) {
        negative_ = false;
    }
}

auto BigInt::abs_less(const BigInt& other) const -> bool {
    if (data_.size() != other.data_.size()) {
        return data_.size() < other.data_.size();
    }
    for (auto i = data_.size(); i-- > 0; ) {
        if (data_[i] != other.data_[i]) {
            return data_[i] < other.data_[i];
        }
    }
    return false;  // 相等
}

auto BigInt::unsigned_add(const BigInt& a, const BigInt& b) -> BigInt {
    BigInt result;
    result.data_.clear();
    result.negative_ = false;

    DoubleDigit carry = 0;
    size_t max_len = std::max(a.data_.size(), b.data_.size());
    for (size_t i = 0; i < max_len || carry; ++i) {
        DoubleDigit da = (i < a.data_.size()) ? a.data_[i] : 0;
        DoubleDigit db = (i < b.data_.size()) ? b.data_[i] : 0;
        DoubleDigit sum = da + db + carry;
        carry = sum / BASE;
        result.data_.push_back(static_cast<Digit>(sum % BASE));
    }
    result.trim();
    return result;
}

auto BigInt::unsigned_sub(const BigInt& a, const BigInt& b) -> BigInt {
    // 要求 |a| >= |b|
    BigInt result;
    result.data_.clear();
    result.negative_ = false;

    DoubleDigit borrow = 0;
    for (size_t i = 0; i < a.data_.size(); ++i) {
        DoubleDigit da = a.data_[i];
        DoubleDigit db = (i < b.data_.size()) ? b.data_[i] : 0;
        DoubleDigit diff = da - db - borrow;
        if (diff > da) {  // 借位导致下溢
            diff += BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result.data_.push_back(static_cast<Digit>(diff));
    }
    result.trim();
    return result;
}

auto BigInt::unsigned_mul(const BigInt& a, const BigInt& b) -> BigInt {
    // O(n*m) 乘法
    BigInt result;
    result.data_.assign(a.data_.size() + b.data_.size(), 0);
    result.negative_ = false;

    for (size_t i = 0; i < a.data_.size(); ++i) {
        DoubleDigit carry = 0;
        for (size_t j = 0; j < b.data_.size(); ++j) {
            DoubleDigit product = static_cast<DoubleDigit>(a.data_[i]) * b.data_[j] +
                                  result.data_[i + j] + carry;
            result.data_[i + j] = static_cast<Digit>(product % BASE);
            carry = product / BASE;
        }
        if (carry) {
            result.data_[i + b.data_.size()] += static_cast<Digit>(carry);
        }
    }
    result.trim();
    return result;
}

auto BigInt::unsigned_div_mod(const BigInt& a, const BigInt& b) -> std::pair<BigInt, BigInt> {
    // 要求 b 非零
    check(!b.is_zero(), "division by zero in unsigned_div_mod");

    // 特殊情况：被除数小于除数
    if (a.abs_less(b)) {
        return {BigInt(0), a};
    }

    // 复制被除数和除数（绝对值）
    std::vector<Digit> u = a.data_;  // 被除数，低位在前
    std::vector<Digit> v = b.data_;  // 除数，低位在前
    size_t m = u.size() - 1;          // 被除数长度-1
    size_t n = v.size();              // 除数长度

    // 归一化：使 v[n-1] >= BASE/2
    Digit d = BASE / (v.back() + 1);
    if (d > 1) {
        // 缩放 u 和 v
        DoubleDigit carry = 0;
        for (size_t i = 0; i < u.size(); ++i) {
            carry += static_cast<DoubleDigit>(u[i]) * d;
            u[i] = static_cast<Digit>(carry % BASE);
            carry /= BASE;
        }
        if (carry) u.push_back(static_cast<Digit>(carry));
        carry = 0;
        for (size_t i = 0; i < v.size(); ++i) {
            carry += static_cast<DoubleDigit>(v[i]) * d;
            v[i] = static_cast<Digit>(carry % BASE);
            carry /= BASE;
        }
        // v 的长度可能增加，但归一化后 v[n-1] 会变大，但不应超过 BASE-1
        // 确保 v 长度不变（因为 d 最大可能使 v[n-1] 变大但不会进位）
    }

    // 现在 v[n-1] >= BASE/2

    // 商 q 的长度为 m - n + 1 或 m - n + 2，但通常为 m - n + 1
    BigInt quotient;
    quotient.data_.resize(m - n + 1, 0);
    quotient.negative_ = false;

    // 主循环：对每个 j = m-n .. 0
    for (size_t j = static_cast<size_t>(m - n + 1); j-- > 0; ) {  // j 从 m-n 递减到 0
        // 试商
        DoubleDigit qhat;
        if (u[j + n] >= v.back()) {
            qhat = BASE - 1;
        } else {
            qhat = (static_cast<DoubleDigit>(u[j + n]) * BASE + u[j + n - 1]) / v.back();
        }

        // 调整 qhat
        while (qhat * static_cast<DoubleDigit>(v[n-2]) >
               ( (static_cast<DoubleDigit>(u[j + n]) * BASE + u[j + n - 1] - qhat * v.back()) * BASE + u[j + n - 2] )) {
            --qhat;
        }

        // 乘法和减法
        DoubleDigit carry = 0;
        DoubleDigit borrow = 0;
        for (size_t i = 0; i < n; ++i) {
            DoubleDigit prod = qhat * v[i] + carry;
            carry = prod / BASE;
            Digit prod_digit = static_cast<Digit>(prod % BASE);
            DoubleDigit sub = static_cast<DoubleDigit>(u[j + i]) - prod_digit - borrow;
            if (sub > u[j + i]) { // 借位
                sub += BASE;
                borrow = 1;
            } else {
                borrow = 0;
            }
            u[j + i] = static_cast<Digit>(sub);
        }
        // 处理最高位
        DoubleDigit final_sub = static_cast<DoubleDigit>(u[j + n]) - carry - borrow;
        if (final_sub > u[j + n]) {
            final_sub += BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }
        u[j + n] = static_cast<Digit>(final_sub);

        // 如果借位为 1，说明 qhat 过大，需要加回（减过头了）
        if (borrow) {
            --qhat;
            carry = 0;
            for (size_t i = 0; i < n; ++i) {
                DoubleDigit sum = static_cast<DoubleDigit>(u[j + i]) + v[i] + carry;
                u[j + i] = static_cast<Digit>(sum % BASE);
                carry = sum / BASE;
            }
            u[j + n] += static_cast<Digit>(carry);
        }

        quotient.data_[j] = static_cast<Digit>(qhat);
    }

    // 去除商的前导零
    quotient.trim();

    // 余数需要反缩放（除以 d）
    BigInt remainder;
    remainder.data_.assign(u.begin(), u.begin() + n);
    remainder.trim();
    if (d > 1) {
        // 除以 d
        DoubleDigit carry = 0;
        for (size_t i = remainder.data_.size(); i-- > 0; ) {
            DoubleDigit cur = carry * BASE + remainder.data_[i];
            remainder.data_[i] = static_cast<Digit>(cur / d);
            carry = cur % d;
        }
        remainder.trim();
    }

    return {quotient, remainder};
}

auto BigInt::div_to_double(const Bigint& other) const -> Result<double, String> {
    if (other.is_zero()) {
        return err<double>("division by zero in div_to_double");
    }
    // 将被除数和除数转换为 double（注意溢出）
    // 由于 double 只有 53 位精度，我们只取高位部分
    // 简单做法：先转换为字符串再转 double（效率低但可靠）
    // 或者使用科学计数法估算
    double a = std::strtod(this->to_string().c_str(), nullptr);
    double b = std::strtod(other.to_string().c_str(), nullptr);
    if (b == 0.0) return err<double>("division by zero");
    double result = a / b;
    // 处理符号
    if (this->negative_ != other.negative_) result = -result;
    return ok(result);
}

auto BigInt::fast_pow_unsigned(const Bigint& base, const Bigint& exp) -> Result<Bigint, String> {
    if (exp.sign() < 0) {
        return err<Bigint>("negative exponent not allowed in fast_pow_unsigned");
    }
    if (exp.is_zero()) {
        return ok(Bigint(1));
    }
    Bigint result(1);
    Bigint cur_base = base;
    Bigint cur_exp = exp;
    while (!cur_exp.is_zero()) {
        if (cur_exp.data_[0] & 1) { // 奇数
            result = result * cur_base;
        }
        cur_base = cur_base * cur_base;
        cur_exp = cur_exp / Bigint(2); // 使用除法
    }
    return ok(result);
}

auto BigInt::shift_left(size_t k) const -> BigInt {
    if (k == 0 || is_zero()) return *this;
    BigInt result;
    result.data_.resize(data_.size() + k, 0);
    std::copy(data_.begin(), data_.end(), result.data_.begin());
    result.negative_ = negative_;
    return result;
}

// ========== 公有成员函数 ==========

auto BigInt::from_string(const char* str) -> Result<BigInt, String> {
    if (!str || *str == '\0') {
        return err<BigInt>("empty string");
    }
    const char* p = str;
    bool negative = false;
    if (*p == '-') {
        negative = true;
        ++p;
        if (*p == '\0') {
            return err<BigInt>("missing digits after minus sign");
        }
    }
    // 跳过前导零
    while (*p == '0') ++p;
    if (*p == '\0') {  // 全是零
        return ok(BigInt(0));
    }

    BigInt result;
    result.data_.clear();
    result.negative_ = negative;
    // 从字符串末尾开始每9位一组转换为一个digit
    const char* end = p;
    while (*end) ++end;  // 找到末尾
    const char* cur = end;
    while (cur > p) {
        const char* start = (cur - 9 > p) ? cur - 9 : p;
        uint32_t val = 0;
        for (const char* q = start; q < cur; ++q) {
            if (*q < '0' || *q > '9') {
                return err<BigInt>("invalid digit");
            }
            val = val * 10 + (*q - '0');
        }
        result.data_.push_back(val);
        cur = start;
    }
    result.trim();
    return ok(std::move(result));
}

auto BigInt::operator==(const BigInt& other) const -> bool {
    if (negative_ != other.negative_ || data_.size() != other.data_.size()) return false;
    return data_ == other.data_;
}

auto BigInt::operator!=(const BigInt& other) const -> bool {
    return !(*this == other);
}

auto BigInt::operator<(const BigInt& other) const -> bool {
    if (negative_ != other.negative_) {
        return negative_;  // 负 < 正
    }
    if (data_.size() != other.data_.size()) {
        return negative_ ? (data_.size() > other.data_.size()) : (data_.size() < other.data_.size());
    }
    for (size_t i = data_.size(); i-- > 0; ) {
        if (data_[i] != other.data_[i]) {
            return negative_ ? (data_[i] > other.data_[i]) : (data_[i] < other.data_[i]);
        }
    }
    return false;  // 相等
}

auto BigInt::operator<=(const BigInt& other) const -> bool {
    return *this < other || *this == other;
}

auto BigInt::operator>(const BigInt& other) const -> bool {
    return other < *this;
}

auto BigInt::operator>=(const BigInt& other) const -> bool {
    return !(*this < other);
}

auto BigInt::operator-() const -> BigInt {
    BigInt result = *this;
    if (!result.is_zero()) {
        result.negative_ = !result.negative_;
    }
    return result;
}

auto BigInt::operator+(const BigInt& other) const -> BigInt {
    if (negative_ == other.negative_) {
        // 同号：绝对值相加，符号不变
        BigInt result = unsigned_add(*this, other);
        result.negative_ = negative_;
        return result;
    } else {
        // 异号：转化为绝对值减法
        if (abs_less(other)) {
            BigInt result = unsigned_sub(other, *this);
            result.negative_ = other.negative_;
            return result;
        } else {
            BigInt result = unsigned_sub(*this, other);
            result.negative_ = negative_;
            return result;
        }
    }
}

auto BigInt::operator-(const BigInt& other) const -> BigInt {
    return *this + (-other);
}

auto BigInt::operator*(const BigInt& other) const -> BigInt {
    if (is_zero() || other.is_zero()) return BigInt(0);
    BigInt result = unsigned_mul(*this, other);
    result.negative_ = negative_ ^ other.negative_;
    return result;
}

auto BigInt::operator/(const BigInt& other) const -> BigInt {
    if (other.is_zero()) {
        check(false, "division by zero");
    }
    auto [q, _] = unsigned_div_mod(this->abs(), other.abs());
    q.negative_ = negative_ ^ other.negative_;
    q.trim();
    return q;
}

auto BigInt::operator%(const BigInt& other) const -> BigInt {
    if (other.is_zero()) {
        check(false, "modulo by zero");
    }
    auto [_, r] = unsigned_div_mod(this->abs(), other.abs());
    // 调整余数符号与被除数一致
    if (negative_ && !r.is_zero()) {
        r = other.abs() - r;
    }
    r.negative_ = negative_;
    r.trim();
    return r;
}

auto BigInt::operator+=(const BigInt& other) -> BigInt& {
    *this = *this + other;
    return *this;
}

auto BigInt::operator-=(const BigInt& other) -> BigInt& {
    *this = *this - other;
    return *this;
}

auto BigInt::operator*=(const BigInt& other) -> BigInt& {
    *this = *this * other;
    return *this;
}

auto BigInt::operator/=(const BigInt& other) -> BigInt& {
    *this = *this / other;
    return *this;
}

auto BigInt::operator%=(const BigInt& other) -> BigInt& {
    *this = *this % other;
    return *this;
}

auto BigInt::pow(const BigInt& exponent) const -> Result<BigInt, String> {
    if (exponent.negative_) {
        return err<BigInt>("exponent cannot be negative");
    }
    if (exponent.is_zero()) {
        return ok(BigInt(1));
    }
    if (is_zero()) {
        return ok(BigInt(0));
    }
    // 快速幂
    BigInt base = *this;
    BigInt exp = exponent;
    BigInt result(1);
    while (!exp.is_zero()) {
        if (exp.data_[0] & 1) {  // 判断奇数
            result = result * base;
        }
        base = base * base;
        exp = exp / BigInt(2);  // 使用除法
    }
    return ok(result);
}

auto BigInt::abs() const -> BigInt {
    BigInt res = *this;
    res.negative_ = false;
    return res;
}

auto BigInt::sign() const -> int {
    if (is_zero()) return 0;
    return negative_ ? -1 : 1;
}

auto BigInt::is_zero() const -> bool {
    return data_.size() == 1 && data_[0] == 0;
}

auto BigInt::to_string() const -> std::string {
    if (is_zero()) return "0";
    std::string s;
    if (negative_) s.push_back('-');
    // 最高位（最高 digit）需要完整输出，不需要前导零
    s += std::to_string(data_.back());
    for (size_t i = data_.size() - 1; i-- > 0; ) {
        std::string part = std::to_string(data_[i]);
        // 前面补零到9位
        if (part.size() < DIGIT_BITS) {
            s.append(DIGIT_BITS - part.size(), '0');
        }
        s += part;
    }
    return s;
}

auto BigInt::to_ks_string() const -> String {
    return String(to_string());
}

auto BigInt::to_uint64() const -> Result<uint64_t, String> {
    if (negative_) {
        return err<uint64_t>("negative value cannot be converted to uint64_t");
    }
    if (*this > BigInt(std::numeric_limits<uint64_t>::max())) {
        return err<uint64_t>("value exceeds uint64_t max");
    }
    uint64_t val = 0;
    for (size_t i = data_.size(); i-- > 0; ) {
        val = val * BASE + data_[i];
    }
    return ok(val);
}

auto BigInt::to_int64() const -> Result<int64_t, String> {
    uint64_t abs_val;
    if (negative_) {
        if (this->abs() > BigInt(static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1)) {
            return err<int64_t>("value exceeds int64_t range");
        }
        abs_val = this->abs().to_uint64().value();
        if (abs_val == static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1) {
            return ok(std::numeric_limits<int64_t>::min());
        }
        return ok(-static_cast<int64_t>(abs_val));
    } else {
        if (*this > BigInt(std::numeric_limits<int64_t>::max())) {
            return err<int64_t>("value exceeds int64_t max");
        }
        auto res = to_uint64();
        if (res.is_err()) {
            return err<int64_t>(res.error()); // 直接返回错误
        }
    }
}

// ========== 输入输出 ==========

auto operator<<(std::ostream& os, const BigInt& num) -> std::ostream& {
    os << num.to_string();
    return os;
}

auto operator>>(std::istream& is, BigInt& num) -> std::istream& {
    std::string s;
    is >> s;
    auto res = BigInt::from_string(s);
    if (res.is_err()) {
        is.setstate(std::ios::failbit);
    } else {
        num = res.value();
    }
    return is;
}

} // namespace ks