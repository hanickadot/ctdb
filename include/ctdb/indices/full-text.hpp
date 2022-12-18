#ifndef CTDB_INDICES_FULLTEXT_HPP
#define CTDB_INDICES_FULLTEXT_HPP

#include <limits>
#include <string_view>
#include <cassert>
#include <concepts>

namespace ctdb {

struct whole_record_as_string {
	constexpr auto operator()(const auto & anything) noexcept {
		return std::string_view(anything);
	}
};

template <size_t N> constexpr inline size_t ngram_generate_count(size_t input_size) noexcept {
	static_assert(N > 0z);

	if (input_size < N) {
		return 0;
	}

	return (input_size - (N - 1z));
}

template <size_t N> constexpr inline size_t ngram_search_count(size_t input_size) noexcept {
	static_assert(N > 0z);

	if (input_size < N) {
		return 0;
	}

	return (input_size + (N - 1z)) / N;
}

template <size_t N> struct ngram {
	using value_type = std::array<char, N>;

	value_type value;
	unsigned position;

protected:
	static auto convert_pointer_to_value(const char * ptr) noexcept -> value_type {
		// copy N bytes into array
		return [&]<size_t... Indices>(std::index_sequence<Indices...>) {
			return value_type{ptr[Indices]...};
		}
		(std::make_index_sequence<N>());
	}

public:
	constexpr ngram(const char * source, unsigned pos) noexcept: value{convert_pointer_to_value(source)}, position{pos} { }

	friend constexpr bool operator==(ngram, ngram) noexcept = default;
};

struct ngram_sentinel {
	size_t length;
};

template <size_t N> struct ngram_iterator {
	const char * base;
	size_t position{0};

	using value_type = ngram<N>;
	using difference_type = ssize_t;

	constexpr auto operator*() const noexcept {
		assert(position <= (std::numeric_limits<unsigned>::max)());
		return ngram<N>{base + position, static_cast<unsigned>(position)};
	}

	constexpr friend bool operator==(ngram_iterator lhs, ngram_sentinel rhs) noexcept {
		return (lhs.position + N) > rhs.length;
	}

	constexpr friend bool operator==(ngram_iterator lhs, ngram_iterator rhs) noexcept {
		return std::tie(lhs.base, lhs.position) == std::tie(rhs.base, rhs.position);
	}

	constexpr friend difference_type operator-(ngram_sentinel lhs, ngram_iterator rhs) noexcept {
		return static_cast<difference_type>(lhs.length) - static_cast<difference_type>(rhs.position);
	}

	constexpr ngram_iterator & operator++() noexcept {
		++position;
		return *this;
	}

	constexpr ngram_iterator operator++(int) noexcept {
		ngram_iterator previous{*this};
		++position;
		return previous;
	}
};

template <size_t N> struct view_as_ngrams {
	using value_type = ngram<N>;
	std::string_view input;

	constexpr view_as_ngrams(std::string_view in) noexcept: input{in} { }

	constexpr size_t size() const noexcept {
		return ngram_generate_count<N>(input.size());
	}

	constexpr auto begin() const noexcept {
		return ngram_iterator<N>{input.data()};
	}

	constexpr auto end() const noexcept {
		return ngram_sentinel{input.size()};
	}
};

struct contains_string {
	std::string_view input;

	explicit constexpr contains_string(std::string_view in) noexcept: input{in} { }
};

template <typename Extractor = whole_record_as_string, size_t N = 4> struct basic_fulltext {
	std::string_view input;

	explicit constexpr basic_fulltext(std::string_view) noexcept {
	}

	// iterator generating ngrams
	constexpr auto begin() {
	}

	constexpr auto end() {
	}
};

// input:  "charlotta"
// ngrams: "cha" "har" "arl" "rlo" "lot" "ott" "tta"
// relpos:  n    +1    +2    +3    +4    +5    +6
// needed: "cha"             "rlo"             "tta"
// relpos:  n                +3                +6

// first search is just ngram value which yield pkey and position
// second search is ngram+pkey+(position+1)
// third is ngram+pkey+(position+2)

// template <typename Extractor, size_t N> struct index_storage_traits<basic_fulltext<Extractor, N>> {
//	template <typename PKey> struct entry {
//		std::array<char, N> ngram;
//		PKey pkey;
//		unsigned position;
//	};
//
//	template <typename PKey> using storage_type = std::set<entry<PKey>, non_unique_comparator<T, entry<PKey>>>;
//
//	template <typename Other> static constexpr bool compatible_type = std::convertible_into<contains_string>;
// };

} // namespace ctdb

#endif
