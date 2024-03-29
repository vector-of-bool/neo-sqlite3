#pragma once

#include "./binding.hpp"
#include "./errable_fwd.hpp"
#include "./errc.hpp"
#include "./row.hpp"

#include <neo/mutref.hpp>
#include <neo/utility.hpp>

struct sqlite3_stmt;
struct sqlite3;

namespace neo::sqlite3 {

class statement;
class connection_ref;
class statement;
template <typename... Ts>
class typed_row;
class auto_reset;

namespace event {

struct step_first {
    statement& st;
};

struct step {
    statement& st;
    errc       ec;
};

struct reset {
    statement& st;
};

}  // namespace event

/**
 * @brief Access the metadata of a statement's result columns
 *
 */
class column {
    const statement* _owner;
    int              _index = 0;

public:
    /**
     * @brief Access the column metadata for 'st' at index 'idx' (zero-based)
     *
     * @param st The statement to inspect
     * @param idx The index of the column to access (zero-based)
     */
    column(const statement& st, int idx)
        : _owner(&st)
        , _index(idx) {}

    // The name of the column
    [[nodiscard]] std::string_view name() const noexcept;

    /// The original name of the column, regardless of the AS
    [[nodiscard]] std::string_view origin_name() const noexcept;

    /// The name of the table that owns the column
    [[nodiscard]] std::string_view table_name() const noexcept;

    /// The name of the database that owns the column
    [[nodiscard]] std::string_view database_name() const noexcept;
};

/**
 * @brief Access to the result column metadata of a SQLite statement.
 */
class column_access {
    const statement* _owner;

public:
    /**
     * @brief Access the metadata of the columns of the given statement
     *
     * @param st The statement to access
     */
    explicit column_access(statement& st)
        : _owner(&st) {}

    /**
     * @brief Access the column at the given index
     *
     * @param idx The index of the column. Left-most column is index zero
     */
    [[nodiscard]] column operator[](int idx) const noexcept;

    [[nodiscard]] int count() const noexcept;
};

/**
 * @brief Access the result row of an in-progress SQLite statement.
 */
class row_access;

/**
 * @brief A prepared statement returns by connection{_ref}::prepare
 *
 * When the object is destroyed, the statement will be freed.
 */
class statement {
    sqlite3_stmt* _stmt_ptr = nullptr;
    void          _destroy() noexcept;

public:
    /// Alias of errc::done
    static constexpr auto done = errc::done;
    /// Alias of errc::row
    static constexpr auto more = errc::row;

    ~statement() {
        // This is defined inline so that compilers have greater visibility to DCE this branch
        if (_stmt_ptr) {
            _destroy();
        }
    }

    explicit statement(sqlite3_stmt*&& ptr) noexcept
        : _stmt_ptr(std::exchange(ptr, nullptr)) {}

    statement(statement&& o) noexcept { _stmt_ptr = o.release(); }

    statement& operator=(statement&& o) noexcept {
        if (_stmt_ptr) {
            _destroy();
        }
        _stmt_ptr = o.release();
        return *this;
    }

    /// Reset the state of the SQLite VM. Prepares the statement to be executed again
    void reset() noexcept;

    /// Obtain the SQLite C API pointer
    [[nodiscard]] sqlite3_stmt* c_ptr() const noexcept { return _stmt_ptr; }
    /// Relinquish ownership of the underlying C API pointer
    [[nodiscard]] sqlite3_stmt* release() noexcept { return std::exchange(_stmt_ptr, nullptr); }

    /**
     * @brief Execute one step of the prepared statement, and return the result
     * code.
     *
     * @return errable<void> The result code of a single step. Will never be errc::misuse
     */
    errable<void> step() noexcept;

    /**
     * @brief Continually execute the statement until it is complete
     */
    errable<void> run_to_completion() noexcept;

    /**
     * @brief Determine whether the statement is currently being executed.
     *
     * @return true If step() has been called and the statement is not done
     * @return false If the statement is not already being executed
     */
    [[nodiscard]] bool is_busy() const noexcept;

    /**
     * @brief Access to the current row of this statement.
     */
    [[nodiscard]] row_access row() const noexcept { return row_access{c_ptr()}; }

    /**
     * @brief Access/modify the parameter bindings of this statement
     */
    [[nodiscard]] binding_access bindings() noexcept { return binding_access{c_ptr()}; }

    /**
     * @brief Access the columne metadata of this statement.
     */
    [[nodiscard]] column_access columns() noexcept { return column_access{*this}; }

    /// Get a reference to the connection associated with this statement
    [[nodiscard]] connection_ref connection() noexcept;

    [[nodiscard, deprecated("Use the connection() method instead.")]]  //
    connection_ref
    database() noexcept;

    [[nodiscard]] inline class auto_reset auto_reset() noexcept;

    [[nodiscard]] std::string_view sql_string() const noexcept;
    [[nodiscard]] std::string      expanded_sql_string() const noexcept;

    constexpr friend void do_repr(auto out, const statement* st) noexcept {
        out.type("neo::sqlite3::statement");
        if (st) {
            out.bracket_value("{}, code={}, expanded={}, current_row={}",
                              out.repr_value(st->c_ptr()),
                              out.repr_value(st->sql_string()),
                              out.repr_value(st->expanded_sql_string()),
                              out.repr_value(st->row()));
        }
    }
};

/**
 * @brief A reference-type that refers to a mutable statement.
 *
 * This is for use as a parameter type that accepts a non-const lvalue or rvalue
 * reference.
 */
using statement_mutref = neo::mutref<statement>;

class [[nodiscard]] auto_reset {
    statement* _st = nullptr;

public:
    constexpr auto_reset() noexcept = default;
    explicit auto_reset(statement& st) noexcept
        : _st(&st) {}

    auto_reset(auto_reset&& o) noexcept
        : _st(neo::take(o._st)) {}

    auto_reset& operator=(auto_reset&& o) noexcept {
        _st = neo::take(o._st);
        return *this;
    }

    ~auto_reset() {
        if (_st) {
            _st->reset();
        }
    }
};

neo::sqlite3::auto_reset statement::auto_reset() noexcept {
    return neo::sqlite3::auto_reset{*this};
}

}  // namespace neo::sqlite3
