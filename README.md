# ks - Kiz's C++ Standard Library

https://img.shields.io/badge/License-MIT-yellow.svg
https://img.shields.io/badge/C%2B%2B-17-blue.svg
https://img.shields.io/badge/CMake-3.15+-brightgreen

ks 是一个为 C++17 设计的现代化、高效、内存安全的组件库。它借鉴了 Rust 的优秀设计理念，提供了一系列比标准库更易用、更安全的工具类，帮助开发者编写更健壮的代码，同时保持高性能。

## ✨ 特性

· 无需异常：所有错误处理均通过 ks::Result 显式进行，避免异常开销和不可预测的控制流。
· 内存安全：大量使用智能指针和 RAII，默认提供边界检查（调试模式下）。
· 高效实现：精心设计的数据结构（如 Dict 采用开放地址哈希表，BigInt 使用 10^9 基底）兼顾性能与内存占用。
· 现代化接口：方法命名参考 Python 和 Rust，直观易用。
· 头文件为主：大多数组件为 header-only，方便集成；少量组件分离实现以减少编译时间。

## 📦 组件列表

组件 描述
ks::Result<T, E> 类似 std::expected 或 Rust 的 Result，用于无异常错误处理。
ks::String UTF-8 字符串，提供类似 Python 的丰富方法（split, strip, replace 等）。
ks::List<T> 类似 std::vector，但添加了 append, extend, pop, join 等实用方法。
ks::Dict<T> 紧凑高效的哈希表，键为 ks::String，采用开放地址线性探测，自动扩容。
ks::print / ks::println 类似 C++23 std::print 的格式化输出，支持 {} 占位符。
ks::Color 命名空间，提供 ANSI 颜色码字符串，可通过宏 KS_DISABLE_COLOR 禁用。
ks::BigInt 无限精度整数，使用 10^9 基底存储，支持高效算术运算（加、减、乘、除、幂）。
ks::Decimal 高精度十进制小数，基于 BigInt 实现，适合金融等需要精确计算的场景。
ks::check(expr, msg) 类似 assert，但接受 ks::String 作为错误消息，并直接终止程序。

## 📐 编码规范（项目使用）

ks 库自身的实现遵循以下规范，您也可以在自己的项目中借鉴：

· 命名空间 ks 内部不缩进。
· 使用 #pragma once 代替头文件保护宏。
· 函数使用后置返回类型（如 auto foo() -> int），void 函数除外。
· 左大括号不换行。
· 变量和函数名使用小写加下划线（snake_case），类型和类名使用大驼峰（PascalCase）。
· 私有成员末尾加下划线（如 data_）。
· 优先使用 auto 声明变量，避免使用 auto*。
· 多使用智能指针管理动态内存。
· 不建议使用 std::array，优先使用 C 风格固定大小数组。
· 允许使用 C 风格强制转换以提高可读性。
· 禁止使用异常和 try-catch，错误处理使用 ks::Result。

## 🔧 构建要求

· 编译器：支持 C++17 的编译器（如 GCC 7+, Clang 5+, MSVC 2017+）
· CMake：3.15 或更高版本
· 可选：Google Test（自动下载，用于测试）

## 🚀 快速开始

1. 集成到你的项目

方法一：作为子模块

```bash
git submodule add https://github.com/yourname/ks.git
```

然后在你的 CMakeLists.txt 中添加：

```cmake
add_subdirectory(ks)
target_link_libraries(your_target ks)
target_include_directories(your_target PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ks)
```

方法二：直接拷贝源码

将 src 文件夹复制到你的项目中，并将 src/string.cpp、src/bigint.cpp、src/decimal.cpp 添加到你的构建系统。

2. 使用示例

```cpp
#include <ks/result.hpp>
#include <ks/string.hpp>
#include <ks/list.hpp>
#include <ks/dict.hpp>
#include <ks/print.hpp>
#include <ks/color.hpp>
#include <ks/bigint.hpp>
#include <ks/decimal.hpp>
#include <ks/check.hpp>

using namespace ks;

int main() {
    // Result 用法
    auto r = ok<int, String>(42);
    if (r.is_ok()) {
        print("value: {}\n", r.value());
    }

    // String 用法
    String s = "  hello world  ";
    print("stripped: '{}'\n", s.strip());

    // List 用法
    List<int> lst = {1, 2, 3};
    lst.append(4);
    print("list: {}\n", lst.join(", "));

    // Dict 用法
    Dict<String> dict;
    dict.insert("name", "Kiz");
    dict.insert("lang", "C++");
    print("name = {}\n", dict.get("name").value_or("unknown"));

    // Color 输出
    println(ks::Color::green + "This is green!" + ks::Color::reset);

    // BigInt 计算
    auto big = BigInt::from_string("12345678901234567890").value();
    auto squared = big * big;
    println("big^2 = {}", squared.to_string());

    // Decimal 精确小数
    auto dec = Decimal::from_string("3.14159").value();
    println("pi = {}", dec.to_string());

    // check 断言
    int x = 10;
    check(x > 5, "x should be greater than 5");
}
```

3. 编译并运行测试

```bash
git clone https://github.com/yourname/ks.git
cd ks
mkdir build && cd build
cmake ..
cmake --build .
ctest
```

## ⚙️ 配置选项

· KS_DISABLE_COLOR：定义此宏可使 ks::Color 中的所有颜色字符串变为空，用于不支持 ANSI 颜色的终端。
  在 CMake 中：
  ```cmake
  target_compile_definitions(ks PUBLIC KS_DISABLE_COLOR)
  ```

## 📄 许可证

本项目使用 MIT 许可证，详见 LICENSE 文件。

---

ks 旨在让 C++ 编程更愉快、更安全。欢迎贡献代码、提出问题或建议！