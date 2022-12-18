#ifndef CTDB_TRAITS_TRAITS_HPP
#define CTDB_TRAITS_TRAITS_HPP

#include "storage/sorted.hpp"
#include "storage/unique-sorted.hpp"
#include "storage/unique.hpp"
#include <optional>
#include <set>
#include <unordered_set>

namespace ctdb {

template <typename IndexType> struct index_storage_traits;
template <typename IndexType> struct index_traits;

template <typename IndexType> using index_storage_traits_of = index_storage_traits<IndexType>;
template <typename IndexType, typename PKey> using index_storage_of = typename index_storage_traits_of<IndexType>::template storage_type<PKey>;

template <typename T> inline constexpr bool is_container = false;
template <typename T> inline constexpr bool is_sorted_container = false;

template <typename... Ts> inline constexpr bool is_container<std::set<Ts...>> = true;
template <typename... Ts> inline constexpr bool is_sorted_container<std::set<Ts...>> = true;

template <typename... Ts> inline constexpr bool is_container<std::unordered_set<Ts...>> = true;

// provide default implementations of addition/find/removal
template <typename IndexTraits, typename PKey> struct index_helper {
	using primary_key = PKey;
	using entry = typename IndexTraits::template entry<primary_key>;
	using storage_type = typename IndexTraits::template storage_type<primary_key>;
	using iterator_type = typename storage_type::iterator;

	static_assert(is_container<storage_type>);

	template <typename T> static constexpr bool compatible_type = IndexTraits::template compatible_type<T>;

	[[nodiscard]] static constexpr auto insert(storage_type & storage, const primary_key & pkey) -> std::optional<iterator_type> {
		// TODO check if 'insert' is not defined in the traits itself
		if (auto [it, success] = storage.emplace(pkey); success) {
			return it;
		} else {
			return std::nullopt;
		}
	}

	template <typename T> [[nodiscard]] static constexpr auto find(const storage_type & storage, const T & value) noexcept -> iterator_type
	requires(compatible_type<T>)
	{
		return storage.find(value);
	}

	template <typename T> [[nodiscard]] static constexpr auto lower_bound(const storage_type & storage, const T & value) noexcept -> iterator_type
	requires(compatible_type<T> && is_sorted_container<storage_type>)
	{
		return storage.lower_bound(value);
	}

	template <typename T> [[nodiscard]] static constexpr auto upper_bound(const storage_type & storage, const T & value) noexcept -> iterator_type
	requires(compatible_type<T> && is_sorted_container<storage_type>)
	{
		return storage.upper_bound(value);
	}

	[[nodiscard]] static constexpr auto begin(const storage_type & storage) noexcept -> iterator_type {
		return storage.begin();
	}

	[[nodiscard]] static constexpr auto end(const storage_type & storage) noexcept -> iterator_type {
		return storage.end();
	}

	[[nodiscard]] static constexpr auto find(const storage_type & storage, const primary_key & pkey) noexcept -> iterator_type {
		return storage.find(pkey);
	}

	[[nodiscard]] static constexpr size_t size(const storage_type & storage) noexcept {
		return storage.size();
	}

	static constexpr void remove(storage_type & storage, iterator_type it) noexcept {
		// TODO check if 'remove' is not defined in the traits itself
		storage.erase(it);
	}
};

} // namespace ctdb

#endif
