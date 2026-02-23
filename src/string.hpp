#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cctype>
#include <string_view>
#include <initializer_list>
#include <sstream>

#include "result.hpp"

namespace ks {

class String {
public:
    // 类型定义
    using SizeType = std::size_t;
    using Iterator = std::string::iterator;
    using ConstIterator = std::string::const_iterator;

    // 构造函数
    String() = default;
    String(const char* str);
    String(const std::string& str);
    String(std::string&& str) noexcept;
    String(const String& other) = default;
    String(String&& other) noexcept = default;
    
    // 从字符串视图构造
    explicit String(std::string_view sv);
    
    // 从重复字符构造
    String(SizeType count, char ch);
    
    // 赋值操作
    auto operator=(const String& other) -> String& = default;
    auto operator=(String&& other) noexcept -> String& = default;
    auto operator=(const char* str) -> String&;
    auto operator=(const std::string& str) -> String&;
    
    // 转换为 std::string
    auto to_std_string() const -> std::string;
    
    // 转换为字符串视图
    auto view() const -> std::string_view;
    
    // 转换为 C 字符串
    auto c_str() const -> const char*;
    
    // 访问器
    auto operator[](SizeType index) -> char&;
    auto operator[](SizeType index) const -> const char&;
    auto at(SizeType index) -> Result<char, String>;
    auto at(SizeType index) const -> Result<char, String>;
    
    // 迭代器
    auto begin() -> Iterator;
    auto begin() const -> ConstIterator;
    auto end() -> Iterator;
    auto end() const -> ConstIterator;
    auto cbegin() const -> ConstIterator;
    auto cend() const -> ConstIterator;
    
    // 容量
    auto len() const -> SizeType;
    auto empty() const -> bool;
    auto capacity() const -> SizeType;
    auto reserve(SizeType new_cap) -> void;
    auto shrink_to_fit() -> void;
    
    // 修改器
    auto clear() -> void;
    auto insert(SizeType pos, const String& str) -> String&;
    auto erase(SizeType pos, SizeType count = 1) -> String&;
    auto push_back(char ch) -> void;
    auto pop_back() -> void;
    auto append(const String& str) -> String&;
    auto append(const char* str) -> String&;
    auto operator+=(const String& other) -> String&;
    auto operator+=(const char* str) -> String&;
    auto operator+=(char ch) -> String&;
    
    // 连接
    friend auto operator+(const String& lhs, const String& rhs) -> String;
    friend auto operator+(const String& lhs, const char* rhs) -> String;
    friend auto operator+(const char* lhs, const String& rhs) -> String;
    
    // 比较
    auto operator==(const String& other) const -> bool;
    auto operator!=(const String& other) const -> bool;
    auto operator<(const String& other) const -> bool;
    auto operator<=(const String& other) const -> bool;
    auto operator>(const String& other) const -> bool;
    auto operator>=(const String& other) const -> bool;
    
    // ---------- 查找 / 判断类 ----------
    
    /// 查找子串，返回索引，没找到返回 -1
    auto find(const String& sub) const -> std::int64_t;
    auto find(const char* sub) const -> std::int64_t;
    
    /// 从右查找子串
    auto rfind(const String& sub) const -> std::int64_t;
    auto rfind(const char* sub) const -> std::int64_t;
    
    /// 查找索引，返回 Result
    auto index(const String& sub) const -> Result<SizeType, String>;
    auto index(const char* sub) const -> Result<SizeType, String>;
    
    /// 从右查找索引
    auto rindex(const String& sub) const -> Result<SizeType, String>;
    auto rindex(const char* sub) const -> Result<SizeType, String>;
    
    /// 子串出现次数
    auto count(const String& sub) const -> SizeType;
    auto count(const char* sub) const -> SizeType;
    
    /// 是否以...开头
    auto startswith(const String& prefix) const -> bool;
    auto startswith(const char* prefix) const -> bool;
    
    /// 是否以...结尾
    auto endswith(const String& suffix) const -> bool;
    auto endswith(const char* suffix) const -> bool;
    
    /// 是否全字母
    auto isalpha() const -> bool;
    
    /// 是否全数字
    auto isdigit() const -> bool;
    
    /// 是否字母/数字
    auto isalnum() const -> bool;
    
    /// 是否全小写
    auto islower() const -> bool;
    
    /// 是否全大写
    auto isupper() const -> bool;
    
    /// 是否空白字符
    auto isspace() const -> bool;
    
    /// 是否标题格式（每个单词首字母大写）
    auto istitle() const -> bool;
    
    // ---------- 大小写转换 ----------
    
    /// 全小写
    auto lower() const -> String;
    
    /// 全大写
    auto upper() const -> String;
    
    /// 首字母大写，其余小写
    auto capitalize() const -> String;
    
    /// 每个单词首字母大写
    auto title() const -> String;
    
    /// 大小写互换
    auto swapcase() const -> String;
    
    // ---------- 修剪 / 填充 / 对齐 ----------
    
    /// 去两边空白
    auto strip() const -> String;
    auto strip(const String& chars) const -> String;
    
    /// 去左边空白
    auto lstrip() const -> String;
    auto lstrip(const String& chars) const -> String;
    
    /// 去右边空白
    auto rstrip() const -> String;
    auto rstrip(const String& chars) const -> String;
    
    /// 居中
    auto center(SizeType width) const -> String;
    auto center(SizeType width, char fillchar) const -> String;
    
    /// 左对齐
    auto ljust(SizeType width) const -> String;
    auto ljust(SizeType width, char fillchar) const -> String;
    
    /// 右对齐
    auto rjust(SizeType width) const -> String;
    auto rjust(SizeType width, char fillchar) const -> String;
    
    /// 前面补0
    auto zfill(SizeType width) const -> String;
    
    // ---------- 分割 / 连接 ----------
    
    /// 按分隔符拆成列表
    auto split() const -> std::vector<String>;  // 默认按空白分割
    auto split(const String& sep) const -> std::vector<String>;
    auto split(const char* sep) const -> std::vector<String>;
    
    /// 从右拆
    auto rsplit() const -> std::vector<String>;
    auto rsplit(const String& sep) const -> std::vector<String>;
    auto rsplit(const char* sep) const -> std::vector<String>;
    
    /// 按换行拆
    auto splitlines(bool keepends = false) const -> std::vector<String>;
    
    /// 分成 (前, sep, 后) 三元组
    auto partition(const String& sep) const -> std::vector<String>;
    auto partition(const char* sep) const -> std::vector<String>;
    
    /// 从右 partition
    auto rpartition(const String& sep) const -> std::vector<String>;
    auto rpartition(const char* sep) const -> std::vector<String>;
    
    /// 用 sep 连接可迭代对象
    static auto join(const std::vector<String>& strings, const String& sep) -> String;
    static auto join(const std::vector<std::string>& strings, const String& sep) -> String;
    static auto join(const std::vector<const char*>& strings, const String& sep) -> String;
    
    // ---------- 替换 ----------
    
    /// 替换
    auto replace(const String& old, const String& new_str) const -> String;
    auto replace(const char* old, const char* new_str) const -> String;
    
    /// 替换 tab 为空格
    auto expandtabs(SizeType tabsize = 8) const -> String;
    
    // ---------- 编码 / 解码 ----------
    
    /// 转 bytes
    auto encode(const char* encoding = "utf-8") const -> std::vector<std::uint8_t>;
    
    /// bytes 转 str
    static auto decode(const std::vector<std::uint8_t>& bytes, const char* encoding = "utf-8") -> Result<String, String>;
    
    // ---------- 格式化 ----------
    
    /// 简单格式化（仅支持 {} 占位符）
    auto format(const std::vector<String>& args) const -> String;
    
    /// 可变参数模板格式化
    template<typename... Args>
    auto format(Args&&... args) const -> String {
        std::vector<String> vec;
        (vec.push_back(String(to_string(std::forward<Args>(args)))), ...);
        return format(vec);
    }
    
    // ---------- 其他实用 ----------
    
    /// 获取子串
    auto substr(SizeType pos = 0, SizeType count = npos) const -> String;
    
    /// 反转
    auto reverse() const -> String;
    
    /// 重复
    auto repeat(SizeType times) const -> String;
    friend auto operator*(const String& str, SizeType times) -> String;
    friend auto operator*(SizeType times, const String& str) -> String;
    
    /// 转换为整数
    auto to_int() const -> Result<std::int64_t, String>;
    
    /// 转换为浮点数
    auto to_float() const -> Result<double, String>;
    
    /// 静态常量
    static const SizeType npos = static_cast<SizeType>(-1);
    
private:
    std::string data_;
    
    /// 辅助函数：将任意类型转换为字符串（用于format）
    template<typename T>
    static auto to_string(T&& value) -> std::string {
        if constexpr (std::is_arithmetic_v<std::decay_t<T>>) {
            return std::to_string(std::forward<T>(value));
        } else if constexpr (std::is_same_v<std::decay_t<T>, String>) {
            return std::forward<T>(value).data_;
        } else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            return std::forward<T>(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, const char*>) {
            return std::string(std::forward<T>(value));
        } else {
            // 默认尝试使用流输出
            std::ostringstream oss;
            oss << std::forward<T>(value);
            return oss.str();
        }
    }
    
    /// 检查字符是否为空白
    static auto is_whitespace(char c) -> bool;
    
    /// 检查字符是否为字母
    static auto is_ascii_alpha(char c) -> bool;
    
    /// 检查字符是否为数字
    static auto is_ascii_digit(char c) -> bool;
    
    /// 检查字符是否为字母数字
    static auto is_ascii_alnum(char c) -> bool;
    
    /// 修剪辅助函数
    auto trim_left(const String& chars) const -> String;
    auto trim_right(const String& chars) const -> String;
};

} // namespace ks