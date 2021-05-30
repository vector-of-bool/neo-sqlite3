#pragma once

#include "./binding.hpp"
#include "./errable.hpp"
#include "./error.hpp"
#include "./row.hpp"

#include <neo/assert.hpp>
#include <neo/ref.hpp>

#include <new>

namespace std {

template <typename T>
struct tuple_size;

template <std::size_t I, typename T>
struct tuple_element;

}  // namespace std

struct sqlite3_stmt;
struct sqlite3;

namespace neo::sqlite3 {

class statement;
class database_ref;
class statement;
template <typename... Ts>
class typed_row;

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
     * @param st The database statement to inspect
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
    [[nodiscard]] column operator[](int idx) const noexcept {
        neo_assert(expects, idx < count(), "Column index is out-of-range", idx, count());
        return column{*_owner, idx};
    }

    [[nodiscard]] int count() const noexcept;
};

/**
 * @brief Access the result row of an in-progress SQLite statement.
 */
class row_access;

/**
 * @brief A prepared statement returns by database{_ref}::prepare
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
     * @brief Execute one step of the prepared statement.
     *
     * Returns the result code of executing one step. If the result returned
     * from step() is neither errc::row nor errc::done, then this will throw
     * a 'sqlite3::errc_error` exception corresponding to the result code.
     *
     * @return errc The result of execution. Fort his overload, either errc::done
     *         or errc::row.
     */
    errc step();

    /**
     * @brief Execute one step of the prepared statement, and return the result
     * code unconditionally.
     *
     * Unlike the zero-argument form, this method will return the result code
     * of executing the statement one step, but will not throw an exception in
     * case of normal error.
     *
     * @note There is one condition: We assert() against errc::misuse. If SQLite
     *       detects a misuse of the library, the program will terminate.
     *
     * @return errc The result code of a single step. Will never be errc::misuse
     */
    [[nodiscard]] errc step(std::nothrow_t) noexcept;

    /**
     * @brief Execute one step of the prepared statement, and place the result
     *        into a 'std::error_code'
     *
     * @param ec Output parameter for the error code. Will always be of the
     *           sqlite3::error_category()
     * @return errc The raw result code of executing the statement.
     */
    errc step(std::error_code& ec) noexcept;

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
    [[nodiscard]] auto row() const noexcept { return row_access{*this}; }

    /**
     * @brief Access/modify the parameter bindings of this statement
     */
    [[nodiscard]] auto bindings() noexcept { return binding_access{*this}; }

    /**
     * @brief Access the columne metadata of this statement.
     */
    [[nodiscard]] auto columns() noexcept { return column_access{*this}; }

    /// Get a reference to the database associated with this statement
    [[nodiscard]] database_ref database() noexcept;
};

/**
 * @brief A reference-type that refers to a mutable statement.
 *
 * This is for use as a parameter type that accepts a non-const lvalue or rvalue
 * reference.
 */
using statement_mutref = neo::mutref<statement>;

}  // namespace neo::sqlite3

template <typename... Ts>
struct std::tuple_size<neo::sqlite3::typed_row<Ts...>> {
    constexpr static std::size_t value = sizeof...(Ts);
};

template <std::size_t I, typename... Ts>
struct std::tuple_element<I, neo::sqlite3::typed_row<Ts...>> {
    using type = neo::sqlite3::typed_row<Ts...>::template nth_type<I>;
};
