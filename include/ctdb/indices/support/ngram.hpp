#ifndef CTDB_INDICES_SUPPORT_NGRAM_HPP
#define CTDB_INDICES_SUPPORT_NGRAM_HPP

#include <array>
#include <ostream>
#include <cassert>
#include <cstddef>

namespace ctdb::support {

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

protected:
	static auto convert_pointer_to_value(const char * ptr) noexcept -> value_type {
		// copy N bytes into array
		return [&]<size_t... Indices>(std::index_sequence<Indices...>) {
			return value_type{ptr[Indices]...};
		}
		(std::make_index_sequence<N>());
	}

public:
	constexpr ngram(const char * source) noexcept: value{convert_pointer_to_value(source)} { }

	friend constexpr bool operator==(ngram, ngram) noexcept = default;
	friend constexpr auto operator<=>(ngram lhs, ngram rhs) noexcept {
		// clang doesn't have lexigraphical_three_way_compare yet
		for (int i = 0; i != int(N); ++i) {
			const auto r = lhs.value[i] <=> rhs.value[i];
			if (!is_eq(r)) {
				return r;
			}
		}
		return std::strong_ordering::equivalent;
	}

	friend std::ostream & operator<<(std::ostream & os, const ngram & e) {
		return os << "'" << std::string_view{e.value.data(), e.value.size()} << "'";
	}
};

template <size_t N> struct ngram_with_position {
	using value_type = ngram<N>;

	value_type value;
	unsigned position;

	constexpr ngram_with_position(value_type val, unsigned pos) noexcept: value{val}, position{pos} { }

	friend constexpr bool operator==(ngram_with_position, ngram_with_position) noexcept = default;
	friend constexpr auto operator<=>(ngram_with_position, ngram_with_position) noexcept = default;
};

struct ngram_sentinel {
	size_t length;
};

template <size_t N> struct ngram_iterator {
	const char * base;
	size_t position{0};

	using value_type = ngram_with_position<N>;
	using difference_type = ssize_t;

	constexpr auto operator*() const noexcept {
		assert(position <= (std::numeric_limits<unsigned>::max)());
		return value_type{base + position, static_cast<unsigned>(position)};
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
	using value_type = ngram_with_position<N>;
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

static constexpr auto convert_to_vector(auto && range) {
	using value_type = std::remove_cvref_t<decltype(*range.begin())>;

	std::vector<value_type> result{};

	result.reserve(range.size());

	for (auto && value: range) {
		result.emplace_back(std::move(value));
	}

	return result;
}

} // namespace ctdb::support

#endif
