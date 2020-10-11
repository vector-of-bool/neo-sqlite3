#pragma once

#include <neo/sqlite3/statement.hpp>

#include <neo/assert.hpp>
#include <neo/iterator_facade.hpp>

#include <cassert>
#include <iterator>

namespace neo::sqlite3 {

/**
 * @brief A range over the results of a SQLite statement.
 *
 * Dereferencing the iterator of iter_rows will return the statement's `row`
 * member.
 */
class iter_rows {
    std::reference_wrapper<statement> _st;

public:
    /**
     * @brief Create a new range-of-rows that pulls results from the given
     * SQLite statement.
     *
     * @param st The statement from which to pull results
     */
    explicit iter_rows(statement& st)
        : _st(st) {}

    /**
     * @brief Iterator to access the resulting rows from the statement.
     */
    class iterator : public neo::iterator_facade<iterator> {
        friend class iter_rows;
        statement* _st = nullptr;

        explicit iterator(statement& st)
            : _st(&st) {
            auto rc = st.step();
            neo_assert(invariant,
                       rc == errc::row || rc == errc::done,
                       "Initial step returned an error");
        }

    public:
        constexpr iterator() = default;

        row_access dereference() const noexcept {
            neo_assert(expects,
                       _st != nullptr,
                       "Dereference of row-iterator with no associated statement");
            neo_assert(expects, !at_end(), "Dereference of finished row-iterator");
            return _st->row();
        }

        void increment() {
            neo_assert(expects,
                       _st != nullptr,
                       "Advanced of row-iterator with no associated statement");
            neo_assert(expects, !at_end(), "Advance of a finished row-iterator");
            auto rc = _st->step();
            neo_assert(invariant, rc == errc::row || rc == errc::done, "step() returned an error");
        }

        struct sentinel_type {};
        bool at_end() const noexcept { return !_st->is_busy(); }
    };

    /**
     * @brief Begin iterating over rows.
     *
     * Calling this function will execute the statement *once* to ready the first
     * result. Beware calling this multiple times.
     */
    iterator begin() const noexcept { return iterator(_st); }

    /**
     * @brief Obtain an end-sentinel for the row iterator
     */
    constexpr auto end() const noexcept { return iterator::sentinel_type(); }
};

}  // namespace neo::sqlite3
