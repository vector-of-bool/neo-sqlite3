#pragma once

#include "./row.hpp"
#include "./statement.hpp"

#include <neo/assert.hpp>
#include <neo/iterator_facade.hpp>

namespace neo::sqlite3 {

class statement;

/**
 * @brief A range over the results of a SQLite statement.
 *
 * Dereferencing the iterator of iter_rows will return the statement's `row`
 * member.
 */
class iter_rows {
    statement* _st = nullptr;

public:
    iter_rows() = default;

    /**
     * @brief Create a new range-of-rows that pulls results from the given
     * SQLite statement.
     *
     * @param st The statement from which to pull results
     */
    explicit iter_rows(statement& st) noexcept
        : _st(&st) {}

    /**
     * @brief Iterator to access the resulting rows from the statement.
     */
    class iterator : public neo::iterator_facade<iterator> {
        friend class iter_rows;
        statement* _st = nullptr;

        explicit iterator(statement& st);

    public:
        constexpr iterator() = default;

        using difference_type = std::ptrdiff_t;
        using value_type      = row_access;
        enum { single_pass_iterator = true };

        value_type dereference() const noexcept {
            neo_assert(expects,
                       _st != nullptr,
                       "Dereference of row-iterator with no associated statement");
            neo_assert(expects, !at_end(), "Dereference of finished row-iterator");
            return row_access{_st->c_ptr()};
        }

        void increment();

        struct sentinel_type {};
        bool operator==(sentinel_type) const noexcept { return at_end(); }
        bool at_end() const noexcept;
    };

    /**
     * @brief Begin iterating over rows.
     *
     * Calling this function will execute the statement *once* to ready the first
     * result. Beware calling this multiple times.
     */
    [[nodiscard]] iterator begin() const {
        neo_assert(expects, _st != nullptr, "Called begin() on default-constructed iter_rows");
        return iterator(*_st);
    }

    /**
     * @brief Obtain an end-sentinel for the row iterator
     */
    [[nodiscard]] constexpr auto end() const noexcept { return iterator::sentinel_type(); }
};

}  // namespace neo::sqlite3

#ifdef __has_include
#if __has_include(<ranges/v3/range/concepts.hpp>)
#include <ranges/v3/range/concepts.hpp>
template <>
constexpr inline bool ranges::v3::enable_view<neo::sqlite3::iter_rows> = true;
#endif
#endif

#include <ranges>
template <>
constexpr inline bool std::ranges::enable_view<neo::sqlite3::iter_rows> = true;