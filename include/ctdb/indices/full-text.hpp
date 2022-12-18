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

	size_t count{0z};
	std::map<ngram<N>, std::set<entry>> data;

	size_t ngram_known() const noexcept {
		return data.size();
	}

	size_t ngram_count() const noexcept {
		return count;
	}

	auto emplace(view_as_ngrams<N> ngrams, PKey pkey) {
		for (auto ngram: ngrams) {
			auto it = data.lower_bound(ngram.value);

			if (it->first == ngram.value) {
				it->second.emplace(pkey, ngram.position);
			} else {
				auto it2 = data.emplace_hint(it, ngram.value, std::set<entry>{});
				it2->second.emplace(pkey, ngram.position);
			}

			++count;
		}
	}

	auto remove(view_as_ngrams<N> ngrams, PKey pkey) {
		for (auto ngram: ngrams) {
			auto it = data.find(ngram.value);
			assert(it != data.end());

			it->second.erase(entry{pkey, ngram.position});
			--count;

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

	static auto intersect(const std::set<entry> & lhs, unsigned lhs_offset, const std::set<entry> & rhs, unsigned rhs_offset) -> std::set<entry> {
		std::set<entry> result;

		// use shorter for linear scan
		// O(a * log(b))
		if (lhs.size() <= rhs.size()) {
			for (const auto & lhs_entry: lhs) {
				if (rhs.contains(entry{lhs_entry.pkey, lhs_entry.position - lhs_offset + rhs_offset})) {
					result.emplace(entry{lhs_entry.pkey, lhs_entry.position - lhs_offset});
				}
			}
		} else {
			for (const auto & rhs_entry: rhs) {
				if (lhs.contains(entry{rhs_entry.pkey, rhs_entry.position - rhs_offset + lhs_offset})) {
					result.emplace(entry{rhs_entry.pkey, rhs_entry.position - rhs_offset});
				}
			}
		}

		return result;
	}

	static auto intersect(std::set<entry> && lhs, const std::set<entry> & rhs, unsigned offset) -> std::set<entry> {
		if (lhs.size() <= rhs.size()) {
			// erase each one which is not present in RHS
			for (auto it = lhs.begin(); it != lhs.end();) {
				// filter out everything which is not in RHS
				if (!rhs.contains(entry{it->pkey, it->position + offset})) {
					it = lhs.erase(it);
				} else {
					++it;
				}
			}

			return lhs;
		} else {
			std::set<entry> result{};

			// iterate over RHS and find each element from LHS, if it's there, extract it from LHS and move to result
			for (const auto & rhs_entry: rhs) {
				if (auto it = lhs.find(entry{rhs_entry.pkey, rhs_entry.position - offset}); it != lhs.end()) {
					result.insert(lhs.extract(it));
				}
			}

			return result;
		}
	}

	struct ngram_matches {
		ngram_with_position<N> value;
		const std::set<entry> * matches;

		constexpr ngram_matches(ngram_with_position<N> v, const std::set<entry> * m) noexcept: value{v}, matches{m} { }

		constexpr size_t size() const noexcept {
			if (matches) {
				return matches->size();
			} else {
				return 0z;
			}
		}

		constexpr bool empty() const noexcept {
			return size() == 0z;
		}

		constexpr friend auto operator<=>(ngram_matches lhs, ngram_matches rhs) noexcept {
			const auto lhs_size = lhs.size();
			const auto rhs_size = rhs.size();

			return std::tie(lhs_size, lhs.value) <=> std::tie(rhs_size, rhs.value);
		}

		constexpr friend auto operator==(ngram_matches lhs, ngram_matches rhs) noexcept {
			return lhs.value == rhs.value;
		}

		constexpr const std::set<entry> & get_set() const noexcept {
			return *matches;
		}

		constexpr unsigned get_relative_position() const noexcept {
			return value.position;
		}
	};

	static auto intersect(const ngram_matches & lhs, const ngram_matches & rhs) -> std::set<entry> {
		return intersect(lhs.get_set(), lhs.get_relative_position(), rhs.get_set(), rhs.get_relative_position());
	}

	static auto intersect(std::set<entry> && lhs, const ngram_matches & rhs) -> std::set<entry> {
		return intersect(std::move(lhs), rhs.get_set(), rhs.get_relative_position());
	}

	auto get_sorted_ngram_matches(view_as_ngrams<N> input) const -> std::vector<ngram_matches> {
		auto matches = std::vector<ngram_matches>{};
		matches.reserve(input.size());

		// TODO filter out unnecessory ngrams

		for (auto ng: input) {
			matches.emplace_back(ng, find_ngram_occurences(ng.value));
		}

		// we need to sort nmatches by size of each sets
		std::sort(matches.begin(), matches.end());

		return matches;
	}

	auto find_all(view_as_ngrams<N> input) -> std::set<entry> {
		const auto matches = get_sorted_ngram_matches(input);

		if (matches.empty()) {
			// there was no ngram to match => can't search for size(input) < N
			return {};
		}

		const auto & first_match = matches[0];

		if (first_match.empty()) {
			// first ngram doesn't match anything => result is empty
			// this will be happen always for first element, as `matches` are sorted
			return {};
		}

		if (matches.size() == 1z) {
			// if there was only one ngram at all (we can't intersect it with anything)
			return first_match.get_set();
		}

		const auto & second_match = matches[1];

		// first two we intersect together (as they are immutable) and allocate all possible results
		std::set<entry> result = intersect(first_match, second_match);

		assert(result.size() <= first_match.size());
		assert(result.size() <= second_match.size());

		// and now we remove non matching items (for each subsequent ngram)
		for (size_t i = 2z; i != matches.size(); ++i) {
			[[maybe_unused]] const auto size_before = result.size();

			result = intersect(std::move(result), matches[i]);

			assert(result.size() <= size_before);
		}

		return result;
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
