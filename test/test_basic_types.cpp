#include <catch.hpp>
#include <zen_serialization/archive.h>

using namespace zen;

template <typename TOut, typename TIn>
void test_integral_types()
{
    // 测试各种整数类型
    bool bool_value = true;
    char char_value = 'A';
    short short_value = -100;
    int int_value = -1000;
    long long_value = -100000L;
    long long long_long_value = -10000000000LL;
    unsigned char uchar_value = 200;
    unsigned short ushort_value = 60000;
    unsigned int uint_value = 4000000000U;
    unsigned long ulong_value = 1234567890UL;
    unsigned long long ulong_long_value = 12345678901234567890ULL;

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("bool_value", bool_value));
    oar(make_nvp("char_value", char_value));
    oar(make_nvp("short_value", short_value));
    oar(make_nvp("int_value", int_value));
    oar(make_nvp("long_value", long_value));
    oar(make_nvp("long_long_value", long_long_value));
    oar(make_nvp("uchar_value", uchar_value));
    oar(make_nvp("ushort_value", ushort_value));
    oar(make_nvp("uint_value", uint_value));
    oar(make_nvp("ulong_value", ulong_value));
    oar(make_nvp("ulong_long_value", ulong_long_value));
    oar.Flush();

    // 反序列化
    bool bool_value_out;
    char char_value_out;
    short short_value_out;
    int int_value_out;
    long long_value_out;
    long long long_long_value_out;
    unsigned char uchar_value_out;
    unsigned short ushort_value_out;
    unsigned int uint_value_out;
    unsigned long ulong_value_out;
    unsigned long long ulong_long_value_out;

    InArchive iar{TIn(ss)};
    iar(make_nvp("bool_value", bool_value_out));
    iar(make_nvp("char_value", char_value_out));
    iar(make_nvp("short_value", short_value_out));
    iar(make_nvp("int_value", int_value_out));
    iar(make_nvp("long_value", long_value_out));
    iar(make_nvp("long_long_value", long_long_value_out));
    iar(make_nvp("uchar_value", uchar_value_out));
    iar(make_nvp("ushort_value", ushort_value_out));
    iar(make_nvp("uint_value", uint_value_out));
    iar(make_nvp("ulong_value", ulong_value_out));
    iar(make_nvp("ulong_long_value", ulong_long_value_out));

    // 验证
    CHECK(bool_value_out == bool_value);
    CHECK(char_value_out == char_value);
    CHECK(short_value_out == short_value);
    CHECK(int_value_out == int_value);
    CHECK(long_value_out == long_value);
    CHECK(long_long_value_out == long_long_value);
    CHECK(uchar_value_out == uchar_value);
    CHECK(ushort_value_out == ushort_value);
    CHECK(uint_value_out == uint_value);
    CHECK(ulong_value_out == ulong_value);
    CHECK(ulong_long_value_out == ulong_long_value);
}

template <typename TOut, typename TIn>
void test_floating_point_types()
{
    // 测试浮点类型
    float float_value = 3.14159f;
    double double_value = 2.718281828459045;
    long double long_double_value = 1.618033988749895L;

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("float_value", float_value));
    oar(make_nvp("double_value", double_value));
    oar(make_nvp("long_double_value", long_double_value));
    oar.Flush();

    // 反序列化
    float float_value_out;
    double double_value_out;
    long double long_double_value_out;

    InArchive iar{TIn(ss)};
    iar(make_nvp("float_value", float_value_out));
    iar(make_nvp("double_value", double_value_out));
    iar(make_nvp("long_double_value", long_double_value_out));

    // 验证
    CHECK(float_value_out == float_value);
    CHECK(double_value_out == double_value);
    CHECK(long_double_value_out == long_double_value);
}

TEST_CASE("basic_types", "[basic]")
{
    test_integral_types<JsonSerializer, JsonDeserializer>();
    test_integral_types<BinarySerializer, BinaryDeserializer>();
    test_floating_point_types<JsonSerializer, JsonDeserializer>();
    test_floating_point_types<BinarySerializer, BinaryDeserializer>();
}