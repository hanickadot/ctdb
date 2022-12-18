#ifndef CTDB_TRAITS_STORAGE_UNIQUE_SORTED_HPP
#define CTDB_TRAITS_STORAGE_UNIQUE_SORTED_HPP

#include "../traits.hpp"
#include <set>
#include <concepts>

namespace ctdb {

// just to allow user mark some index unique
template <typename Index> struct unique_sorted { };

// forward declaration
template <typename IndexType> struct index_storage_traits;

// define comparators

// and rest will behave exactly as original index but only comparison will be a different
template <typename Entry> struct unique_comparator {
	using is_transparent = void;

	using value_type = std::remove_cvref_t<decltype(*std::declval<const Entry &>())>;
	using const_reference = const Entry &;

	// unique comparison ignores comparisong based on Entry type
	constexpr bool operator()(const_reference lhs, const_reference rhs) const noexcept {
		return *lhs < *rhs;
	}

	// comparison against other types is always against the view only
	constexpr bool operator()(const_reference lhs, const std::totally_ordered_with<value_type> auto & rhs) const noexcept {
		return *lhs < rhs;
	}

	constexpr bool operator()(const std::totally_ordered_with<value_type> auto & lhs, const_reference rhs) const noexcept {
		return lhs < *rhs;
	}
};

template <typename T> struct index_storage_traits<unique_sorted<T>> {
	template <typename PKey> using entry = PKey;
	template <typename PKey> using storage_type = std::set<entry<PKey>, unique_comparator<entry<PKey>>>;

	template <typename Other> static constexpr bool compatible_type = std::totally_ordered_with<Other, T>;
};

} // namespace ctdb

#endif
