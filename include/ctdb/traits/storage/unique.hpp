#ifndef CTDB_TRAITS_STORAGE_UNIQUE_HPP
#define CTDB_TRAITS_STORAGE_UNIQUE_HPP

#include "../traits.hpp"
#include <unordered_set>
#include <concepts>

namespace ctdb {

// just to allow user mark some index unique
template <typename Index> struct unique { };

// forward declaration
template <typename IndexType> struct index_storage_traits;

// define comparators

template <typename T, typename Hash> concept hashable_by = // make sure this type is hashable by hash type
	requires(const Hash & h, const T & val) {
		{ h(val) } -> std::same_as<size_t>;
	};

template <typename IndexView, typename Entry, typename Hash> struct unique_equality_hash {
	using is_transparent = void;
	using hash_type = Hash;

	using index_view = IndexView;
	using value_type = std::remove_cvref_t<decltype(*std::declval<const Entry &>())>;
	using const_reference = const Entry &;

	constexpr auto operator()(const_reference value) const noexcept {
		return hash_type{}(static_cast<index_view>(*value));
	}

	template <typename T>
	requires(std::equality_comparable_with<T, value_type> && hashable_by<T, hash_type>)
	constexpr auto operator()(const T & value) const noexcept {
		return hash_type{}(value);
	}
};

template <typename IndexView, typename Entry> struct unique_equality {
	using is_transparent = void;

	using index_view = IndexView;
	using value_type = std::remove_cvref_t<decltype(*std::declval<const Entry &>())>;
	using const_reference = const Entry &;

	// unique comparison ignores comparisong based on Entry type
	constexpr bool operator()(const_reference lhs, const_reference rhs) const noexcept {
		return static_cast<index_view>(*lhs) == static_cast<index_view>(*rhs);
	}

	// comparison against other types is always against the view only
	constexpr bool operator()(const_reference lhs, const std::equality_comparable_with<value_type> auto & rhs) const noexcept {
		return static_cast<index_view>(*lhs) == rhs;
	}

	constexpr bool operator()(const std::equality_comparable_with<value_type> auto & lhs, const_reference rhs) const noexcept {
		return lhs == static_cast<index_view>(*rhs);
	}
};

template <typename Index> concept has_hash_type = //
	requires() {
		typename Index::hash_type;
	};

template <typename Index> struct get_hash_functor {
	using type = std::hash<Index>;
};

template <has_hash_type Index> struct get_hash_functor<Index> {
	using type = typename Index::hash_type;
};

template <typename Index> struct index_storage_traits<unique<Index>> {
	using hash_type = typename get_hash_functor<Index>::type;
	template <typename PKey> using entry = PKey;
	template <typename PKey> using storage_type = std::unordered_set<entry<PKey>, unique_equality_hash<Index, entry<PKey>, hash_type>, unique_equality<Index, entry<PKey>>>;

	template <typename Other> static constexpr bool compatible_type = std::equality_comparable_with<Other, Index> && hashable_by<Other, hash_type>;
};

} // namespace ctdb

#endif
