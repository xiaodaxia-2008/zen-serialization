#include <catch.hpp>
#include <zen_serialization/archive.h>
#include <complex>

using namespace zen;

template <typename TOut, typename TIn>
void test_complex()
{
    // 测试基本的complex类型
    std::complex<double> c1(3.14, 2.71);
    std::complex<double> c2(-1.5, 4.2);
    std::complex<double> c3(0.0, 0.0);
    std::complex<double> c4(5.5, 0.0);  // 纯实数
    std::complex<double> c5(0.0, -3.3); // 纯虚数

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("c1", c1));
    oar(make_nvp("c2", c2));
    oar(make_nvp("c3", c3));
    oar(make_nvp("c4", c4));
    oar(make_nvp("c5", c5));
    oar.Flush();

    std::complex<double> c1_out, c2_out, c3_out, c4_out, c5_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("c1", c1_out));
    iar(make_nvp("c2", c2_out));
    iar(make_nvp("c3", c3_out));
    iar(make_nvp("c4", c4_out));
    iar(make_nvp("c5", c5_out));

    CHECK(c1_out == c1);
    CHECK(c2_out == c2);
    CHECK(c3_out == c3);
    CHECK(c4_out == c4);
    CHECK(c5_out == c5);
}

template <typename TOut, typename TIn>
void test_complex_float()
{
    // 测试float类型的complex
    std::complex<float> c1(3.14f, 2.71f);
    std::complex<float> c2(-1.5f, 4.2f);

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("c1", c1));
    oar(make_nvp("c2", c2));
    oar.Flush();

    std::complex<float> c1_out, c2_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("c1", c1_out));
    iar(make_nvp("c2", c2_out));

    CHECK(c1_out == c1);
    CHECK(c2_out == c2);
}

template <typename TOut, typename TIn>
void test_complex_long_double()
{
    // 测试long double类型的complex
    std::complex<long double> c1(3.141592653589793238L, 2.718281828459045235L);
    std::complex<long double> c2(-1.5L, 4.2L);

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("c1", c1));
    oar(make_nvp("c2", c2));
    oar.Flush();

    std::complex<long double> c1_out, c2_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("c1", c1_out));
    iar(make_nvp("c2", c2_out));

    CHECK(c1_out == c1);
    CHECK(c2_out == c2);
}

template <typename TOut, typename TIn>
void test_vector_complex()
{
    // 测试complex类型的容器
    std::vector<std::complex<double>> complex_vector = {
        {1.1, 2.2},
        {3.3, 4.4},
        {-1.0, -2.0},
        {0.0, 0.0},
        {5.5, 0.0}
    };

    std::stringstream ss;
    OutArchive oar{TOut(ss)};
    oar(make_nvp("complex_vector", complex_vector));
    oar.Flush();

    std::vector<std::complex<double>> complex_vector_out;
    InArchive iar{TIn(ss)};
    iar(make_nvp("complex_vector", complex_vector_out));

    CHECK(complex_vector_out == complex_vector);
}

TEST_CASE("complex", "[complex]")
{
    test_complex<JsonSerializer, JsonDeserializer>();
    test_complex<BinarySerializer, BinaryDeserializer>();
    
    test_complex_float<JsonSerializer, JsonDeserializer>();
    test_complex_float<BinarySerializer, BinaryDeserializer>();
    
    test_complex_long_double<JsonSerializer, JsonDeserializer>();
    test_complex_long_double<BinarySerializer, BinaryDeserializer>();
    
    test_vector_complex<JsonSerializer, JsonDeserializer>();
    test_vector_complex<BinarySerializer, BinaryDeserializer>();
}