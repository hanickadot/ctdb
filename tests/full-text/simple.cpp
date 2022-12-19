#include <ctdb/indices/full-text.hpp>
#include <iostream>
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("simple fulltext") {
	std::set<std::string, std::less<void>> strings;

	ctdb::simple_fulltext_reverse_index<decltype(strings)::iterator, 4> index;

	auto add = [&](std::string_view in) {
		auto [it, success] = strings.emplace(std::string{in});
		assert(success);
		index.emplace(in, it);
	};

	auto remove = [&](std::string_view in) {
		if (auto it = strings.find(in); it != strings.end()) {
			index.remove(in, it);
			strings.erase(it);
		}
	};

	REQUIRE(index.ngram_known() == 0z);
	REQUIRE(index.ngram_count() == 0z);

	add("xxcharlotte");
	add("pokus");
	add("hana is owner of charlotte the dog");
	add("some charlatan");
	add("charchar");
	add("-charchar");
	add("charcoal");
	add("charlotte is the best dog");
	add("this is really long text, this is really long text, this is really long text, lorem ipsum, whatever, hana");
	add("šarlota is charlotte");
	add("charlotte is the charlotte");
	add("data and lore are androids");

	REQUIRE(index.find_all("char"sv).size() == 12z);

	remove("-charchar");

	REQUIRE(index.find_all("char"sv).size() == 10z);

	// REQUIRE(index.ngram_known() == 138z);
	// REQUIRE(index.ngram_count() == 250z);

	auto search = [&](std::string_view query) {
		std::cout << "searching for '" << query << "':\n";
		unsigned id = 0;
		const auto result = index.find_all(query);
		if (result.empty()) {
			std::cout << "nothing was found\n";
		} else {
			for (auto e: result) {
				std::cout << ++id << ": " << std::addressof(*e.pkey) << " '" << *e.pkey << "'@" << e.position << ".." << (e.position + query.size()) << "\n";
			}
		}
	};

	search("lotte is");
	search("lorem ipsum");
	search("hana");
	search("is the");
	search("lly long text, this is really long text, this is");
}