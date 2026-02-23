cmake_minimum_required(VERSION 3.15)
project(ks LANGUAGES CXX)

# 设置 C++ 标准为 C++17（C++18 未正式存在，使用 C++17）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 可选：禁用异常（如果不需要）
# add_compile_options(-fno-exceptions)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 收集源文件
# 有实际实现的 cpp 文件
set(KS_SOURCES
    src/string.cpp
    src/bigint.cpp
    src/decimal.cpp
)

# 头文件（用于 IDE 显示，不是必需的）
set(KS_HEADERS
    src/result.hpp
    src/string.hpp
    src/list.hpp
    src/dict.hpp
    src/print.hpp
    src/color.hpp
    src/bigint.hpp
    src/decimal.hpp
    src/check.hpp
)

# 创建 ks 库（静态库）
add_library(ks STATIC ${KS_SOURCES} ${KS_HEADERS})

# 对于只包含头文件的组件，不需要额外处理
# 但如果某些源文件是空的，也可以添加，但不影响

# 指定目标包含目录
target_include_directories(ks PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 可选：定义预处理器宏，例如禁用颜色
# target_compile_definitions(ks PUBLIC KS_DISABLE_COLOR)

# ========== 测试 ==========
# 处理 Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 添加测试可执行文件
add_executable(test test.cpp)

# 链接库
target_link_libraries(test
    ks
    GTest::gtest
    GTest::gtest_main
)

# 包含目录（如果需要）
target_include_directories(test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 启用测试
include(GoogleTest)
gtest_discover_tests(test)