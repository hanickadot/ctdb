#include <catch2/catch_test_macros.hpp>
#include <ctdb/ctdb.hpp>
#include <string>
#include <tuple>
#include <compare>

struct name_and_age {
	std::string name;
	unsigned age;

	friend constexpr bool operator==(const name_and_age & lhs, const name_and_age & rhs) noexcept {
		return std::make_tuple(std::string_view{lhs.name}, lhs.age) == std::make_tuple(std::string_view{rhs.name}, rhs.age);
	}
	friend constexpr auto operator<=>(const name_and_age & lhs, const name_and_age & rhs) noexcept {
		return std::make_tuple(std::string_view{lhs.name}, lhs.age) <=> std::make_tuple(std::string_view{rhs.name}, rhs.age);
	}
};

struct age {
	unsigned value;

	explicit constexpr age(unsigned v) noexcept: value{v} { }
	explicit constexpr age(const name_and_age & orig) noexcept: value{orig.age} { }

	friend constexpr bool operator==(age, age) noexcept = default;
	friend constexpr auto operator<=>(age, age) noexcept = default;

	friend constexpr bool operator==(const name_and_age & lhs, age rhs) noexcept {
		return lhs.age == rhs.value;
	}

	friend constexpr auto operator<=>(const name_and_age & lhs, age rhs) noexcept {
		return lhs.age <=> rhs.value;
	}
};

TEST_CASE("basic") {
	const auto a_person = name_and_age{"charlotte", 6};
	REQUIRE(a_person < age{14});
	REQUIRE(a_person == age{6});
	REQUIRE(a_person > age{5});
}