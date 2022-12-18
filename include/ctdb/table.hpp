#ifndef CTDB_TABLE_HPP
#define CTDB_TABLE_HPP

#include "indices/indices.hpp"
#include <list>
#include <utility>
#include <cassert>
#include <compare>

namespace ctdb {

template <typename T> struct table_range {
	T first;
	T last;

	constexpr table_range(T f, T l) noexcept: first{f}, last{l} { }

	constexpr auto begin() const noexcept {
		return first;
	}

	constexpr auto end() const noexcept {
		return last;
	}

	constexpr auto ascending() const noexcept {
		return *this;
	}

	constexpr auto descending() const noexcept {
		return table_range(std::reverse_iterator(last), std::reverse_iterator(first));
	}

	constexpr size_t size() const noexcept {
		return static_cast<size_t>(std::distance(first, last));
	}
};

struct always_same {
	constexpr bool operator()(const auto &, const auto &) const noexcept {
		return false;
	}
};

template <typename Record, typename... Indices> struct table {
	using record_type = Record;

	// TODO: use hive, to avoid many allocations
	std::list<record_type> content;
	using primary_key = typename decltype(content)::iterator;

	static_assert(sizeof(primary_key) == sizeof(void *));

	indices_tuple<primary_key, Indices...> indices;

	template <typename... Args> constexpr auto emplace(Args &&... args) -> std::optional<primary_key> {
		// this will always insert new record at the end O(1)
		const auto & item = content.emplace_front(std::forward<Args>(args)...);
		const auto it = content.begin();

		assert(item == *it);

		// and now insert into indices
		if (!indices.insert(it)) {
			// and if any of them fails, rollback
			content.erase(it);
			return std::nullopt;
		}

		return it;
	}

	constexpr bool erase(primary_key it) noexcept {
		if (indices.remove(it)) {
			content.erase(it);
			return true;
		} else {
			return false;
		}
	}

	constexpr size_t size() const noexcept {
		return content.size();
	}

	template <typename Type> constexpr auto size() const noexcept {
		// be aware of old GCC ABI!
		return indices.template size<Type>();
	}

	constexpr auto all() const noexcept {
		return table_range{content.begin(), content.end()};
	}

	template <typename Type> constexpr auto all() const noexcept {
		return indices.template all<Type>();
	}

	template <typename Type> constexpr auto equal(const Type & value) const noexcept {
		return indices.template equal<Type>(value);
	}

	template <typename Type> constexpr auto operator==(const Type & value) const noexcept {
		return indices.template equal<Type>(value);
	}
};

} // namespace ctdb

#endif
