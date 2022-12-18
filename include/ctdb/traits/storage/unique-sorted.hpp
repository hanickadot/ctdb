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
template <typename IndexView, typename Entry> struct unique_comparator {
	using is_transparent = void;

	using index_view = IndexView;
	using value_type = std::remove_cvref_t<decltype(*std::declval<const Entry &>())>;
	using const_reference = const Entry &;

	// unique comparison ignores comparisong based on Entry type
	constexpr bool operator()(const_reference lhs, const_reference rhs) const noexcept {
		return static_cast<index_view>(*lhs) < static_cast<index_view>(*rhs);
	}

	// comparison against other types is always against the view only
	constexpr bool operator()(const_reference lhs, const std::totally_ordered_with<value_type> auto & rhs) const noexcept {
		return static_cast<index_view>(*lhs) < rhs;
	}

	constexpr bool operator()(const std::totally_ordered_with<value_type> auto & lhs, const_reference rhs) const noexcept {
		return lhs < static_cast<index_view>(*rhs);
	}
};

template <typename Index> struct index_storage_traits<unique_sorted<Index>> {
	template <typename PKey> using entry = PKey;
	template <typename PKey> using storage_type = std::set<entry<PKey>, unique_comparator<Index, entry<PKey>>>;

	template <typename Other> static constexpr bool compatible_type = std::totally_ordered_with<Other, Index>;
};

} // namespace ctdb

#endif
