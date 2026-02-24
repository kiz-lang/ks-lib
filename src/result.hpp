#pragma once

#include <variant>
#include <type_traits>
#include <cassert>
#include <utility>
#include <exception>  // for std::terminate

namespace ks {

// 前向声明
template<typename T, typename E>
class Result;

// ========== 辅助类型：Ok<T> 和 Err<E> ==========
// 用于隐式构造 Result 的标记类型，类似 Rust 的 Ok 和 Err

template<typename T>
struct Ok {
    T value;
    explicit Ok(T&& v) : value(std::move(v)) {}
    explicit Ok(const T& v) : value(v) {}
};

template<typename E>
struct Err {
    E error;
    explicit Err(E&& e) : error(std::move(e)) {}
    explicit Err(const E& e) : error(e) {}
};

// 推导指引（C++17 可选，但加上有帮助）
template<typename T> Ok(T) -> Ok<T>;
template<typename E> Err(E) -> Err<E>;

// ========== 通用 Result (T 非 void) ==========
template<typename T, typename E>
class Result {
    std::variant<T, E> data_;
public:
    using ValueType = T;
    using ErrorType = E;

    // 从 Ok 和 Err 隐式构造
    Result(Ok<T> ok) : data_(std::in_place_type<T>, std::move(ok.value)) {}
    Result(Err<E> err) : data_(std::in_place_type<E>, std::move(err.error)) {}

    // 静态工厂方法（也可用，但推荐使用 Ok/Err）
    static auto ok(T value) -> Result {
        return Result(Ok<T>{std::move(value)});
    }
    static auto err(E error) -> Result {
        return Result(Err<E>{std::move(error)});
    }

    Result() = delete;

    // 检查
    auto is_ok() const noexcept -> bool {
        return std::holds_alternative<T>(data_);
    }
    auto is_err() const noexcept -> bool {
        return std::holds_alternative<E>(data_);
    }

    // 值访问（左值版本）
    auto value() & -> T& {
        assert(is_ok() && "Called value() on an error Result");
        return std::get<T>(data_);
    }
    auto value() const& -> const T& {
        assert(is_ok() && "Called value() on an error Result");
        return std::get<T>(data_);
    }

    // 值访问（右值版本，允许移动出值）
    auto value() && -> T&& {
        assert(is_ok() && "Called value() on an error Result");
        return std::move(std::get<T>(data_));
    }

    // 错误访问
    auto error() & -> E& {
        assert(is_err() && "Called error() on a success Result");
        return std::get<E>(data_);
    }
    auto error() const& -> const E& {
        assert(is_err() && "Called error() on a success Result");
        return std::get<E>(data_);
    }
    auto error() && -> E&& {
        assert(is_err() && "Called error() on a success Result");
        return std::move(std::get<E>(data_));
    }

    // 取默认值
    template<typename U>
    auto value_or(U&& default_value) const& -> T {
        return is_ok() ? value() : T(std::forward<U>(default_value));
    }
    template<typename U>
    auto value_or(U&& default_value) && -> T {
        return is_ok() ? std::move(*this).value() : T(std::forward<U>(default_value));
    }

    // 解引用运算符
    auto operator*() & -> T& { return value(); }
    auto operator*() const& -> const T& { return value(); }
    auto operator*() && -> T&& { return std::move(*this).value(); }

    auto operator->() const -> const T* { return &value(); }
    auto operator->() -> T* { return &value(); }

    // 映射值
    template<typename F>
    auto map(F&& f) const& -> Result<typename std::invoke_result_t<F, T>, E> {
        using U = typename std::invoke_result_t<F, T>;
        static_assert(!std::is_void_v<U>, "map() function must return a non-void type");
        if (is_ok()) {
            return Result<U, E>::ok(f(value()));
        } else {
            return Result<U, E>::err(error());
        }
    }

    template<typename F>
    auto map(F&& f) && -> Result<typename std::invoke_result_t<F, T>, E> {
        using U = typename std::invoke_result_t<F, T>;
        static_assert(!std::is_void_v<U>, "map() function must return a non-void type");
        if (is_ok()) {
            return Result<U, E>::ok(f(std::move(*this).value()));
        } else {
            return Result<U, E>::err(std::move(*this).error());
        }
    }

    // 映射错误
    template<typename F>
    auto map_err(F&& f) const& -> Result<T, typename std::invoke_result_t<F, E>> {
        using NewError = typename std::invoke_result_t<F, E>;
        if (is_err()) {
            return Result<T, NewError>::err(f(error()));
        } else {
            return Result<T, NewError>::ok(value());
        }
    }

    template<typename F>
    auto map_err(F&& f) && -> Result<T, typename std::invoke_result_t<F, E>> {
        using NewError = typename std::invoke_result_t<F, E>;
        if (is_err()) {
            return Result<T, NewError>::err(f(std::move(*this).error()));
        } else {
            return Result<T, NewError>::ok(std::move(*this).value());
        }
    }

    // 链式操作：and_then
    template<typename F>
    auto and_then(F&& f) const& -> decltype(f(value())) {
        using ResultType = decltype(f(value()));
        if (is_ok()) {
            return f(value());
        } else {
            return ResultType::err(error());
        }
    }

    template<typename F>
    auto and_then(F&& f) && -> decltype(f(std::move(*this).value())) {
        using ResultType = decltype(f(std::move(*this).value()));
        if (is_ok()) {
            return f(std::move(*this).value());
        } else {
            return ResultType::err(std::move(*this).error());
        }
    }

    // 副作用：or_else
    template<typename F>
    auto or_else(F&& f) const& -> Result<T, E> {
        if (is_err()) {
            f(error());
            return *this; // 返回自身（拷贝）
        } else {
            return *this;
        }
    }

    template<typename F>
    auto or_else(F&& f) && -> Result<T, E> {
        if (is_err()) {
            f(std::move(*this).error());
            return std::move(*this);
        } else {
            return std::move(*this);
        }
    }

    // 失败时终止（类似 Rust 的 unwrap/expect）
    auto unwrap() const& -> const T& {
        if (is_err()) {
            std::terminate(); // 实际可打印错误信息
        }
        return value();
    }

    auto unwrap() && -> T&& {
        if (is_err()) {
            std::terminate();
        }
        return std::move(*this).value();
    }

    auto expect(const char* msg) const& -> const T& {
        if (is_err()) {
            // 理想情况应打印 msg 和错误信息，此处简化
            std::terminate();
        }
        return value();
    }

    auto expect(const char* msg) && -> T&& {
        if (is_err()) {
            std::terminate();
        }
        return std::move(*this).value();
    }
};

// ========== Result<void, E> 特化 ==========
template<typename E>
class Result<void, E> {
public:
    using ValueType = void;
    using ErrorType = E;

    // 从 Err 构造（Ok 特化：无 Ok<void> 类型，直接使用静态方法）
    Result(Err<E> err) : data_(std::in_place_type<E>, std::move(err.error)) {}

    static auto ok() -> Result {
        return Result(std::monostate{});
    }
    static auto err(E error) -> Result {
        return Result(Err<E>{std::move(error)});
    }

    Result() = delete;

    auto is_ok() const noexcept -> bool {
        return std::holds_alternative<std::monostate>(data_);
    }
    auto is_err() const noexcept -> bool {
        return std::holds_alternative<E>(data_);
    }

    void value() const {
        assert(is_ok() && "Called value() on an error Result");
    }

    auto error() & -> E& {
        assert(is_err() && "Called error() on a success Result");
        return std::get<E>(data_);
    }
    auto error() const& -> const E& {
        assert(is_err() && "Called error() on a success Result");
        return std::get<E>(data_);
    }
    auto error() && -> E&& {
        assert(is_err() && "Called error() on a success Result");
        return std::move(std::get<E>(data_));
    }

    // 映射值
    template<typename F>
    auto map(F&& f) const -> Result<typename std::invoke_result_t<F>, E> {
        using U = typename std::invoke_result_t<F>;
        static_assert(!std::is_void_v<U>, "map() function must return a non-void type");
        if (is_ok()) {
            return Result<U, E>::ok(f());
        } else {
            return Result<U, E>::err(error());
        }
    }

    // 映射错误
    template<typename F>
    auto map_err(F&& f) const& -> Result<void, typename std::invoke_result_t<F, E>> {
        using NewError = typename std::invoke_result_t<F, E>;
        if (is_err()) {
            return Result<void, NewError>::err(f(error()));
        } else {
            return Result<void, NewError>::ok();
        }
    }

    template<typename F>
    auto map_err(F&& f) && -> Result<void, typename std::invoke_result_t<F, E>> {
        using NewError = typename std::invoke_result_t<F, E>;
        if (is_err()) {
            return Result<void, NewError>::err(f(std::move(*this).error()));
        } else {
            return Result<void, NewError>::ok();
        }
    }

    // and_then
    template<typename F>
    auto and_then(F&& f) const -> decltype(f()) {
        using ResultType = decltype(f());
        if (is_ok()) {
            return f();
        } else {
            return ResultType::err(error());
        }
    }

    // or_else
    template<typename F>
    auto or_else(F&& f) const& -> Result<void, E> {
        if (is_err()) {
            f(error());
            return *this;
        } else {
            return *this;
        }
    }

    template<typename F>
    auto or_else(F&& f) && -> Result<void, E> {
        if (is_err()) {
            f(std::move(*this).error());
            return std::move(*this);
        } else {
            return std::move(*this);
        }
    }

    // unwrap/expect
    void unwrap() const {
        if (is_err()) std::terminate();
    }
    void expect(const char* msg) const {
        if (is_err()) std::terminate();
    }

private:
    std::variant<std::monostate, E> data_;

    explicit Result(std::monostate) : data_(std::in_place_type<std::monostate>) {}
    explicit Result(Err<E> err) : data_(std::in_place_type<E>, std::move(err.error)) {}
};

// ========== 全局辅助函数（使用 Ok/Err） ==========
template<typename T>
auto ok(T&& value) {
    return Ok<std::decay_t<T>>{std::forward<T>(value)};
}

template<typename E>
auto err(E&& error) {
    return Err<std::decay_t<E>>{std::forward<E>(error)};
}

} // namespace ks