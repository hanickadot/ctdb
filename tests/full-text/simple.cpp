#include <ctdb/indices/full-text.hpp>
#include <iostream>
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("simple fulltext") {
	std::set<std::string> strings;

	ctdb::simple_fulltext_reverse_index<decltype(strings)::iterator, 4> index;

	auto add = [&](std::string_view in) {
		auto [it, success] = strings.emplace(std::string{in});
		assert(success);
		index.emplace(in, it);
	};

	add("xxcharlotte");
	add("pokus");
	add("hana");
	add("some charlatan");
	add("charchar");
	add("-charchar");
	add("charcoal");
	add("charlotte is the best dog");
	add("šarlota is charlotte");
	add("charlotte is the charlotte");
	index.find_first("charl"sv);
}