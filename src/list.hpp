#pragma once

#include "result.hpp"
#include "string.hpp"
#include <vector>
#include <algorithm>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <cstddef>

namespace ks {

namespace detail {

// 自定义分配器，在分配失败时调用 check 终止程序，避免抛出异常
template<typename T>
struct CheckAllocator {
    using value_type = T;

    CheckAllocator() = default;
    template<typename U> CheckAllocator(const CheckAllocator<U>&) noexcept {}

    auto allocate(std::size_t n) -> T* {
        T* ptr = (T*)::operator new(n * sizeof(T), std::nothrow);
        check(ptr != nullptr, "memory allocation failed in ks::List");
        return ptr;
    }

    auto deallocate(T* ptr, std::size_t) noexcept -> void {
        ::operator delete(ptr);
    }

    // 用于比较分配器是否相等（总是相等）
    template<typename U>
    auto operator==(const CheckAllocator<U>&) const noexcept -> bool { return true; }
    template<typename U>
    auto operator!=(const CheckAllocator<U>&) const noexcept -> bool { return false; }
};

// 将任意类型转换为 ks::String 的辅助函数（用于 join）
template<typename T>
auto to_string_for_join(const T& value) -> String {
    if constexpr (std::is_arithmetic_v<T>) {
        return String(std::to_string(value));
    } else if constexpr (std::is_same_v<T, String>) {
        return value;
    } else if constexpr (std::is_same_v<T, std::string>) {
        return String(value);
    } else if constexpr (std::is_same_v<T, const char*>) {
        return String(value);
    } else if constexpr (std::is_same_v<T, char>) {
        return String(1, value);
    } else {
        // 其他类型尝试使用输出流
        std::ostringstream oss;
        oss << value;
        return String(oss.str());
    }
}

} // namespace detail

template<typename T>
class List {
public:
    using ValueType = T;
    using SizeType = std::size_t;
    using Iterator = typename std::vector<T, detail::CheckAllocator<T>>::iterator;
    using ConstIterator = typename std::vector<T, detail::CheckAllocator<T>>::const_iterator;
    using ReverseIterator = typename std::vector<T, detail::CheckAllocator<T>>::reverse_iterator;
    using ConstReverseIterator = typename std::vector<T, detail::CheckAllocator<T>>::const_reverse_iterator;

    // 构造函数
    List() = default;
    explicit List(SizeType count, const T& value = T()) : data_(count, value) {}
    List(std::initializer_list<T> init) : data_(init) {}
    template<typename InputIt>
    List(InputIt first, InputIt last) : data_(first, last) {}
    List(const List& other) = default;
    List(List&& other) noexcept = default;

    // 赋值操作
    auto operator=(const List& other) -> List& = default;
    auto operator=(List&& other) noexcept -> List& = default;
    auto operator=(std::initializer_list<T> init) -> List& {
        data_ = init;
        return *this;
    }

    // 元素访问
    auto at(SizeType pos) -> Result<T&, String> {
        if (pos >= size()) {
            return err<T&>("list index out of range");
        }
        return ok(data_[pos]);
    }

    auto at(SizeType pos) const -> Result<const T&, String> {
        if (pos >= size()) {
            return err<const T&>("list index out of range");
        }
        return ok(data_[pos]);
    }

    auto operator[](SizeType pos) -> T& {
        return data_[pos];
    }

    auto operator[](SizeType pos) const -> const T& {
        return data_[pos];
    }

    auto front() -> T& {
        check(!empty(), "list::front(): list is empty");
        return data_.front();
    }

    auto front() const -> const T& {
        check(!empty(), "list::front(): list is empty");
        return data_.front();
    }

    auto back() -> T& {
        check(!empty(), "list::back(): list is empty");
        return data_.back();
    }

    auto back() const -> const T& {
        check(!empty(), "list::back(): list is empty");
        return data_.back();
    }

    // 迭代器
    auto begin() -> Iterator { return data_.begin(); }
    auto begin() const -> ConstIterator { return data_.begin(); }
    auto cbegin() const -> ConstIterator { return data_.cbegin(); }
    auto end() -> Iterator { return data_.end(); }
    auto end() const -> ConstIterator { return data_.end(); }
    auto cend() const -> ConstIterator { return data_.cend(); }
    auto rbegin() -> ReverseIterator { return data_.rbegin(); }
    auto rbegin() const -> ConstReverseIterator { return data_.rbegin(); }
    auto crbegin() const -> ConstReverseIterator { return data_.crbegin(); }
    auto rend() -> ReverseIterator { return data_.rend(); }
    auto rend() const -> ConstReverseIterator { return data_.rend(); }
    auto crend() const -> ConstReverseIterator { return data_.crend(); }

    // 容量
    auto empty() const noexcept -> bool {
        return data_.empty();
    }

    auto size() const noexcept -> SizeType {
        return data_.size();
    }

    auto capacity() const noexcept -> SizeType {
        return data_.capacity();
    }

    auto reserve(SizeType new_cap) -> void {
        data_.reserve(new_cap);
    }

    auto shrink_to_fit() -> void {
        data_.shrink_to_fit();
    }

    // 修改器

    /// 在末尾添加一个元素
    auto append(const T& value) -> void {
        data_.push_back(value);
    }

    auto append(T&& value) -> void {
        data_.push_back(std::move(value));
    }

    /// 在末尾添加多个元素（可迭代对象）
    template<typename Iterable>
    auto extend(const Iterable& iterable) -> void {
        for (const auto& item : iterable) {
            data_.push_back(item);
        }
    }

    template<typename Iterable>
    auto extend(Iterable&& iterable) -> void {
        for (auto&& item : iterable) {
            data_.push_back(std::forward<decltype(item)>(item));
        }
    }

    /// 在指定位置插入元素
    auto insert(SizeType pos, const T& value) -> void {
        check(pos <= size(), "insert: position out of range");
        data_.insert(data_.begin() + pos, value);
    }

    auto insert(SizeType pos, T&& value) -> void {
        check(pos <= size(), "insert: position out of range");
        data_.insert(data_.begin() + pos, std::move(value));
    }

    /// 删除第一个值为 x 的元素，如果不存在返回错误
    auto remove(const T& value) -> Result<void, String> {
        auto it = std::find(data_.begin(), data_.end(), value);
        if (it == data_.end()) {
            return err<void>("remove: value not found");
        }
        data_.erase(it);
        return ok();
    }

    /// 删除并返回最后一个元素，如果列表为空返回错误
    auto pop() -> Result<T, String> {
        if (empty()) {
            return err<T>("pop: list is empty");
        }
        T value = std::move(data_.back());
        data_.pop_back();
        return ok(std::move(value));
    }

    /// 删除并返回索引 i 处的元素，如果索引无效返回错误
    auto pop(SizeType i) -> Result<T, String> {
        if (i >= size()) {
            return err<T>("pop: index out of range");
        }
        T value = std::move(data_[i]);
        data_.erase(data_.begin() + i);
        return ok(std::move(value));
    }

    /// 清空列表
    auto clear() -> void {
        data_.clear();
    }

    /// 查找第一个值为 x 的索引，返回 Result
    auto index(const T& x, SizeType start = 0, SizeType end = npos) const -> Result<SizeType, String> {
        SizeType actual_end = (end == npos) ? size() : end;
        if (start > size() || actual_end > size() || start >= actual_end) {
            return err<SizeType>("index: invalid range");
        }
        for (SizeType i = start; i < actual_end; ++i) {
            if (data_[i] == x) {
                return ok(i);
            }
        }
        return err<SizeType>("index: value not found in range");
    }

    /// 统计 x 出现次数
    auto count(const T& x) const -> SizeType {
        return std::count(data_.begin(), data_.end(), x);
    }

    /// 原地排序
    auto sort(bool reverse = false) -> void {
        if (reverse) {
            std::sort(data_.begin(), data_.end(), std::greater<T>());
        } else {
            std::sort(data_.begin(), data_.end());
        }
    }

    /// 使用自定义比较函数排序
    template<typename Compare>
    auto sort(Compare comp, bool reverse = false) -> void {
        if (reverse) {
            std::sort(data_.begin(), data_.end(), [comp](const T& a, const T& b) {
                return comp(b, a);
            });
        } else {
            std::sort(data_.begin(), data_.end(), comp);
        }
    }

    /// 原地反转
    auto reverse() -> void {
        std::reverse(data_.begin(), data_.end());
    }

    /// 返回新列表（浅拷贝）
    auto copy() const -> List {
        return *this;
    }

    /// 返回新排序列表
    auto sorted(bool reverse = false) const -> List {
        List result = *this;
        result.sort(reverse);
        return result;
    }

    template<typename Compare>
    auto sorted(Compare comp, bool reverse = false) const -> List {
        List result = *this;
        result.sort(comp, reverse);
        return result;
    }

    /// 返回反转迭代器（实际上返回一个新的反向迭代器范围，但为了 Python 风格，返回新的列表？）
    /// 这里按照 Python 的 reversed 返回一个迭代器，我们返回一个代理对象？为了简单，返回新列表。
    auto reversed() const -> List {
        List result;
        result.data_.reserve(size());
        for (auto it = data_.rbegin(); it != data_.rend(); ++it) {
            result.data_.push_back(*it);
        }
        return result;
    }

    /// 用分隔符连接元素的字符串表示，返回 ks::String
    auto join(const String& sep) const -> String {
        String result;
        for (SizeType i = 0; i < size(); ++i) {
            if (i > 0) {
                result += sep;
            }
            result += detail::to_string_for_join(data_[i]);
        }
        return result;
    }

    /// 判断是否为空
    auto isempty() const -> bool {
        return empty();
    }

    /// 获取长度
    auto len() const -> SizeType {
        return size();
    }

    /// 静态常量 npos
    static const SizeType npos = static_cast<SizeType>(-1);

private:
    std::vector<T, detail::CheckAllocator<T>> data_;
};

// 非成员函数

/// 求最小值（列表不能为空）
template<typename T>
auto min(const List<T>& lst) -> Result<T, String> {
    if (lst.empty()) {
        return err<T>("min(): list is empty");
    }
    auto it = std::min_element(lst.begin(), lst.end());
    return ok(*it);
}

/// 求最大值
template<typename T>
auto max(const List<T>& lst) -> Result<T, String> {
    if (lst.empty()) {
        return err<T>("max(): list is empty");
    }
    auto it = std::max_element(lst.begin(), lst.end());
    return ok(*it);
}

/// 求和（要求 T 支持加法，默认从 T{} 开始）
template<typename T>
auto sum(const List<T>& lst, T init = T{}) -> T {
    T result = init;
    for (const auto& v : lst) {
        result = result + v;
    }
    return result;
}

} // namespace ks