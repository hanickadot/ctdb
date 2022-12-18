#ifndef CTDB_INDICES_FULLTEXT_HPP
#define CTDB_INDICES_FULLTEXT_HPP

#include <algorithm>
#include <limits>
#include <map>
#include <ostream>
#include <set>
#include <string_view>
#include <cassert>
#include <concepts>

// iostream
#include <iostream>

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

template <typename PKey, size_t N> struct simple_fulltext_reverse_index {
	struct entry {
		PKey pkey;
		unsigned position;

		constexpr entry(PKey pk, unsigned pos) noexcept: pkey{pk}, position{pos} { }

		friend constexpr bool operator==(const entry & lhs, const entry & rhs) noexcept {
			const auto lhs_addr = std::addressof(*lhs.pkey);
			const auto rhs_addr = std::addressof(*rhs.pkey);

			return std::tie(lhs_addr, lhs.position) == std::tie(rhs_addr, rhs.position);
		}

		friend constexpr auto operator<=>(const entry & lhs, const entry & rhs) noexcept {
			const auto lhs_addr = std::addressof(*lhs.pkey);
			const auto rhs_addr = std::addressof(*rhs.pkey);

			return std::tie(lhs_addr, lhs.position) <=> std::tie(rhs_addr, rhs.position);
		}

		friend std::ostream & operator<<(std::ostream & os, const entry & e) {
			return os << std::addressof(*e.pkey) << " '" << *e.pkey << "'@" << e.value.position;
		}
	};

	std::map<ngram<N>, std::set<entry>> data;

	auto emplace(view_as_ngrams<N> ngrams, PKey pkey) {
		for (auto ngram: ngrams) {
			auto it = data.lower_bound(ngram.value);

			if (it->first == ngram.value) {
				it->second.emplace(pkey, ngram.position);
			} else {
				auto it2 = data.emplace_hint(it, ngram.value, std::set<entry>{});
				it2->second.emplace(pkey, ngram.position);
			}
		}
	}

	auto remove(view_as_ngrams<N> ngrams, PKey pkey) {
		for (auto ngram: ngrams) {
			auto it = data.find(ngram.value);
			assert(it != data.end());

			it->second.erase(entry{pkey, ngram.position});

			if (it->second.empty()) {
				data.erase(it);
			}
		}
	}

	static constexpr auto convert_to_vector(auto && range) {
		using value_type = std::remove_cvref_t<decltype(*range.begin())>;

		std::vector<value_type> result{};

		result.reserve(range.size());

		for (auto && value: range) {
			result.emplace_back(std::move(value));
		}

		return result;
	}

	auto find_ngram_occurences(ngram<N> value) const noexcept -> const std::set<entry> * {
		if (const auto it = data.find(value); it != data.end()) {
			return std::addressof(it->second);
		} else {
			return nullptr;
		}
	}

	auto find_first(view_as_ngrams<N> input) -> std::optional<PKey> {
		std::cout << "SEARCHING FOR '" << input.input << "' ngram = " << input.size() << "\n";

		std::map<entry, unsigned> candidates;

		// TODO optimization

		// total complexity is O(len * (log(n) + k * log(k)))

		// O(len)
		for (auto ng: input) {
			// find ngram set O(log n)
			const std::set<entry> * occurences = find_ngram_occurences(ng.value);

			if (occurences == nullptr) {
				std::cout << "not found [some of ngrams doesn't exists!]\n";
				return std::nullopt;
			}

			// increment each found item k * log(k)
			for (const auto & e: *occurences) {
				auto [it, success] = candidates.try_emplace(entry{e.pkey, e.position - ng.position}, 1);

				if (!success) {
					it->second++;
				}
			}
		}

		std::cout << "found\n";

		for (auto [e, count]: candidates) {
			if (count == input.size()) {
				std::cout << "  " << std::addressof(*e.pkey) << " '" << *e.pkey << "'@" << e.position << " (hit_count = " << count << ")\n";
			}
		}

		return std::nullopt;
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
