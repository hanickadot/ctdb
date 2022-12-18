#include <ctdb/table.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("table") {
	ctdb::table<std::string> tbl;

	const auto a = tbl.emplace("hello");
	const auto b = tbl.emplace("there");

	REQUIRE(a != std::nullopt);
	REQUIRE(b != std::nullopt);

	REQUIRE(a != b);

	REQUIRE(tbl.size() == 2z);

	const auto c = tbl.emplace("hello");

	REQUIRE(c != std::nullopt);
	REQUIRE(tbl.size() == 3z);
}

struct number_of_character {
	size_t sz;
	explicit constexpr number_of_character(std::string_view in) noexcept: sz{in.size()} { }

	constexpr friend bool operator==(number_of_character, number_of_character) noexcept = default;
	constexpr friend auto operator<=>(number_of_character, number_of_character) noexcept = default;
};

TEST_CASE("table with special index") {
	ctdb::table<std::string, ctdb::sorted<number_of_character>> tbl;

	tbl.emplace("aaaaaaa7");
	tbl.emplace("bbbbbb6");
	tbl.emplace("ccccc5");
	tbl.emplace("dddd4");
	tbl.emplace("eee3");
	tbl.emplace("ff2");
	tbl.emplace("g1");

	REQUIRE(tbl.size<number_of_character>() == 7);

	{
		std::string tmp{};

		REQUIRE(tbl.all<number_of_character>().size() == 7); // slower

		for (const auto & item: tbl.all<number_of_character>().ascending()) {
			tmp += item + ".";
		}

		REQUIRE(tmp == "g1.ff2.eee3.dddd4.ccccc5.bbbbbb6.aaaaaaa7.");
	}

	{
		std::string tmp{};

		REQUIRE(tbl.all<number_of_character>().descending().size() == 7); // slower

		for (const auto & item: tbl.all<number_of_character>().descending()) {
			tmp += item + ".";
		}

		REQUIRE(tmp == "aaaaaaa7.bbbbbb6.ccccc5.dddd4.eee3.ff2.g1.");
	}
}

TEST_CASE("unique_index (sorted)") {
	ctdb::table<std::string, ctdb::unique_sorted<std::string_view>> tbl;

	const auto a = tbl.emplace("hello");
	const auto b = tbl.emplace("there");

	REQUIRE(a != std::nullopt);
	REQUIRE(b != std::nullopt);

	REQUIRE(a != b);

	REQUIRE(tbl.size() == 2z);

	const auto c = tbl.emplace("hello");

	REQUIRE(c == std::nullopt);
	REQUIRE(tbl.size() == 2z);

	const auto d = tbl.emplace("hana");

	REQUIRE(d != std::nullopt);
	REQUIRE(tbl.size() == 3z);
}

TEST_CASE("unique_index (unsorted)") {
	ctdb::table<std::string, ctdb::unique<std::string_view>> tbl;

	const auto a = tbl.emplace("hello");
	const auto b = tbl.emplace("there");

	REQUIRE(a != std::nullopt);
	REQUIRE(b != std::nullopt);

	REQUIRE(a != b);

	REQUIRE(tbl.size() == 2z);

	const auto c = tbl.emplace("hello");

	REQUIRE(c == std::nullopt);
	REQUIRE(tbl.size() == 2z);

	const auto d = tbl.emplace("hana");

	REQUIRE(d != std::nullopt);
	REQUIRE(tbl.size() == 3z);
}

template <typename> struct identify;
template <typename T> auto id(T && val) -> identify<T>;

TEST_CASE("sorted by string_view") {
	ctdb::table<std::string, ctdb::sorted<std::string_view>> tbl;

	tbl.emplace("z");
	tbl.emplace("d");
	tbl.emplace("a");
	tbl.emplace("b");
	tbl.emplace("k");
	tbl.emplace("c");

	REQUIRE(tbl.size() == 6);

	const auto rng = tbl.all<std::string_view>();

	auto it = rng.begin();
	const auto end = rng.end();

	REQUIRE(it != end);
	REQUIRE(*it == "a");
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == "b");
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == "c");
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == "d");
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == "k");
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == "z");
	++it;

	REQUIRE(it == end);

	std::string tmp{};

	for (const auto & item: tbl.all<std::string_view>()) {
		tmp += item;
	}

	REQUIRE(tmp == "abcdkz");
}

TEST_CASE("sorted by string_view (reversed)") {
	ctdb::table<std::string, ctdb::sorted<std::string_view>> tbl;

	tbl.emplace("z");
	tbl.emplace("d");
	tbl.emplace("a");
	tbl.emplace("b");
	tbl.emplace("k");
	tbl.emplace("c");

	REQUIRE(tbl.size() == 6);

	REQUIRE(tbl.size<std::string_view>() == 6);

	const auto rng = tbl.all<std::string_view>().descending();

	auto it = rng.begin();
	const auto end = rng.end();

	REQUIRE(it != end);
	REQUIRE(*it == "z");
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == "k");
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == "d");
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == "c");
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == "b");
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == "a");
	++it;

	REQUIRE(it == end);

	std::string tmp{};

	for (const auto & item: tbl.all<std::string_view>().descending()) {
		tmp += item;
	}

	REQUIRE(tmp == "zkdcba");
}

template <char c> struct contains {
	bool value;

	// conversion from record type
	explicit constexpr contains(std::string_view input) noexcept: value{input.find(c) != input.npos} { }

	// constructor for search queries
	explicit constexpr contains(bool v) noexcept: value{v} { }

	// provide comparison (why this is not default?)
	constexpr friend bool operator==(contains, contains) noexcept = default;
	constexpr friend auto operator<=>(contains, contains) noexcept = default;
};

TEST_CASE("containing 'a' index") {
	ctdb::table<std::string, contains<'a'>> tbl;

	tbl.emplace("c++");
	tbl.emplace("bebe");
	tbl.emplace("hehe");
	tbl.emplace("aloha");
	tbl.emplace("ahoj");

	REQUIRE(tbl.size() == 5);
	REQUIRE(tbl.size<contains<'a'>>() == 5);

	// all without 'a' inside the record
	const auto rng_false = (tbl == contains<'a'>(false));
	REQUIRE(rng_false.size() == 3);

	// all with 'a' inside the record
	const auto rng = (tbl == contains<'a'>(true));
	REQUIRE(rng.size() == 2);

	auto it = rng.begin();
	const auto end = rng.end();

	REQUIRE(it != end);
	const auto & first = *it;
	const bool first_match = (*it == "aloha") || (*it == "ahoj"); // order unspecified
	REQUIRE(first_match);
	++it;

	REQUIRE(it != end);
	const bool second_match = (*it == "aloha") || (*it == "ahoj"); // order unspecified
	REQUIRE(second_match);
	REQUIRE(*it != first); // but it can't be same as first one :)
	++it;

	REQUIRE(it == end);
}