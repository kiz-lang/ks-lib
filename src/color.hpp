#pragma once

#include <string>

namespace ks {
namespace Color {

// 宏控制：如果定义了 KS_DISABLE_COLOR，则所有颜色字符串为空；否则为ANSI转义序列
#ifdef KS_DISABLE_COLOR
    #define KS_COLOR_STRING(s) ""
#else
    #define KS_COLOR_STRING(s) s
#endif

/// 重置所有属性
inline std::string reset = KS_COLOR_STRING("\033[0m");

/// 文本样式控制
inline std::string bold = KS_COLOR_STRING("\033[1m");        // 加粗
inline std::string faint = KS_COLOR_STRING("\033[2m");       // 淡色
inline std::string italic = KS_COLOR_STRING("\033[3m");      // 斜体（部分终端支持）
inline std::string underline = KS_COLOR_STRING("\033[4m");   // 下划线
inline std::string blink = KS_COLOR_STRING("\033[5m");       // 闪烁（部分终端支持）
inline std::string reverse = KS_COLOR_STRING("\033[7m");     // 反色显示
inline std::string hidden = KS_COLOR_STRING("\033[8m");      // 隐藏文本（部分终端支持）

/// 前景色（文字颜色）- 标准色
inline std::string black = KS_COLOR_STRING("\033[30m");      // 黑色
inline std::string red = KS_COLOR_STRING("\033[31m");        // 红色
inline std::string green = KS_COLOR_STRING("\033[32m");      // 绿色
inline std::string yellow = KS_COLOR_STRING("\033[33m");     // 黄色
inline std::string blue = KS_COLOR_STRING("\033[34m");       // 蓝色
inline std::string magenta = KS_COLOR_STRING("\033[35m");    // 品红色
inline std::string cyan = KS_COLOR_STRING("\033[36m");       // 青色
inline std::string white = KS_COLOR_STRING("\033[37m");      // 白色

/// 前景色 - 高亮色（亮色系）
inline std::string bright_black = KS_COLOR_STRING("\033[90m");   // 亮黑色（灰色）
inline std::string bright_red = KS_COLOR_STRING("\033[91m");     // 亮红色
inline std::string bright_green = KS_COLOR_STRING("\033[92m");   // 亮绿色
inline std::string bright_yellow = KS_COLOR_STRING("\033[93m");  // 亮黄色
inline std::string bright_blue = KS_COLOR_STRING("\033[94m");    // 亮蓝色
inline std::string bright_magenta = KS_COLOR_STRING("\033[95m"); // 亮品红色
inline std::string bright_cyan = KS_COLOR_STRING("\033[96m");    // 亮青色
inline std::string bright_white = KS_COLOR_STRING("\033[97m");   // 亮白色

/// 背景色 - 标准色
inline std::string bg_black = KS_COLOR_STRING("\033[40m");       // 背景黑色
inline std::string bg_red = KS_COLOR_STRING("\033[41m");         // 背景红色
inline std::string bg_green = KS_COLOR_STRING("\033[42m");       // 背景绿色
inline std::string bg_yellow = KS_COLOR_STRING("\033[43m");      // 背景黄色
inline std::string bg_blue = KS_COLOR_STRING("\033[44m");        // 背景蓝色
inline std::string bg_magenta = KS_COLOR_STRING("\033[45m");     // 背景品红色
inline std::string bg_cyan = KS_COLOR_STRING("\033[46m");        // 背景青色
inline std::string bg_white = KS_COLOR_STRING("\033[47m");       // 背景白色

/// 背景色 - 高亮色（亮色系）
inline std::string bg_bright_black = KS_COLOR_STRING("\033[100m");  // 背景亮黑色（灰色）
inline std::string bg_bright_red = KS_COLOR_STRING("\033[101m");    // 背景亮红色
inline std::string bg_bright_green = KS_COLOR_STRING("\033[102m");  // 背景亮绿色
inline std::string bg_bright_yellow = KS_COLOR_STRING("\033[103m"); // 背景亮黄色
inline std::string bg_bright_blue = KS_COLOR_STRING("\033[104m");   // 背景亮蓝色
inline std::string bg_bright_magenta = KS_COLOR_STRING("\033[105m");// 背景亮品红色
inline std::string bg_bright_cyan = KS_COLOR_STRING("\033[106m");   // 背景亮青色
inline std::string bg_bright_white = KS_COLOR_STRING("\033[107m");  // 背景亮白色

#undef KS_COLOR_STRING

} // namespace Color
} // namespace ks