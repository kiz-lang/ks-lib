#pragma once

#include "string.hpp"
#include "result.hpp"
#include <vector>
#include <cstddef>
#include <functional>
#include <utility>
#include <iterator>

namespace ks {

// 前向声明
template<typename T>
class Dict;

namespace detail {

/// 槽位状态
enum class SlotState { Empty, Occupied, Deleted };

/// 哈希表条目
template<typename T>
struct Entry {
    String key;
    T value;
    SlotState state;

    Entry() : state(SlotState::Empty) {}
    Entry(const String& k, const T& v) : key(k), value(v), state(SlotState::Occupied) {}
    Entry(String&& k, T&& v) : key(std::move(k)), value(std::move(v)), state(SlotState::Occupied) {}
};

/// 计算字符串哈希值
inline auto hash_string(const String& s) -> std::size_t {
    return std::hash<std::string>{}(s.to_std_string());
}

/// 探测函数：线性探测
inline auto next_probe(std::size_t index, std::size_t i, std::size_t capacity) -> std::size_t {
    return (index + i) % capacity;  // 线性探测：index + i
}

} // namespace detail

// ========== Dict 迭代器（跳过空和已删除） ==========
template<typename T>
class DictIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename std::vector<detail::Entry<T>>::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    DictIterator(typename std::vector<detail::Entry<T>>::iterator it,
                 typename std::vector<detail::Entry<T>>::iterator end)
        : it_(it), end_(end) {
        skip_invalid();
    }

    auto operator*() -> reference { return *it_; }
    auto operator->() -> pointer { return &(*it_); }

    auto operator++() -> DictIterator& {
        ++it_;
        skip_invalid();
        return *this;
    }

    auto operator++(int) -> DictIterator {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    friend auto operator==(const DictIterator& a, const DictIterator& b) -> bool {
        return a.it_ == b.it_;
    }

    friend auto operator!=(const DictIterator& a, const DictIterator& b) -> bool {
        return !(a == b);
    }

private:
    typename std::vector<detail::Entry<T>>::iterator it_;
    typename std::vector<detail::Entry<T>>::iterator end_;

    void skip_invalid() {
        while (it_ != end_ && (it_->state != detail::SlotState::Occupied)) {
            ++it_;
        }
    }
};

template<typename T>
class DictConstIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const typename std::vector<detail::Entry<T>>::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    DictConstIterator(typename std::vector<detail::Entry<T>>::const_iterator it,
                      typename std::vector<detail::Entry<T>>::const_iterator end)
        : it_(it), end_(end) {
        skip_invalid();
    }

    auto operator*() -> reference { return *it_; }
    auto operator->() -> pointer { return &(*it_); }

    auto operator++() -> DictConstIterator& {
        ++it_;
        skip_invalid();
        return *this;
    }

    auto operator++(int) -> DictConstIterator {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    friend auto operator==(const DictConstIterator& a, const DictConstIterator& b) -> bool {
        return a.it_ == b.it_;
    }

    friend auto operator!=(const DictConstIterator& a, const DictConstIterator& b) -> bool {
        return !(a == b);
    }

private:
    typename std::vector<detail::Entry<T>>::const_iterator it_;
    typename std::vector<detail::Entry<T>>::const_iterator end_;

    void skip_invalid() {
        while (it_ != end_ && (it_->state != detail::SlotState::Occupied)) {
            ++it_;
        }
    }
};

// ========== 视图类 ==========
template<typename T>
class KeysView {
public:
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const String;
        using difference_type = std::ptrdiff_t;
        using pointer = const String*;
        using reference = const String&;

        Iterator(DictConstIterator<T> it) : it_(it) {}

        auto operator*() -> reference { return it_->key; }
        auto operator->() -> pointer { return &it_->key; }

        auto operator++() -> Iterator& {
            ++it_;
            return *this;
        }

        auto operator++(int) -> Iterator {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        friend auto operator==(const Iterator& a, const Iterator& b) -> bool {
            return a.it_ == b.it_;
        }

        friend auto operator!=(const Iterator& a, const Iterator& b) -> bool {
            return !(a == b);
        }

    private:
        DictConstIterator<T> it_;
    };

    KeysView(const Dict<T>* dict) : dict_(dict) {}

    auto begin() const -> Iterator { return Iterator(dict_->begin()); }
    auto end() const -> Iterator { return Iterator(dict_->end()); }

private:
    const Dict<T>* dict_;
};

template<typename T>
class ValuesView {
public:
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        Iterator(DictConstIterator<T> it) : it_(it) {}

        auto operator*() -> reference { return it_->value; }
        auto operator->() -> pointer { return &it_->value; }

        auto operator++() -> Iterator& {
            ++it_;
            return *this;
        }

        auto operator++(int) -> Iterator {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        friend auto operator==(const Iterator& a, const Iterator& b) -> bool {
            return a.it_ == b.it_;
        }

        friend auto operator!=(const Iterator& a, const Iterator& b) -> bool {
            return !(a == b);
        }

    private:
        DictConstIterator<T> it_;
    };

    ValuesView(const Dict<T>* dict) : dict_(dict) {}

    auto begin() const -> Iterator { return Iterator(dict_->begin()); }
    auto end() const -> Iterator { return Iterator(dict_->end()); }

private:
    const Dict<T>* dict_;
};

template<typename T>
class ItemsView {
public:
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<const String&, const T&>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type;

        Iterator(DictConstIterator<T> it) : it_(it) {}

        auto operator*() -> reference {
            return {it_->key, it_->value};
        }

        auto operator++() -> Iterator& {
            ++it_;
            return *this;
        }

        auto operator++(int) -> Iterator {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        friend auto operator==(const Iterator& a, const Iterator& b) -> bool {
            return a.it_ == b.it_;
        }

        friend auto operator!=(const Iterator& a, const Iterator& b) -> bool {
            return !(a == b);
        }

    private:
        DictConstIterator<T> it_;
    };

    ItemsView(const Dict<T>* dict) : dict_(dict) {}

    auto begin() const -> Iterator { return Iterator(dict_->begin()); }
    auto end() const -> Iterator { return Iterator(dict_->end()); }

private:
    const Dict<T>* dict_;
};

// ========== Dict 主类 ==========
template<typename T>
class Dict {
public:
    using SizeType = std::size_t;
    using Iterator = DictIterator<T>;
    using ConstIterator = DictConstIterator<T>;

    /// 默认构造，初始容量为 16
    Dict() : entries_(16), size_(0), deleted_count_(0) {}

    /// 拷贝构造
    Dict(const Dict& other) : entries_(other.entries_), size_(other.size_), deleted_count_(other.deleted_count_) {}

    /// 移动构造
    Dict(Dict&& other) noexcept
        : entries_(std::move(other.entries_)),
          size_(other.size_),
          deleted_count_(other.deleted_count_) {
        other.size_ = 0;
        other.deleted_count_ = 0;
    }

    /// 从初始化列表构造
    Dict(std::initializer_list<std::pair<const String, T>> init) : Dict() {
        for (const auto& pair : init) {
            insert(pair.first, pair.second);
        }
    }

    /// 赋值操作
    auto operator=(const Dict& other) -> Dict& {
        if (this != &other) {
            entries_ = other.entries_;
            size_ = other.size_;
            deleted_count_ = other.deleted_count_;
        }
        return *this;
    }

    auto operator=(Dict&& other) noexcept -> Dict& {
        if (this != &other) {
            entries_ = std::move(other.entries_);
            size_ = other.size_;
            deleted_count_ = other.deleted_count_;
            other.size_ = 0;
            other.deleted_count_ = 0;
        }
        return *this;
    }

    // ========== 容量 ==========
    auto empty() const -> bool { return size_ == 0; }
    auto size() const -> SizeType { return size_; }
    auto isempty() const -> bool { return empty(); }

    // ========== 迭代器 ==========
    auto begin() -> Iterator {
        return Iterator(entries_.begin(), entries_.end());
    }
    auto begin() const -> ConstIterator {
        return ConstIterator(entries_.cbegin(), entries_.cend());
    }
    auto cbegin() const -> ConstIterator {
        return ConstIterator(entries_.cbegin(), entries_.cend());
    }
    auto end() -> Iterator {
        return Iterator(entries_.end(), entries_.end());
    }
    auto end() const -> ConstIterator {
        return ConstIterator(entries_.cend(), entries_.cend());
    }
    auto cend() const -> ConstIterator {
        return ConstIterator(entries_.cend(), entries_.cend());
    }

    // ========== 访问器 ==========

    /// 获取键对应的值，如果不存在返回错误
    auto get(const String& key) const -> Result<const T&, String> {
        auto idx = find_index(key);
        if (idx && entries_[*idx].state == detail::SlotState::Occupied) {
            return ok(entries_[*idx].value);
        }
        return err<const T&>("key not found");
    }

    /// 获取键对应的值，如果不存在返回默认值
    auto get(const String& key, const T& default_value) const -> T {
        auto idx = find_index(key);
        if (idx && entries_[*idx].state == detail::SlotState::Occupied) {
            return entries_[*idx].value;
        }
        return default_value;
    }

    /// 下标访问（非 const）：如果键不存在，插入默认构造的值并返回引用
    auto operator[](const String& key) -> T& {
        auto* entry = find_or_insert(key);
        return entry->value;
    }

    /// 下标访问（const）：如果键不存在，报错
    auto operator[](const String& key) const -> const T& {
        auto idx = find_index(key);
        if (!idx || entries_[*idx].state != detail::SlotState::Occupied) {
            check(false, "key not found in const dict access");
        }
        return entries_[*idx].value;
    }

    // ========== 修改器 ==========

    /// 插入或更新键值对
    auto insert(const String& key, const T& value) -> void {
        auto* entry = find_or_insert(key);
        entry->value = value;
    }

    auto insert(const String& key, T&& value) -> void {
        auto* entry = find_or_insert(key);
        entry->value = std::move(value);
    }

    /// 批量更新
    template<typename DictLike>
    auto update(const DictLike& other) -> void {
        for (const auto& kv : other) {
            insert(kv.key, kv.value);
        }
    }

    /// 设置默认值：如果键存在，返回值；否则插入默认值并返回它
    auto setdefault(const String& key, const T& default_value = T()) -> T& {
        auto* entry = find_or_insert(key);
        // 如果条目是新插入的（刚创建），需要设置值？find_or_insert 在插入时会使用默认值？
        // 我们设计 find_or_insert 返回条目指针，如果条目之前存在则直接返回，如果不存在则插入一个默认构造的条目（value 默认构造）
        // 所以需要手动设置默认值。我们让 find_or_insert 插入默认构造，然后如果需要可以覆盖。
        // 但 setdefault 应该只在键不存在时才设置默认值，如果存在则不改变。
        // 我们可以在 find_or_insert 中插入默认值，然后在外部判断是否是新的，如果是新的则设置默认值。
        // 或者我们实现一个 try_emplace 风格。简单起见，我们修改 find_or_insert 返回状态指示是否为新插入。
        // 这里为了简单，先实现一个简单版本：
        auto* entry = find_for_setdefault(key);
        // 如果 entry 是新插入的，它的 value 是默认构造，我们在这里设置为 default_value
        // 但我们需要知道是不是新插入的。我们将在私有方法中返回一个 pair。
        // 暂时这样：
        auto result = find_or_insert_detail(key);
        if (result.second) { // 新插入
            result.first->value = default_value;
        }
        return result.first->value;
    }

    /// 删除指定键，返回被删除的值（如果键不存在且未提供默认值，返回错误）
    auto pop(const String& key) -> Result<T, String> {
        auto idx = find_index(key);
        if (!idx || entries_[*idx].state != detail::SlotState::Occupied) {
            return err<T>("pop: key not found");
        }
        T value = std::move(entries_[*idx].value);
        entries_[*idx].state = detail::SlotState::Deleted;
        --size_;
        ++deleted_count_;
        return ok(std::move(value));
    }

    auto pop(const String& key, const T& default_value) -> T {
        auto idx = find_index(key);
        if (!idx || entries_[*idx].state != detail::SlotState::Occupied) {
            return default_value;
        }
        T value = std::move(entries_[*idx].value);
        entries_[*idx].state = detail::SlotState::Deleted;
        --size_;
        ++deleted_count_;
        return value;
    }

    /// 删除并返回一个键值对（LIFO：返回最后一个占用项，即从后往前第一个占用）
    auto popitem() -> Result<std::pair<String, T>, String> {
        if (empty()) {
            return err<std::pair<String, T>>("popitem: dictionary is empty");
        }
        for (auto it = entries_.rbegin(); it != entries_.rend(); ++it) {
            if (it->state == detail::SlotState::Occupied) {
                std::pair<String, T> result(std::move(it->key), std::move(it->value));
                it->state = detail::SlotState::Deleted;
                --size_;
                ++deleted_count_;
                return ok(std::move(result));
            }
        }
        // 理论上不会到这里
        return err<std::pair<String, T>>("popitem: no occupied slot found");
    }

    /// 清空字典
    auto clear() -> void {
        entries_.assign(entries_.size(), detail::Entry<T>()); // 重置所有为空
        size_ = 0;
        deleted_count_ = 0;
    }

    // ========== 视图 ==========
    auto keys() const -> KeysView<T> {
        return KeysView<T>(this);
    }

    auto values() const -> ValuesView<T> {
        return ValuesView<T>(this);
    }

    auto items() const -> ItemsView<T> {
        return ItemsView<T>(this);
    }

    // ========== 拷贝 ==========
    auto copy() const -> Dict {
        return *this;
    }

private:
    std::vector<detail::Entry<T>> entries_;
    SizeType size_;          // 实际占用项数（不包括已删除）
    SizeType deleted_count_; // 已删除标记数

    static constexpr double LOAD_FACTOR = 0.75;
    static constexpr SizeType MIN_CAPACITY = 16;

    /// 计算负载因子
    auto load_factor() const -> double {
        return static_cast<double>(size_ + deleted_count_) / entries_.size();
    }

    /// 是否需要扩容
    auto need_rehash() const -> bool {
        return load_factor() >= LOAD_FACTOR;
    }

    /// 查找键的索引，返回 std::optional<size_t>，如果不存在返回空
    auto find_index(const String& key) const -> std::optional<SizeType> {
        if (entries_.empty()) return std::nullopt;
        SizeType capacity = entries_.size();
        SizeType hash = detail::hash_string(key);
        SizeType index = hash % capacity;
        SizeType i = 0;
        while (i < capacity) {
            const auto& entry = entries_[index];
            if (entry.state == detail::SlotState::Empty) {
                return std::nullopt; // 遇到空，停止查找
            }
            if (entry.state == detail::SlotState::Occupied && entry.key == key) {
                return index; // 找到
            }
            // 继续探测
            ++i;
            index = detail::next_probe(hash % capacity, i, capacity);
        }
        return std::nullopt; // 循环了整个表，没找到
    }

    /// 查找键，如果存在返回条目指针，否则返回一个可插入的位置（空或已删除）
    /// 返回 pair<Entry*, bool>，bool 表示是否为新插入（true 表示新插入，false 表示已存在）
    auto find_or_insert_detail(const String& key) -> std::pair<detail::Entry<T>*, bool> {
        if (need_rehash()) {
            rehash();
        }
        SizeType capacity = entries_.size();
        SizeType hash = detail::hash_string(key);
        SizeType index = hash % capacity;
        SizeType i = 0;
        std::optional<SizeType> first_deleted;
        while (i < capacity) {
            auto& entry = entries_[index];
            if (entry.state == detail::SlotState::Empty) {
                // 找到空位，如果之前有已删除位置，优先使用已删除
                if (first_deleted) {
                    index = *first_deleted;
                    --deleted_count_;  // <--- 修复：重用已删除槽位时减少计数
                }
                // 初始化条目（确保 value 被默认构造）
                entries_[index].key = key;
                entries_[index].value = T{};  // 安全初始化
                entries_[index].state = detail::SlotState::Occupied;
                ++size_;
                return {&entries_[index], true};
            }
            if (entry.state == detail::SlotState::Deleted) {
                if (!first_deleted) {
                    first_deleted = index;
                }
            }
            if (entry.state == detail::SlotState::Occupied && entry.key == key) {
                return {&entries_[index], false}; // 已存在
            }
            ++i;
            index = detail::next_probe(hash % capacity, i, capacity);
        }
        // 理论上不会到这里，因为表已满，应该先扩容
        check(false, "hash table full, should have rehashed");
        return {nullptr, false};
    }

    /// 用于 setdefault 的查找，如果键存在返回条目，否则插入默认构造的条目并返回
    /// 返回 pair<Entry*, bool>，bool 表示是否为新插入
    auto find_or_insert(const String& key) -> detail::Entry<T>* {
        return find_or_insert_detail(key).first;
    }

    /// 重新哈希
    auto rehash() -> void {
        SizeType new_capacity = entries_.size() * 2;
        std::vector<detail::Entry<T>> new_entries(new_capacity);
        // 重新插入所有占用项
        for (auto& entry : entries_) {
            if (entry.state == detail::SlotState::Occupied) {
                SizeType hash = detail::hash_string(entry.key);
                SizeType index = hash % new_capacity;
                SizeType i = 0;
                while (true) {
                    auto& new_entry = new_entries[index];
                    if (new_entry.state == detail::SlotState::Empty) {
                        new_entry.key = std::move(entry.key);
                        new_entry.value = std::move(entry.value);
                        new_entry.state = detail::SlotState::Occupied;
                        break;
                    }
                    ++i;
                    index = detail::next_probe(hash % new_capacity, i, new_capacity);
                }
            }
        }
        entries_ = std::move(new_entries);
        deleted_count_ = 0;
    }
};

} // namespace ks