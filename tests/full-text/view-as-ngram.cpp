#include <ctdb/indices/full-text.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("view_as_ngram basic") {
	constexpr auto in = "aloha"sv;

	const auto rng = ctdb::view_as_ngrams<3>(in);

	REQUIRE(rng.size() == 3);

	auto it = rng.begin();
	const auto end = rng.end();

	REQUIRE(it != end);
	REQUIRE(*it == ctdb::ngram_with_position<3>{"alo", 0});
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == ctdb::ngram_with_position<3>{"loh", 1});
	++it;

	REQUIRE(it != end);
	REQUIRE(*it == ctdb::ngram_with_position<3>{"oha", 2});
	++it;

	REQUIRE(it == end);

	static_assert(std::forward_iterator<decltype(it)>);
	REQUIRE(std::forward_iterator<decltype(it)>);

	// try if we can iterate it
	std::vector<ctdb::ngram_with_position<3>> vec;

	for (auto ngram: rng) {
		vec.push_back(ngram);
	}

	REQUIRE(vec == std::vector<ctdb::ngram_with_position<3>>{{"alo", 0}, {"loh", 1}, {"oha", 2}});
}

constexpr auto convert_to_vector(auto && range) {
	using value_type = std::remove_cvref_t<decltype(*range.begin())>;

	std::vector<value_type> result{};

	for (auto && value: range) {
		result.emplace_back(std::move(value));
	}

	return result;
}

TEST_CASE("view_as_ngram basic (charlotte, 2)") {
	constexpr auto input = "charlotte";

	using ngram_view = ctdb::view_as_ngrams<2>;

	REQUIRE(ngram_view(input).size() == 8z);

	const auto list = convert_to_vector(ngram_view(input));

	REQUIRE(list == std::vector<ngram_view::value_type>{{"ch", 0}, {"ha", 1}, {"ar", 2}, {"rl", 3}, {"lo", 4}, {"ot", 5}, {"tt", 6}, {"te", 7}});
}

TEST_CASE("view_as_ngram basic (charlotte, 3)") {
	constexpr auto input = "charlotte";

	using ngram_view = ctdb::view_as_ngrams<3>;

	REQUIRE(ngram_view(input).size() == 7z);

	const auto list = convert_to_vector(ngram_view(input));

	REQUIRE(list == std::vector<ngram_view::value_type>{{"cha", 0}, {"har", 1}, {"arl", 2}, {"rlo", 3}, {"lot", 4}, {"ott", 5}, {"tte", 6}});
}

TEST_CASE("view_as_ngram basic (charlotte, 4)") {
	constexpr auto input = "charlotte";

	using ngram_view = ctdb::view_as_ngrams<4>;

	REQUIRE(ngram_view(input).size() == 6z);

	const auto list = convert_to_vector(ngram_view(input));

	REQUIRE(list == std::vector<ngram_view::value_type>{{"char", 0}, {"harl", 1}, {"arlo", 2}, {"rlot", 3}, {"lott", 4}, {"otte", 5}});
}

TEST_CASE("view_as_ngram basic (charlotte, 5)") {
	constexpr auto input = "charlotte";

	using ngram_view = ctdb::view_as_ngrams<5>;

	REQUIRE(ngram_view(input).size() == 5z);

	const auto list = convert_to_vector(ngram_view(input));

	REQUIRE(list == std::vector<ngram_view::value_type>{{"charl", 0}, {"harlo", 1}, {"arlot", 2}, {"rlott", 3}, {"lotte", 4}});
}
