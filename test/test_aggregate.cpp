
#include <zen_serialization/aggregate.h>
#include <zen_serialization/archive.h>

#include <catch.hpp>

using namespace zen;

struct Agg {
	int a{0};
	std::string b{""};
	double c{0.0};
};

template <typename TOut, typename TIn>
void test_aggregate()
{
	Agg in{.a = 42, .b = "hello", .c = 3.14159};
	Agg out{};

	std::stringstream ss;
	OutArchive oar{TOut{ss}};
	oar(make_nvp("agg", in));
	oar.Flush();

	InArchive iar{TIn{ss}};
	iar(make_nvp("agg", out));

	REQUIRE(out.a == in.a);
	REQUIRE(out.b == in.b);
	CHECK_THAT(out.c, Catch::Matchers::WithinAbs(in.c, 0.0001));
}

TEST_CASE("aggregate", "[aggregate]")
{
	test_aggregate<JsonSerializer, JsonDeserializer>();
	test_aggregate<BinarySerializer, BinaryDeserializer>();
}
