#ifndef CTDB_INDICES_INDICES_HPP
#define CTDB_INDICES_INDICES_HPP

#include "../traits/traits.hpp"
#include <iterator>

namespace ctdb {

template <typename PKey, typename...> struct indices_tuple;

template <typename> inline constexpr bool type_is_not_compatible_with_any_index = false;
template <typename> inline constexpr bool unknown_order_tag = false;

struct ascending_order_tag { };
struct descending_order_tag { };

constexpr inline auto asc = ascending_order_tag{};
constexpr inline auto desc = descending_order_tag{};

template <typename OrigIterator> struct index_iterator: OrigIterator {
	using value_type = decltype(*std::declval<typename std::iterator_traits<OrigIterator>::value_type>());
	using reference_type = const value_type &;

	constexpr index_iterator(OrigIterator orig): OrigIterator{orig} { }

	constexpr reference_type operator*() const noexcept {
		return OrigIterator::operator*().operator*();
	}
};

template <typename T> struct index_range {
	T first;
	T last;

	constexpr index_range(T f, T l) noexcept: first{f}, last{l} { }
	constexpr index_range(const index_range &) noexcept = default;
	constexpr index_range(index_range &&) noexcept = default;

	constexpr auto begin() const noexcept {
		return index_iterator{first};
	}

	constexpr auto end() const noexcept {
		return index_iterator{last};
	}

	constexpr auto ascending() const noexcept {
		return *this;
	}

	constexpr auto descending() const noexcept {
		using rev = decltype(std::make_reverse_iterator(last));
		return index_range<rev>(std::make_reverse_iterator(last), std::make_reverse_iterator(first));
	}

	constexpr size_t size() const noexcept {
		return static_cast<size_t>(std::distance(first, last));
	}
};

template <typename PKey> struct indices_tuple<PKey> {
	constexpr bool insert(PKey) const noexcept {
		return true;
	}

	constexpr bool remove(PKey) const noexcept {
		return true;
	}

	template <typename Type> constexpr auto all() const noexcept -> index_range<const void *> {
		static_assert(type_is_not_compatible_with_any_index<Type>);
		return {nullptr, nullptr};
	}

	template <typename Type> constexpr auto size() const noexcept -> size_t {
		static_assert(type_is_not_compatible_with_any_index<Type>);
		return 0z;
	}
};

template <typename PKey, typename Head, typename... Tail> struct indices_tuple<PKey, Head, Tail...> {
	using record_type = std::remove_cvref_t<decltype(*std::declval<PKey>())>;
	using index_traits = index_storage_traits_of<Head>;
	using storage_type = index_storage_of<Head, PKey>;

	using helper = index_helper<index_traits, PKey>;

	storage_type index_data;
	indices_tuple<PKey, Tail...> tail;

	constexpr bool insert(PKey key) {
		const auto opt_it = helper::insert(index_data, key);

		if (!opt_it) {
			return false;
		}

		if (!tail.insert(key)) {
			helper::remove(index_data, *opt_it);
			return false;
		}

		return true;
	}

	constexpr bool remove(PKey key) noexcept {
		const auto it = helper::find(index_data, key);

		if (it == helper_end(index_data)) {
			return false;
		}

		if (!tail.remove(key)) {
			return false;
		}

		helper::remove(index_data, it);

		return true;
	}

	template <typename Type> constexpr auto size() const noexcept {
		if constexpr (helper::template compatible_type<Type>) {
			return helper::size(index_data);

		} else {
			return tail.template size<Type>();
		}
	}

	template <typename Type> constexpr auto all() const noexcept {
		if constexpr (helper::template compatible_type<Type>) {
			return index_range{helper::begin(index_data), helper::end(index_data)};

		} else {
			return tail.template all<Type>();
		}
	}

	template <typename Type> constexpr auto equal(const Type & value) const noexcept {
		if constexpr (helper::template compatible_type<Type>) {
			return index_range{helper::lower_bound(index_data, value), helper::upper_bound(index_data, value)};

		} else {
			return tail.equal(value);
		}
	}
};

} // namespace ctdb

#endif
