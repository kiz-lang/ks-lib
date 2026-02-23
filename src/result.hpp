#pragma once

#include <variant>
#include <type_traits>
#include <cassert>
#include <utility>

namespace ks {

// 前向声明
template<typename T, typename E>
class Result;

/// 创建一个成功的 Result（值版本）
template<typename T, typename E>
auto ok(T&& value) -> Result<std::decay_t<T>, std::decay_t<E>> {
    return Result<std::decay_t<T>, std::decay_t<E>>::ok(std::forward<T>(value));
}

/// 创建一个失败的 Result（错误版本，适用于任何 T，包括 void）
template<typename E>
auto err(E&& error) -> Result<void, std::decay_t<E>> {
    return Result<void, std::decay_t<E>>::err(std::forward<E>(error));
}

// ========== 通用 Result (T 非 void) ==========
template<typename T, typename E>
class Result {
public:
    using ValueType = T;
    using ErrorType = E;

    /// 构造一个成功的 Result
    static auto ok(T value) -> Result {
        return Result(std::move(value));
    }

    /// 构造一个失败的 Result
    static auto err(E error) -> Result {
        return Result(std::move(error));
    }

    Result() = delete;

    /// 检查是否成功
    auto is_ok() const noexcept -> bool {
        return std::holds_alternative<T>(data_);
    }

    /// 检查是否失败
    auto is_err() const noexcept -> bool {
        return std::holds_alternative<E>(data_);
    }

    /// 获取值的 const 引用（如果失败则触发断言）
    auto value() const -> const T& {
        assert(is_ok() && "Called value() on an error Result");
        return std::get<T>(data_);
    }

    /// 获取值的引用（如果失败则触发断言）
    auto value() -> T& {
        assert(is_ok() && "Called value() on an error Result");
        return std::get<T>(data_);
    }

    /// 获取错误的 const 引用（如果成功则触发断言）
    auto error() const -> const E& {
        assert(is_err() && "Called error() on a success Result");
        return std::get<E>(data_);
    }

    /// 获取错误的引用（如果成功则触发断言）
    auto error() -> E& {
        assert(is_err() && "Called error() on a success Result");
        return std::get<E>(data_);
    }

    /// 如果成功返回值，否则返回默认值
    template<typename U>
    auto value_or(U&& default_value) const -> T {
        return is_ok() ? value() : static_cast<T>(std::forward<U>(default_value));
    }

    /// 将成功值通过函数 f 映射到新类型 U，返回 Result<U, E>
    template<typename F>
    auto map(F&& f) const -> Result<typename std::invoke_result_t<F, T>, E> {
        using U = typename std::invoke_result_t<F, T>;
        if (is_ok()) {
            return Result<U, E>::ok(f(value()));
        } else {
            return Result<U, E>::err(error());
        }
    }

    /// 将错误通过函数 f 映射到新类型 NewError，返回 Result<T, NewError>
    template<typename F>
    auto map_err(F&& f) const -> Result<T, typename std::invoke_result_t<F, E>> {
        using NewError = typename std::invoke_result_t<F, E>;
        if (is_err()) {
            return Result<T, NewError>::err(f(error()));
        } else {
            return Result<T, NewError>::ok(value());
        }
    }

    /// 如果成功，则调用 f（返回 Result<U, E>）并返回其结果；否则传播错误
    template<typename F>
    auto and_then(F&& f) const -> decltype(f(value())) {
        using ResultType = decltype(f(value()));
        if (is_ok()) {
            return f(value());
        } else {
            return ResultType::err(error());
        }
    }

    /// 如果失败，则调用 f（处理错误），并返回自身（通常用于副作用）
    template<typename F>
    auto or_else(F&& f) const -> Result<T, E> {
        if (is_err()) {
            f(error());
            return err(error());
        } else {
            return *this;
        }
    }

    /// 解引用运算符，直接访问值（失败时断言）
    auto operator*() const -> const T& {
        return value();
    }

    auto operator*() -> T& {
        return value();
    }

    /// 箭头运算符，允许访问值的成员
    auto operator->() const -> const T* {
        return &value();
    }

    auto operator->() -> T* {
        return &value();
    }

private:
    std::variant<T, E> data_;

    explicit Result(T value) : data_(std::in_place_type<T>, std::move(value)) {}
    explicit Result(E error) : data_(std::in_place_type<E>, std::move(error)) {}
};

// ========== Result<void, E> 特化 ==========
template<typename E>
class Result<void, E> {
public:
    using ValueType = void;
    using ErrorType = E;

    /// 构造一个成功的 void Result
    static auto ok() -> Result {
        return Result(std::monostate{});
    }

    /// 构造一个失败的 Result
    static auto err(E error) -> Result {
        return Result(std::move(error));
    }

    Result() = delete;

    /// 检查是否成功
    auto is_ok() const noexcept -> bool {
        return std::holds_alternative<std::monostate>(data_);
    }

    /// 检查是否失败
    auto is_err() const noexcept -> bool {
        return std::holds_alternative<E>(data_);
    }

    /// void 版本 value() 仅用于断言成功
    auto value() const -> void {
        assert(is_ok() && "Called value() on an error Result");
    }

    /// 获取错误的 const 引用
    auto error() const -> const E& {
        assert(is_err() && "Called error() on a success Result");
        return std::get<E>(data_);
    }

    /// 获取错误的引用
    auto error() -> E& {
        assert(is_err() && "Called error() on a success Result");
        return std::get<E>(data_);
    }

    /// 映射：如果成功，调用 f（无参数）并返回 Result<U, E>
    template<typename F>
    auto map(F&& f) const -> Result<typename std::invoke_result_t<F>, E> {
        using U = typename std::invoke_result_t<F>;
        if (is_ok()) {
            return Result<U, E>::ok(f());
        } else {
            return Result<U, E>::err(error());
        }
    }

    /// 映射错误
    template<typename F>
    auto map_err(F&& f) const -> Result<void, typename std::invoke_result_t<F, E>> {
        using NewError = typename std::invoke_result_t<F, E>;
        if (is_err()) {
            return Result<void, NewError>::err(f(error()));
        } else {
            return Result<void, NewError>::ok();
        }
    }

    /// 如果成功，调用 f（返回 Result<U, E>）并返回其结果
    template<typename F>
    auto and_then(F&& f) const -> decltype(f()) {
        using ResultType = decltype(f());
        if (is_ok()) {
            return f();
        } else {
            return ResultType::err(error());
        }
    }

    /// 如果失败，调用 f 处理错误，并返回自身
    template<typename F>
    auto or_else(F&& f) const -> Result<void, E> {
        if (is_err()) {
            f(error());
            return err(error());
        } else {
            return *this;
        }
    }

private:
    std::variant<std::monostate, E> data_;

    explicit Result(std::monostate) : data_(std::in_place_type<std::monostate>) {}
    explicit Result(E error) : data_(std::in_place_type<E>, std::move(error)) {}
};

} // namespace ks