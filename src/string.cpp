#include "string.hpp"
#include <cstring>
#include <cwchar>
#include <codecvt>
#include <locale>
#include <vector>
#include <sstream>
#include <iomanip>

namespace ks {

// ==================== 构造函数和赋值 ====================

String::String(const char* str) : data_(str ? str : "") {}

String::String(const std::string& str) : data_(str) {}

String::String(std::string&& str) noexcept : data_(std::move(str)) {}

String::String(std::string_view sv) : data_(sv.data(), sv.size()) {}

String::String(SizeType count, char ch) : data_(count, ch) {}

auto String::operator=(const char* str) -> String& {
    data_ = str ? str : "";
    return *this;
}

auto String::operator=(const std::string& str) -> String& {
    data_ = str;
    return *this;
}

// ==================== 转换函数 ====================

auto String::to_std_string() const -> std::string {
    return data_;
}

auto String::view() const -> std::string_view {
    return std::string_view(data_);
}

auto String::c_str() const -> const char* {
    return data_.c_str();
}

// ==================== 访问器 ====================

auto String::operator[](SizeType index) -> char& {
    return data_[index];
}

auto String::operator[](SizeType index) const -> const char& {
    return data_[index];
}

auto String::at(SizeType index) -> Result<char, String> {
    if (index >= data_.size()) {
        return err<char>("index out of range");
    }
    return ok(data_[index]);
}

auto String::at(SizeType index) const -> Result<char, String> {
    if (index >= data_.size()) {
        return err<char>("index out of range");
    }
    return ok(data_[index]);
}

// ==================== 迭代器 ====================

auto String::begin() -> Iterator {
    return data_.begin();
}

auto String::begin() const -> ConstIterator {
    return data_.begin();
}

auto String::end() -> Iterator {
    return data_.end();
}

auto String::end() const -> ConstIterator {
    return data_.end();
}

auto String::cbegin() const -> ConstIterator {
    return data_.cbegin();
}

auto String::cend() const -> ConstIterator {
    return data_.cend();
}

// ==================== 容量 ====================

auto String::len() const -> SizeType {
    return data_.size();
}

auto String::empty() const -> bool {
    return data_.empty();
}

auto String::capacity() const -> SizeType {
    return data_.capacity();
}

auto String::reserve(SizeType new_cap) -> void {
    data_.reserve(new_cap);
}

auto String::shrink_to_fit() -> void {
    data_.shrink_to_fit();
}

// ==================== 修改器 ====================

auto String::clear() -> void {
    data_.clear();
}

auto String::insert(SizeType pos, const String& str) -> String& {
    data_.insert(pos, str.data_);
    return *this;
}

auto String::erase(SizeType pos, SizeType count) -> String& {
    data_.erase(pos, count);
    return *this;
}

auto String::push_back(char ch) -> void {
    data_.push_back(ch);
}

auto String::pop_back() -> void {
    data_.pop_back();
}

auto String::append(const String& str) -> String& {
    data_.append(str.data_);
    return *this;
}

auto String::append(const char* str) -> String& {
    data_.append(str);
    return *this;
}

auto String::operator+=(const String& other) -> String& {
    data_ += other.data_;
    return *this;
}

auto String::operator+=(const char* str) -> String& {
    data_ += str;
    return *this;
}

auto String::operator+=(char ch) -> String& {
    data_ += ch;
    return *this;
}

// ==================== 连接 ====================

auto operator+(const String& lhs, const String& rhs) -> String {
    String result(lhs);
    result += rhs;
    return result;
}

auto operator+(const String& lhs, const char* rhs) -> String {
    String result(lhs);
    result += rhs;
    return result;
}

auto operator+(const char* lhs, const String& rhs) -> String {
    String result(lhs);
    result += rhs;
    return result;
}

// ==================== 比较 ====================

auto String::operator==(const String& other) const -> bool {
    return data_ == other.data_;
}

auto String::operator!=(const String& other) const -> bool {
    return data_ != other.data_;
}

auto String::operator<(const String& other) const -> bool {
    return data_ < other.data_;
}

auto String::operator<=(const String& other) const -> bool {
    return data_ <= other.data_;
}

auto String::operator>(const String& other) const -> bool {
    return data_ > other.data_;
}

auto String::operator>=(const String& other) const -> bool {
    return data_ >= other.data_;
}

// ==================== 查找 / 判断类 ====================

auto String::find(const String& sub) const -> std::int64_t {
    auto pos = data_.find(sub.data_);
    return pos == std::string::npos ? -1 : static_cast<std::int64_t>(pos);
}

auto String::find(const char* sub) const -> std::int64_t {
    auto pos = data_.find(sub);
    return pos == std::string::npos ? -1 : static_cast<std::int64_t>(pos);
}

auto String::rfind(const String& sub) const -> std::int64_t {
    auto pos = data_.rfind(sub.data_);
    return pos == std::string::npos ? -1 : static_cast<std::int64_t>(pos);
}

auto String::rfind(const char* sub) const -> std::int64_t {
    auto pos = data_.rfind(sub);
    return pos == std::string::npos ? -1 : static_cast<std::int64_t>(pos);
}

auto String::index(const String& sub) const -> Result<SizeType, String> {
    auto pos = data_.find(sub.data_);
    if (pos == std::string::npos) {
        return err<SizeType>("substring not found");
    }
    return ok(pos);
}

auto String::index(const char* sub) const -> Result<SizeType, String> {
    auto pos = data_.find(sub);
    if (pos == std::string::npos) {
        return err<SizeType>("substring not found");
    }
    return ok(pos);
}

auto String::rindex(const String& sub) const -> Result<SizeType, String> {
    auto pos = data_.rfind(sub.data_);
    if (pos == std::string::npos) {
        return err<SizeType>("substring not found");
    }
    return ok(pos);
}

auto String::rindex(const char* sub) const -> Result<SizeType, String> {
    auto pos = data_.rfind(sub);
    if (pos == std::string::npos) {
        return err<SizeType>("substring not found");
    }
    return ok(pos);
}

auto String::count(const String& sub) const -> SizeType {
    SizeType count = 0;
    SizeType pos = 0;
    SizeType step = sub.len();
    if (step == 0) return 0;
    while ((pos = data_.find(sub.data_, pos)) != std::string::npos) {
        ++count;
        pos += step;
    }
    return count;
}

auto String::count(const char* sub) const -> SizeType {
    return count(String(sub));
}

auto String::startswith(const String& prefix) const -> bool {
    if (prefix.len() > len()) return false;
    return data_.compare(0, prefix.len(), prefix.data_) == 0;
}

auto String::startswith(const char* prefix) const -> bool {
    return startswith(String(prefix));
}

auto String::endswith(const String& suffix) const -> bool {
    if (suffix.len() > len()) return false;
    return data_.compare(len() - suffix.len(), suffix.len(), suffix.data_) == 0;
}

auto String::endswith(const char* suffix) const -> bool {
    return endswith(String(suffix));
}

auto String::isalpha() const -> bool {
    if (empty()) return false;
    for (char c : data_) {
        if (!is_ascii_alpha(c)) return false;
    }
    return true;
}

auto String::isdigit() const -> bool {
    if (empty()) return false;
    for (char c : data_) {
        if (!is_ascii_digit(c)) return false;
    }
    return true;
}

auto String::isalnum() const -> bool {
    if (empty()) return false;
    for (char c : data_) {
        if (!is_ascii_alnum(c)) return false;
    }
    return true;
}

auto String::islower() const -> bool {
    bool has_lower = false;
    for (char c : data_) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            if (!std::islower(static_cast<unsigned char>(c))) return false;
            has_lower = true;
        }
    }
    return has_lower;
}

auto String::isupper() const -> bool {
    bool has_upper = false;
    for (char c : data_) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            if (!std::isupper(static_cast<unsigned char>(c))) return false;
            has_upper = true;
        }
    }
    return has_upper;
}

auto String::isspace() const -> bool {
    if (empty()) return false;
    for (char c : data_) {
        if (!is_whitespace(c)) return false;
    }
    return true;
}

auto String::istitle() const -> bool {
    bool in_word = false;
    for (char c : data_) {
        if (is_ascii_alpha(c)) {
            if (in_word) {
                if (!std::islower(static_cast<unsigned char>(c))) return false;
            } else {
                if (!std::isupper(static_cast<unsigned char>(c))) return false;
                in_word = true;
            }
        } else {
            in_word = false;
        }
    }
    return true;
}

// ==================== 大小写转换 ====================

auto String::lower() const -> String {
    std::string result;
    result.reserve(len());
    for (char c : data_) {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return String(std::move(result));
}

auto String::upper() const -> String {
    std::string result;
    result.reserve(len());
    for (char c : data_) {
        result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    return String(std::move(result));
}

auto String::capitalize() const -> String {
    if (empty()) return *this;
    std::string result = data_;
    result[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[0])));
    for (SizeType i = 1; i < result.size(); ++i) {
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    }
    return String(std::move(result));
}

auto String::title() const -> String {
    std::string result;
    result.reserve(len());
    bool new_word = true;
    for (char c : data_) {
        if (is_ascii_alpha(c)) {
            if (new_word) {
                result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
                new_word = false;
            } else {
                result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            }
        } else {
            result.push_back(c);
            new_word = true;
        }
    }
    return String(std::move(result));
}

auto String::swapcase() const -> String {
    std::string result;
    result.reserve(len());
    for (char c : data_) {
        if (std::isupper(static_cast<unsigned char>(c))) {
            result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        } else if (std::islower(static_cast<unsigned char>(c))) {
            result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
        } else {
            result.push_back(c);
        }
    }
    return String(std::move(result));
}

// ==================== 修剪 / 填充 / 对齐 ====================

auto String::is_whitespace(char c) -> bool {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

auto String::trim_left(const String& chars) const -> String {
    SizeType start = 0;
    while (start < len() && chars.find(data_[start]) != -1) {
        ++start;
    }
    return substr(start);
}

auto String::trim_right(const String& chars) const -> String {
    SizeType end = len();
    while (end > 0 && chars.find(data_[end-1]) != -1) {
        --end;
    }
    return substr(0, end);
}

auto String::strip() const -> String {
    return strip(String(" \t\n\r\f\v"));
}

auto String::strip(const String& chars) const -> String {
    return trim_left(chars).trim_right(chars);
}

auto String::lstrip() const -> String {
    return lstrip(String(" \t\n\r\f\v"));
}

auto String::lstrip(const String& chars) const -> String {
    return trim_left(chars);
}

auto String::rstrip() const -> String {
    return rstrip(String(" \t\n\r\f\v"));
}

auto String::rstrip(const String& chars) const -> String {
    return trim_right(chars);
}

auto String::center(SizeType width) const -> String {
    return center(width, ' ');
}

auto String::center(SizeType width, char fillchar) const -> String {
    if (width <= len()) return *this;
    SizeType total_pad = width - len();
    SizeType left_pad = total_pad / 2;
    SizeType right_pad = total_pad - left_pad;
    return String(left_pad, fillchar) + *this + String(right_pad, fillchar);
}

auto String::ljust(SizeType width) const -> String {
    return ljust(width, ' ');
}

auto String::ljust(SizeType width, char fillchar) const -> String {
    if (width <= len()) return *this;
    return *this + String(width - len(), fillchar);
}

auto String::rjust(SizeType width) const -> String {
    return rjust(width, ' ');
}

auto String::rjust(SizeType width, char fillchar) const -> String {
    if (width <= len()) return *this;
    return String(width - len(), fillchar) + *this;
}

auto String::zfill(SizeType width) const -> String {
    if (width <= len()) return *this;
    bool negative = !empty() && data_[0] == '-';
    SizeType pad = width - len();
    if (negative) {
        return String("-") + String(pad, '0') + substr(1);
    } else {
        return String(pad, '0') + *this;
    }
}

// ==================== 分割 / 连接 ====================

auto String::split() const -> std::vector<String> {
    std::vector<String> result;
    SizeType start = 0;
    SizeType end = 0;
    while (end < len()) {
        // 跳过空白
        while (end < len() && is_whitespace(data_[end])) ++end;
        if (end >= len()) break;
        start = end;
        // 找到非空白结束
        while (end < len() && !is_whitespace(data_[end])) ++end;
        result.push_back(substr(start, end - start));
    }
    return result;
}

auto String::split(const String& sep) const -> std::vector<String> {
    std::vector<String> result;
    if (sep.empty()) {
        // 按字符分割？按照 Python 行为，空分隔符会引发 ValueError，但我们返回每个字符
        for (char c : data_) {
            result.push_back(String(1, c));
        }
        return result;
    }
    SizeType pos = 0;
    SizeType found;
    while ((found = data_.find(sep.data_, pos)) != std::string::npos) {
        result.push_back(substr(pos, found - pos));
        pos = found + sep.len();
    }
    result.push_back(substr(pos));
    return result;
}

auto String::split(const char* sep) const -> std::vector<String> {
    return split(String(sep));
}

auto String::rsplit() const -> std::vector<String> {
    // 与 split 相同，默认按空白分割
    return split();
}

auto String::rsplit(const String& sep) const -> std::vector<String> {
    std::vector<String> result;
    if (sep.empty()) {
        for (char c : data_) {
            result.push_back(String(1, c));
        }
        return result;
    }
    SizeType pos = len();
    SizeType found;
    while ((found = data_.rfind(sep.data_, pos - 1)) != std::string::npos) {
        result.insert(result.begin(), substr(found + sep.len(), pos - found - sep.len()));
        pos = found;
    }
    result.insert(result.begin(), substr(0, pos));
    return result;
}

auto String::rsplit(const char* sep) const -> std::vector<String> {
    return rsplit(String(sep));
}

auto String::splitlines(bool keepends) const -> std::vector<String> {
    std::vector<String> result;
    SizeType start = 0;
    SizeType end = 0;
    while (end < len()) {
        if (data_[end] == '\n' || data_[end] == '\r') {
            SizeType line_end = end;
            // 处理 \r\n
            if (data_[end] == '\r' && end + 1 < len() && data_[end + 1] == '\n') {
                if (keepends) {
                    result.push_back(substr(start, end + 2 - start));
                } else {
                    result.push_back(substr(start, end - start));
                }
                end += 2;
            } else {
                if (keepends) {
                    result.push_back(substr(start, end + 1 - start));
                } else {
                    result.push_back(substr(start, end - start));
                }
                ++end;
            }
            start = end;
        } else {
            ++end;
        }
    }
    if (start < len()) {
        result.push_back(substr(start));
    }
    return result;
}

auto String::partition(const String& sep) const -> std::vector<String> {
    std::vector<String> result;
    auto pos = data_.find(sep.data_);
    if (pos == std::string::npos) {
        result.push_back(*this);
        result.push_back(String());
        result.push_back(String());
    } else {
        result.push_back(substr(0, pos));
        result.push_back(sep);
        result.push_back(substr(pos + sep.len()));
    }
    return result;
}

auto String::partition(const char* sep) const -> std::vector<String> {
    return partition(String(sep));
}

auto String::rpartition(const String& sep) const -> std::vector<String> {
    std::vector<String> result;
    auto pos = data_.rfind(sep.data_);
    if (pos == std::string::npos) {
        result.push_back(String());
        result.push_back(String());
        result.push_back(*this);
    } else {
        result.push_back(substr(0, pos));
        result.push_back(sep);
        result.push_back(substr(pos + sep.len()));
    }
    return result;
}

auto String::rpartition(const char* sep) const -> std::vector<String> {
    return rpartition(String(sep));
}

auto String::join(const std::vector<String>& strings, const String& sep) -> String {
    String result;
    for (SizeType i = 0; i < strings.size(); ++i) {
        if (i > 0) result += sep;
        result += strings[i];
    }
    return result;
}

auto String::join(const std::vector<std::string>& strings, const String& sep) -> String {
    String result;
    for (SizeType i = 0; i < strings.size(); ++i) {
        if (i > 0) result += sep;
        result += strings[i];
    }
    return result;
}

auto String::join(const std::vector<const char*>& strings, const String& sep) -> String {
    String result;
    for (SizeType i = 0; i < strings.size(); ++i) {
        if (i > 0) result += sep;
        result += strings[i];
    }
    return result;
}

// ==================== 替换 ====================

auto String::replace(const String& old, const String& new_str) const -> String {
    if (old.empty()) return *this;
    std::string result;
    SizeType pos = 0;
    SizeType found;
    SizeType old_len = old.len();
    while ((found = data_.find(old.data_, pos)) != std::string::npos) {
        result.append(data_, pos, found - pos);
        result.append(new_str.data_);
        pos = found + old_len;
    }
    result.append(data_, pos);
    return String(std::move(result));
}

auto String::replace(const char* old, const char* new_str) const -> String {
    return replace(String(old), String(new_str));
}

auto String::expandtabs(SizeType tabsize) const -> String {
    std::string result;
    SizeType column = 0;
    for (char c : data_) {
        if (c == '\t') {
            SizeType spaces = tabsize - (column % tabsize);
            result.append(spaces, ' ');
            column += spaces;
        } else {
            result.push_back(c);
            if (c == '\n' || c == '\r') {
                column = 0;
            } else {
                ++column;
            }
        }
    }
    return String(std::move(result));
}

// ==================== 编码 / 解码 ====================

auto String::encode(const char* encoding) const -> std::vector<std::uint8_t> {
    // 简化：只处理 UTF-8，直接返回字节
    if (std::strcmp(encoding, "utf-8") == 0 || std::strcmp(encoding, "UTF-8") == 0) {
        std::vector<std::uint8_t> bytes(data_.begin(), data_.end());
        return bytes;
    }
    // 其他编码暂不支持
    return std::vector<std::uint8_t>();
}

auto String::decode(const std::vector<std::uint8_t>& bytes, const char* encoding) -> Result<String, String> {
    if (std::strcmp(encoding, "utf-8") == 0 || std::strcmp(encoding, "UTF-8") == 0) {
        // 简单的 UTF-8 有效性检查（略）
        std::string str(bytes.begin(), bytes.end());
        return ok(String(std::move(str)));
    }
    return err<String>("unsupported encoding");
}

// ==================== 格式化 ====================

auto String::format(const std::vector<String>& args) const -> String {
    std::string result;
    SizeType last = 0;
    SizeType i = 0;
    while (i < len() - 1) {
        if (data_[i] == '{' && data_[i+1] == '}') {
            result.append(data_, last, i - last);
            if (!args.empty()) {
                result.append(args[0].data_);  // 简化：只使用第一个参数
            }
            i += 2;
            last = i;
        } else {
            ++i;
        }
    }
    result.append(data_, last);
    return String(std::move(result));
}

// ==================== 其他实用 ====================

auto String::substr(SizeType pos, SizeType count) const -> String {
    if (pos > len()) return String();
    if (count == npos || pos + count > len()) {
        count = len() - pos;
    }
    return String(data_.substr(pos, count));
}

auto String::reverse() const -> String {
    std::string result(data_.rbegin(), data_.rend());
    return String(std::move(result));
}

auto String::repeat(SizeType times) const -> String {
    std::string result;
    result.reserve(len() * times);
    for (SizeType i = 0; i < times; ++i) {
        result += data_;
    }
    return String(std::move(result));
}

auto operator*(const String& str, String::SizeType times) -> String {
    return str.repeat(times);
}

auto operator*(String::SizeType times, const String& str) -> String {
    return str.repeat(times);
}

auto String::to_int() const -> Result<std::int64_t, String> {
    char* endptr;
    errno = 0;
    long long val = std::strtoll(data_.c_str(), &endptr, 10);
    if (errno == ERANGE || val > INT64_MAX || val < INT64_MIN) {
        return err<std::int64_t>("integer out of range");
    }
    if (endptr == data_.c_str() || *endptr != '\0') {
        return err<std::int64_t>("invalid integer format");
    }
    return ok(static_cast<std::int64_t>(val));
}

auto String::to_float() const -> Result<double, String> {
    char* endptr;
    errno = 0;
    double val = std::strtod(data_.c_str(), &endptr);
    if (errno == ERANGE) {
        return err<double>("float out of range");
    }
    if (endptr == data_.c_str() || *endptr != '\0') {
        return err<double>("invalid float format");
    }
    return ok(val);
}

// ==================== 静态辅助函数 ====================

auto String::is_ascii_alpha(char c) -> bool {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

auto String::is_ascii_digit(char c) -> bool {
    return c >= '0' && c <= '9';
}

auto String::is_ascii_alnum(char c) -> bool {
    return is_ascii_alpha(c) || is_ascii_digit(c);
}

} // namespace ks