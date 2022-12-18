# Compile Time DataBase (CTDB)

Simple relational database defined as a type.

Simple motivational example:

```c++
struct number_of_character {
	size_t sz;
	explicit constexpr number_of_character(size_t s) noexcept: sz{s} { }
	explicit constexpr number_of_character(std::string_view in) noexcept: sz{in.size()} { }

	constexpr friend bool operator==(number_of_character, number_of_character) noexcept = default;
	constexpr friend auto operator<=>(number_of_character, number_of_character) noexcept = default;
};

ctdb::table<std::string, number_of_characters> tbl;

// this will find range based on `number_of_characters` index in O(log n)
for (const std::string & record: tbl.equal(number_of_character(4)) {
	std::cout << record << "\n";
}

```