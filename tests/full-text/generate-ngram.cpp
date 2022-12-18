#include <ctdb/indices/full-text.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("ngram_generate_count (1)") {
	// zero for short strings
	REQUIRE(ctdb::ngram_generate_count<1>(0z) == 0z);

	// one for exactly ngram
	REQUIRE(ctdb::ngram_generate_count<1>(1z) == 1z);

	// linearly
	REQUIRE(ctdb::ngram_generate_count<1>(2z) == 2z);
	REQUIRE(ctdb::ngram_generate_count<1>(3z) == 3z);
	REQUIRE(ctdb::ngram_generate_count<1>(4z) == 4z);
	REQUIRE(ctdb::ngram_generate_count<1>(5z) == 5z);
	REQUIRE(ctdb::ngram_generate_count<1>(6z) == 6z);
	REQUIRE(ctdb::ngram_generate_count<1>(7z) == 7z);
	REQUIRE(ctdb::ngram_generate_count<1>(8z) == 8z);
}

TEST_CASE("ngram_generate_count (2)") {
	// zero for short strings
	REQUIRE(ctdb::ngram_generate_count<2>(0z) == 0z);
	REQUIRE(ctdb::ngram_generate_count<2>(1z) == 0z);

	// one for exactly ngram
	REQUIRE(ctdb::ngram_generate_count<2>(2z) == 1z);

	// rest...
	REQUIRE(ctdb::ngram_generate_count<2>(3z) == 2z);
	REQUIRE(ctdb::ngram_generate_count<2>(4z) == 3z);
	REQUIRE(ctdb::ngram_generate_count<2>(5z) == 4z);
	REQUIRE(ctdb::ngram_generate_count<2>(6z) == 5z);
	REQUIRE(ctdb::ngram_generate_count<2>(7z) == 6z);
}

TEST_CASE("ngram_generate_count (3)") {
	// zero for short strings
	REQUIRE(ctdb::ngram_generate_count<3>(0z) == 0z);
	REQUIRE(ctdb::ngram_generate_count<3>(1z) == 0z);
	REQUIRE(ctdb::ngram_generate_count<3>(2z) == 0z);

	// one for exactly ngram
	REQUIRE(ctdb::ngram_generate_count<3>(3z) == 1z);

	// rest...
	REQUIRE(ctdb::ngram_generate_count<3>(4z) == 2z);
	REQUIRE(ctdb::ngram_generate_count<3>(5z) == 3z);
	REQUIRE(ctdb::ngram_generate_count<3>(6z) == 4z);
	REQUIRE(ctdb::ngram_generate_count<3>(7z) == 5z);
	REQUIRE(ctdb::ngram_generate_count<3>(8z) == 6z);
	REQUIRE(ctdb::ngram_generate_count<3>(9z) == 7z);
}

TEST_CASE("ngram_generate_count (4)") {
	// zero for short strings
	REQUIRE(ctdb::ngram_generate_count<4>(0z) == 0z);
	REQUIRE(ctdb::ngram_generate_count<4>(1z) == 0z);
	REQUIRE(ctdb::ngram_generate_count<4>(2z) == 0z);
	REQUIRE(ctdb::ngram_generate_count<4>(3z) == 0z);

	// one for exactly ngram
	REQUIRE(ctdb::ngram_generate_count<4>(4z) == 1z);

	// rest...
	REQUIRE(ctdb::ngram_generate_count<4>(5z) == 2z);
	REQUIRE(ctdb::ngram_generate_count<4>(6z) == 3z);
	REQUIRE(ctdb::ngram_generate_count<4>(7z) == 4z);
	REQUIRE(ctdb::ngram_generate_count<4>(8z) == 5z);
	REQUIRE(ctdb::ngram_generate_count<4>(9z) == 6z);
	REQUIRE(ctdb::ngram_generate_count<4>(10z) == 7z);
	REQUIRE(ctdb::ngram_generate_count<4>(11z) == 8z);
	REQUIRE(ctdb::ngram_generate_count<4>(12z) == 9z);
}