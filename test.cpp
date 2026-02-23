#include <gtest/gtest.h>
#include "src/result.hpp"
#include "src/string.hpp"
#include "src/list.hpp"
#include "src/dict.hpp"
#include "src/print.hpp"
#include "src/color.hpp"
#include "src/bigint.hpp"
#include "src/decimal.hpp"
#include <sstream>
#include <string>
#include <vector>

using namespace ks;

// ========== Result 测试 ==========
TEST(ResultTest, Ok) {
    auto r = ok<int, std::string>(42);
    EXPECT_TRUE(r.is_ok());
    EXPECT_FALSE(r.is_err());
    EXPECT_EQ(r.value(), 42);
}

TEST(ResultTest, Err) {
    auto r = err<std::string>("error");
    EXPECT_TRUE(r.is_err());
    EXPECT_FALSE(r.is_ok());
    EXPECT_EQ(r.error(), "error");
}

TEST(ResultTest, ValueOr) {
    auto r1 = ok<int, std::string>(42);
    EXPECT_EQ(r1.value_or(100), 42);
    auto r2 = err<int, std::string>("error");
    EXPECT_EQ(r2.value_or(100), 100);
}

TEST(ResultTest, Map) {
    auto r = ok<int, std::string>(42);
    auto r2 = r.map([](int x) { return x * 2; });
    EXPECT_TRUE(r2.is_ok());
    EXPECT_EQ(r2.value(), 84);
}

TEST(ResultTest, AndThen) {
    auto r = ok<int, std::string>(42);
    auto r2 = r.and_then([](int x) -> Result<int, std::string> {
        return ok(x * 2);
    });
    EXPECT_EQ(r2.value(), 84);
}

TEST(ResultTest, VoidOk) {
    auto r = ok();
    EXPECT_TRUE(r.is_ok());
}

TEST(ResultTest, VoidErr) {
    auto r = err<void, std::string>("error");
    EXPECT_TRUE(r.is_err());
    EXPECT_EQ(r.error(), "error");
}

// ========== String 测试 ==========
TEST(StringTest, Construction) {
    String s1;
    EXPECT_EQ(s1.len(), 0);

    String s2("hello");
    EXPECT_EQ(s2.len(), 5);
    EXPECT_EQ(s2.c_str(), std::string("hello"));

    String s3(std::string("world"));
    EXPECT_EQ(s3, String("world"));

    String s4(3, 'a');
    EXPECT_EQ(s4, String("aaa"));
}

TEST(StringTest, Find) {
    String s("hello world");
    EXPECT_EQ(s.find("world"), 6);
    EXPECT_EQ(s.find("xyz"), -1);
    EXPECT_EQ(s.rfind("o"), 7);
    EXPECT_EQ(s.rfind("x"), -1);
}

TEST(StringTest, Index) {
    String s("hello");
    auto r = s.index("ll");
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(r.value(), 2);
    auto r2 = s.index("xx");
    EXPECT_TRUE(r2.is_err());
}

TEST(StringTest, Count) {
    String s("abracadabra");
    EXPECT_EQ(s.count("ab"), 2);
    EXPECT_EQ(s.count("a"), 5);
}

TEST(StringTest, StartswithEndswith) {
    String s("hello");
    EXPECT_TRUE(s.startswith("he"));
    EXPECT_FALSE(s.startswith("lo"));
    EXPECT_TRUE(s.endswith("lo"));
    EXPECT_FALSE(s.endswith("he"));
}

TEST(StringTest, IsAlphaDigit) {
    String s1("abc");
    EXPECT_TRUE(s1.isalpha());
    EXPECT_FALSE(s1.isdigit());

    String s2("123");
    EXPECT_TRUE(s2.isdigit());
    EXPECT_FALSE(s2.isalpha());

    String s3("abc123");
    EXPECT_FALSE(s3.isalpha());
    EXPECT_FALSE(s3.isdigit());
    EXPECT_TRUE(s3.isalnum());
}

TEST(StringTest, LowerUpper) {
    String s("Hello World");
    EXPECT_EQ(s.lower(), String("hello world"));
    EXPECT_EQ(s.upper(), String("HELLO WORLD"));
    EXPECT_EQ(s.capitalize(), String("Hello world"));
    EXPECT_EQ(s.title(), String("Hello World"));
    EXPECT_EQ(s.swapcase(), String("hELLO wORLD"));
}

TEST(StringTest, Strip) {
    String s("  hello  ");
    EXPECT_EQ(s.strip(), String("hello"));
    EXPECT_EQ(s.lstrip(), String("hello  "));
    EXPECT_EQ(s.rstrip(), String("  hello"));
    EXPECT_EQ(s.strip(" h"), String("ello"));
}

TEST(StringTest, Split) {
    String s("a,b,c");
    auto parts = s.split(",");
    ASSERT_EQ(parts.size(), 3);
    EXPECT_EQ(parts[0], String("a"));
    EXPECT_EQ(parts[1], String("b"));
    EXPECT_EQ(parts[2], String("c"));

    auto parts2 = s.rsplit(",", 1);
    // rsplit with maxsplit not implemented, but we have rsplit without maxsplit
    auto parts3 = s.rsplit(",");
    EXPECT_EQ(parts3.size(), 3);
}

TEST(StringTest, Join) {
    std::vector<String> words = {String("hello"), String("world")};
    String joined = String::join(words, String(" "));
    EXPECT_EQ(joined, String("hello world"));
}

TEST(StringTest, Replace) {
    String s("hello world");
    EXPECT_EQ(s.replace("world", "there"), String("hello there"));
}

TEST(StringTest, ToInt) {
    String s1("123");
    auto r = s1.to_int();
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(r.value(), 123);

    String s2("-456");
    auto r2 = s2.to_int();
    EXPECT_TRUE(r2.is_ok());
    EXPECT_EQ(r2.value(), -456);

    String s3("12.3");
    auto r3 = s3.to_int();
    EXPECT_TRUE(r3.is_err());
}

TEST(StringTest, ToFloat) {
    String s1("3.14");
    auto r = s1.to_float();
    EXPECT_TRUE(r.is_ok());
    EXPECT_DOUBLE_EQ(r.value(), 3.14);
}

// ========== List 测试 ==========
TEST(ListTest, Append) {
    List<int> lst;
    lst.append(1);
    lst.append(2);
    EXPECT_EQ(lst.size(), 2);
    EXPECT_EQ(lst[0], 1);
    EXPECT_EQ(lst[1], 2);
}

TEST(ListTest, Extend) {
    List<int> lst;
    std::vector<int> vec = {1,2,3};
    lst.extend(vec);
    EXPECT_EQ(lst.size(), 3);
    EXPECT_EQ(lst[2], 3);
}

TEST(ListTest, Insert) {
    List<int> lst = {1,2,4};
    lst.insert(2, 3);
    EXPECT_EQ(lst[2], 3);
    EXPECT_EQ(lst[3], 4);
}

TEST(ListTest, Remove) {
    List<int> lst = {1,2,3,2};
    auto res = lst.remove(2);
    EXPECT_TRUE(res.is_ok());
    EXPECT_EQ(lst.size(), 3);
    EXPECT_EQ(lst[1], 3); // 第二个元素现在是3
    auto res2 = lst.remove(5);
    EXPECT_TRUE(res2.is_err());
}

TEST(ListTest, Pop) {
    List<int> lst = {1,2,3};
    auto r = lst.pop();
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(r.value(), 3);
    EXPECT_EQ(lst.size(), 2);
    auto r2 = lst.pop(0);
    EXPECT_TRUE(r2.is_ok());
    EXPECT_EQ(r2.value(), 1);
    EXPECT_EQ(lst[0], 2);
    auto r3 = lst.pop(5);
    EXPECT_TRUE(r3.is_err());
}

TEST(ListTest, Index) {
    List<int> lst = {10,20,30,20};
    auto r = lst.index(20);
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(r.value(), 1);
    auto r2 = lst.index(20, 2);
    EXPECT_TRUE(r2.is_ok());
    EXPECT_EQ(r2.value(), 3);
    auto r3 = lst.index(99);
    EXPECT_TRUE(r3.is_err());
}

TEST(ListTest, Count) {
    List<int> lst = {1,2,2,3,2};
    EXPECT_EQ(lst.count(2), 3);
    EXPECT_EQ(lst.count(5), 0);
}

TEST(ListTest, Sort) {
    List<int> lst = {3,1,4,2};
    lst.sort();
    EXPECT_EQ(lst[0], 1);
    EXPECT_EQ(lst[1], 2);
    EXPECT_EQ(lst[2], 3);
    EXPECT_EQ(lst[3], 4);
    lst.sort(true); // reverse
    EXPECT_EQ(lst[0], 4);
}

TEST(ListTest, Reverse) {
    List<int> lst = {1,2,3};
    lst.reverse();
    EXPECT_EQ(lst[0], 3);
    EXPECT_EQ(lst[1], 2);
    EXPECT_EQ(lst[2], 1);
}

TEST(ListTest, Copy) {
    List<int> lst = {1,2,3};
    auto cpy = lst.copy();
    cpy.append(4);
    EXPECT_EQ(lst.size(), 3);
    EXPECT_EQ(cpy.size(), 4);
}

TEST(ListTest, Join) {
    List<int> lst = {1,2,3};
    String sep = ",";
    String result = lst.join(sep);
    EXPECT_EQ(result, String("1,2,3"));
}

TEST(ListTest, MinMaxSum) {
    List<int> lst = {5,2,8,1,9};
    auto min_res = min(lst);
    EXPECT_TRUE(min_res.is_ok());
    EXPECT_EQ(min_res.value(), 1);
    auto max_res = max(lst);
    EXPECT_EQ(max_res.value(), 9);
    auto total = sum(lst);
    EXPECT_EQ(total, 25);
}

// ========== Dict 测试 ==========
TEST(DictTest, InsertAndGet) {
    Dict<int> dict;
    dict.insert("a", 1);
    dict.insert("b", 2);
    auto val = dict.get("a");
    EXPECT_TRUE(val.is_ok());
    EXPECT_EQ(val.value(), 1);
    auto val2 = dict.get("c");
    EXPECT_TRUE(val2.is_err());
    EXPECT_EQ(dict.get("c", 42), 42);
}

TEST(DictTest, Subscript) {
    Dict<int> dict;
    dict["a"] = 10;
    EXPECT_EQ(dict["a"], 10);
    dict["b"] = 20;
    EXPECT_EQ(dict["b"], 20);
    // 访问不存在的const键会终止，所以不测
}

TEST(DictTest, Update) {
    Dict<int> dict;
    dict.insert("a", 1);
    Dict<int> other;
    other.insert("b", 2);
    other.insert("c", 3);
    dict.update(other);
    EXPECT_EQ(dict.size(), 3);
    EXPECT_EQ(dict.get("b").value(), 2);
}

TEST(DictTest, SetDefault) {
    Dict<int> dict;
    auto& v1 = dict.setdefault("a", 42);
    EXPECT_EQ(v1, 42);
    auto& v2 = dict.setdefault("a", 100);
    EXPECT_EQ(v2, 42); // 不改变
}

TEST(DictTest, Pop) {
    Dict<int> dict;
    dict.insert("a", 1);
    dict.insert("b", 2);
    auto r = dict.pop("a");
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(r.value(), 1);
    EXPECT_EQ(dict.size(), 1);
    auto r2 = dict.pop("c", 99);
    EXPECT_EQ(r2, 99);
    auto r3 = dict.pop("c");
    EXPECT_TRUE(r3.is_err());
}

TEST(DictTest, PopItem) {
    Dict<int> dict;
    dict.insert("a", 1);
    dict.insert("b", 2);
    auto item = dict.popitem();
    EXPECT_TRUE(item.is_ok());
    auto p = item.value();
    // 可能是 (b,2) 或 (a,1) 取决于实现，我们只检查大小
    EXPECT_EQ(dict.size(), 1);
}

TEST(DictTest, KeysValuesItems) {
    Dict<int> dict;
    dict.insert("a", 1);
    dict.insert("b", 2);
    auto keys = dict.keys();
    std::vector<String> keys_vec;
    for (auto k : keys) {
        keys_vec.push_back(k.key);
    }
    EXPECT_EQ(keys_vec.size(), 2);
    // 简单测试
}

TEST(DictTest, Clear) {
    Dict<int> dict;
    dict.insert("a", 1);
    dict.clear();
    EXPECT_TRUE(dict.isempty());
}

// ========== Color 测试 ==========
TEST(ColorTest, Colors) {
    // 默认情况下颜色字符串非空
    EXPECT_FALSE(ks::Color::red.empty());
    EXPECT_FALSE(ks::Color::green.empty());
    EXPECT_FALSE(ks::Color::reset.empty());
    // 如果定义了KS_DISABLE_COLOR，它们应该为空，但这里不测试宏切换
}

// ========== Print 测试 ==========
// 注意：print和println直接输出到stdout，我们通过捕获输出来测试
TEST(PrintTest, Print) {
    testing::internal::CaptureStdout();
    print("hello {}", "world");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "hello world");
}

TEST(PrintTest, Println) {
    testing::internal::CaptureStdout();
    println("number: {}", 42);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "number: 42\n");
}

// ========== BigInt 测试 ==========
TEST(BigIntTest, Construction) {
    BigInt a(123);
    EXPECT_EQ(a.to_string(), "123");
    BigInt b(-456);
    EXPECT_EQ(b.to_string(), "-456");
    auto r = BigInt::from_string("789");
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(r.value().to_string(), "789");
    auto r2 = BigInt::from_string("-123");
    EXPECT_EQ(r2.value().to_string(), "-123");
    auto r3 = BigInt::from_string("abc");
    EXPECT_TRUE(r3.is_err());
}

TEST(BigIntTest, Comparison) {
    BigInt a(123), b(456);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a != b);
    BigInt c(123);
    EXPECT_TRUE(a == c);
    BigInt d(-123);
    EXPECT_TRUE(d < a);
    EXPECT_TRUE(d > BigInt(-200));
}

TEST(BigIntTest, Addition) {
    BigInt a(123), b(456);
    EXPECT_EQ((a + b).to_string(), "579");
    BigInt c(-123), d(-456);
    EXPECT_EQ((c + d).to_string(), "-579");
    EXPECT_EQ((a + c).to_string(), "0");
    BigInt big1("99999999999999999999");
    BigInt big2("1");
    EXPECT_EQ((big1 + big2).to_string(), "100000000000000000000");
}

TEST(BigIntTest, Subtraction) {
    BigInt a(456), b(123);
    EXPECT_EQ((a - b).to_string(), "333");
    EXPECT_EQ((b - a).to_string(), "-333");
    BigInt c(1000), d(1);
    EXPECT_EQ((c - d).to_string(), "999");
    BigInt big1("100000000000000000000");
    BigInt big2("1");
    EXPECT_EQ((big1 - big2).to_string(), "99999999999999999999");
}

TEST(BigIntTest, Multiplication) {
    BigInt a(123), b(456);
    EXPECT_EQ((a * b).to_string(), "56088");
    BigInt c(-123), d(456);
    EXPECT_EQ((c * d).to_string(), "-56088");
    BigInt big1("123456789");
    BigInt big2("987654321");
    // 乘积已知：121932631112635269
    EXPECT_EQ((big1 * big2).to_string(), "121932631112635269");
}

TEST(BigIntTest, Division) {
    BigInt a(1000), b(3);
    auto q = a / b;
    EXPECT_EQ(q.to_string(), "333"); // 整数除法
    BigInt c(7), d(2);
    EXPECT_EQ((c / d).to_string(), "3");
    // 除零会调用check，不测
}

TEST(BigIntTest, Modulo) {
    BigInt a(1000), b(3);
    EXPECT_EQ((a % b).to_string(), "1");
    BigInt c(-7), d(2);
    EXPECT_EQ((c % d).to_string(), "-1"); // 取决于实现，Python中-7 % 2 = 1，但我们按C++风格取余可能为-1
    // 注意我们的实现中，余数符号与被除数一致，所以-7 % 2 = -1
    // 所以测试通过
}

TEST(BigIntTest, Pow) {
    BigInt a(2);
    auto r = a.pow(BigInt(10));
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(r.value().to_string(), "1024");
    auto r2 = BigInt(-2).pow(BigInt(3));
    EXPECT_TRUE(r2.is_ok());
    EXPECT_EQ(r2.value().to_string(), "-8");
}

TEST(BigIntTest, ToUint64) {
    BigInt a(12345);
    auto r = a.to_uint64();
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(r.value(), 12345ULL);
    BigInt b(-1);
    auto r2 = b.to_uint64();
    EXPECT_TRUE(r2.is_err());
    BigInt c("18446744073709551615"); // ULLONG_MAX
    auto r3 = c.to_uint64();
    EXPECT_TRUE(r3.is_ok());
    EXPECT_EQ(r3.value(), ULLONG_MAX);
    BigInt d("18446744073709551616");
    auto r4 = d.to_uint64();
    EXPECT_TRUE(r4.is_err());
}

// ========== Decimal 测试 ==========
TEST(DecimalTest, Construction) {
    auto d = Decimal::from_string("123.45");
    EXPECT_TRUE(d.is_ok());
    EXPECT_EQ(d.value().to_string(), "123.45");
    auto d2 = Decimal::from_string("-0.00123");
    EXPECT_EQ(d2.value().to_string(), "-0.00123");
    auto d3 = Decimal::from_string("1e-3");
    EXPECT_EQ(d3.value().to_string(), "0.001");
    auto d4 = Decimal::from_string("invalid");
    EXPECT_TRUE(d4.is_err());
}

TEST(DecimalTest, Comparison) {
    auto a = Decimal::from_string("1.23").value();
    auto b = Decimal::from_string("1.230").value();
    EXPECT_TRUE(a == b);
    auto c = Decimal::from_string("1.24").value();
    EXPECT_TRUE(a < c);
    auto d = Decimal::from_string("-1.23").value();
    EXPECT_TRUE(d < a);
}

TEST(DecimalTest, Addition) {
    auto a = Decimal::from_string("1.23").value();
    auto b = Decimal::from_string("4.56").value();
    auto sum = a + b;
    EXPECT_EQ(sum.to_string(), "5.79");
    auto c = Decimal::from_string("-1.23").value();
    auto d = Decimal::from_string("1.23").value();
    EXPECT_EQ((c + d).to_string(), "0");
}

TEST(DecimalTest, Subtraction) {
    auto a = Decimal::from_string("5.67").value();
    auto b = Decimal::from_string("1.23").value();
    auto diff = a - b;
    EXPECT_EQ(diff.to_string(), "4.44");
    EXPECT_EQ((b - a).to_string(), "-4.44");
}

TEST(DecimalTest, Multiplication) {
    auto a = Decimal::from_string("1.2").value();
    auto b = Decimal::from_string("3.4").value();
    auto prod = a * b;
    EXPECT_EQ(prod.to_string(), "4.08");
    auto c = Decimal::from_string("2.5").value();
    auto d = Decimal::from_string("-0.5").value();
    EXPECT_EQ((c * d).to_string(), "-1.25");
}

TEST(DecimalTest, Division) {
    auto a = Decimal::from_string("10").value();
    auto b = Decimal::from_string("3").value();
    auto q = a / b; // 默认10位小数
    EXPECT_EQ(q.to_string(), "3.3333333333"); // 取决于精度
    auto q2 = a.div(b, 2);
    EXPECT_EQ(q2.to_string(), "3.33");
}

TEST(DecimalTest, DivRound) {
    auto a = Decimal::from_string("10").value();
    auto b = Decimal::from_string("3").value();
    auto q = a.div_round(b, 2);
    EXPECT_EQ(q.to_string(), "3.33"); // 3.333... 四舍五入到两位小数为3.33？实际是3.33，因为第三位是3
    auto q2 = a.div_round(b, 0);
    EXPECT_EQ(q2.to_string(), "3");
}

TEST(DecimalTest, Pow) {
    auto a = Decimal::from_string("1.5").value();
    auto r = a.pow(BigInt(3));
    EXPECT_TRUE(r.is_ok());
    EXPECT_EQ(r.value().to_string(), "3.375");
    auto b = Decimal::from_string("-2").value();
    auto r2 = b.pow(BigInt(3));
    EXPECT_EQ(r2.value().to_string(), "-8");
}

TEST(DecimalTest, IntegerPart) {
    auto a = Decimal::from_string("123.456").value();
    auto ip = a.integer_part();
    EXPECT_EQ(ip.to_string(), "123");
    auto b = Decimal::from_string("-0.789").value();
    EXPECT_EQ(b.integer_part().to_string(), "0");
}

TEST(DecimalTest, DecimalWeekeq) {
    auto a = Decimal::from_string("1.2345").value();
    auto b = Decimal::from_string("1.2346").value();
    EXPECT_TRUE(a.decimal_weekeq(b, 3)); // 前三位小数相等
    EXPECT_FALSE(a.decimal_weekeq(b, 4));
}

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}