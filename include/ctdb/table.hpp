#ifndef CTDB_TABLE_HPP
#define CTDB_TABLE_HPP

#include "indices/indices.hpp"
#include <memory>
#include <set>
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

template <typename Record, typename... Indices> struct table {
	struct primary_record;

	class primary_key {
		const Record * record;
		friend struct primary_record;

	public:
		constexpr const Record & view_record() const noexcept {
			assert(record != nullptr);
			return *record;
		}

		constexpr const Record & operator*() const noexcept {
			assert(record != nullptr);
			return *record;
		}

		constexpr primary_key(const Record * r) noexcept: record{r} { }
		constexpr primary_key(const primary_key &) noexcept = default;
		constexpr primary_key(primary_key &&) noexcept = default;
		constexpr ~primary_key() noexcept = default;
		constexpr auto operator=(const primary_key &) noexcept -> primary_key & = default;
		constexpr auto operator=(primary_key &&) noexcept -> primary_key & = default;

		explicit constexpr operator bool() const noexcept {
			return record != nullptr;
		}

		friend constexpr bool operator==(primary_key, primary_key) noexcept = default;
		friend constexpr bool operator==(primary_key lhs, std::nullptr_t) noexcept {
			return lhs.record == nullptr;
		}

		friend constexpr auto operator<=>(primary_key lhs, primary_key rhs) noexcept = default;
	};

	struct primary_record: std::unique_ptr<Record> {
		using super = std::unique_ptr<Record>;
		using super::super;

		explicit constexpr primary_record(std::unique_ptr<Record> orig) noexcept: super{std::move(orig)} { }

		template <typename... Args> constexpr static auto make(Args &&... args) {
			return primary_record{std::make_unique<Record>(std::forward<Args>(args)...)};
		}

		constexpr auto get_primary_key() const noexcept -> primary_key {
			return primary_key(this->get());
		}

		friend constexpr bool operator==(const primary_record & lhs, const primary_record & rhs) noexcept {
			return lhs.get() == rhs.et();
		}

		friend constexpr auto operator<=>(const primary_record & lhs, const primary_record & rhs) noexcept {
			return std::compare_three_way{}(lhs.get(), rhs.get());
		}

		friend constexpr bool operator==(const primary_record & lhs, primary_key rhs) noexcept {
			return lhs.get() == rhs.record;
		}

		friend constexpr auto operator<=>(const primary_record & lhs, primary_key rhs) noexcept {
			return std::compare_three_way{}(lhs.get(), rhs.record);
		}
	};

	// TODO: use hive without unique_ptr
	std::set<primary_record, std::less<void>> content;
	indices_tuple<primary_key, Indices...> indices;

	template <typename... Args> constexpr auto emplace(Args &&... args) -> primary_key {
		const auto [it, success] = content.emplace(primary_record::make(std::forward<Args>(args)...));

		if (!success) {
			return nullptr;
		}

		const auto pkey = it->get_primary_key();
		assert(success);

		if (!indices.insert(pkey)) {
			content.erase(it);
			return nullptr;
		}

		return pkey;
	}

	constexpr size_t size() const noexcept {
		return content.size();
	}

	template <typename Type> constexpr auto size() const noexcept {
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
