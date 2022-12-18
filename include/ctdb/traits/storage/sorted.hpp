#ifndef CTDB_TRAITS_STORAGE_SORTED_HPP
#define CTDB_TRAITS_STORAGE_SORTED_HPP

#include "../traits.hpp"
#include <set>
#include <tuple>
#include <concepts>

namespace ctdb {

// mark with sorted to use non-unique sorted index (based on std::set)
template <typename> struct sorted { };

// forward declaration
template <typename IndexType> struct index_storage_traits;

// define comparators

template <typename IndexView, typename Entry> struct non_unique_comparator {
	using is_transparent = void;

	using index_view = IndexView;
	using value_type = std::remove_cvref_t<decltype(*std::declval<const Entry &>())>;
	using const_reference = const Entry &;

	constexpr bool operator()(const_reference lhs, const_reference rhs) const noexcept {
		// first we sort based on semantics of the view, and then based on the primary_key
		const auto & lhs_view = static_cast<index_view>(*lhs);
		const auto & rhs_view = static_cast<index_view>(*rhs);

		// we can't compare iterators directly, we need to take their addresses
		const auto lhs_addr = std::addressof(*lhs);
		const auto rhs_addr = std::addressof(*rhs);

		// TODO: do it differently as comparing two pointers is UB
		return std::tie(lhs_view, lhs_addr) < std::tie(rhs_view, rhs_addr);
	}

	// comparison against other types is always against the view only
	constexpr bool operator()(const_reference lhs, const std::totally_ordered_with<IndexView> auto & rhs) const noexcept {
		return static_cast<index_view>(*lhs) < rhs;
	}

	constexpr bool operator()(const std::totally_ordered_with<IndexView> auto & lhs, const_reference rhs) const noexcept {
		return lhs < static_cast<index_view>(*rhs);
	}
};

// simplest traits
template <typename Index> struct index_storage_traits<sorted<Index>> {
	template <typename PKey> using entry = PKey;
	template <typename PKey> using storage_type = std::set<entry<PKey>, non_unique_comparator<Index, entry<PKey>>>;

	template <typename Other> static constexpr bool compatible_type = std::totally_ordered_with<Other, Index>;
};

template <typename T> struct index_storage_traits: index_storage_traits<sorted<T>> {
	// this is default index!
};

} // namespace ctdb

#endif
