#pragma once

#include "./iter_rows.hpp"
#include "./iter_tuples.hpp"
#include "./statement.hpp"

#include <neo/concepts.hpp>
#include <neo/fwd.hpp>
#include <neo/range_concepts.hpp>

namespace neo::sqlite3 {

/**
 * @brief Reset the given statement and bind each given argument
 */
template <bindable... Args>
errable<void> reset_and_bind(statement& st, const Args&... bindings) noexcept {
    st.reset();
    return st.bindings().bind_all(bindings...);
}

/**
 * @brief Reset, rebind, and execute a prepared statement to completion.
 *
 * Generated rows are discarded.
 *
 * @param st The statement to execute
 * @param bindings The bindings to apply to the prepared statement
 */
template <bindable... Args>
errable<void> exec(statement_mutref st, const Args&... bindings) {
    // Because we are running to completion, we know that any strings will outlive the statement
    // execution, so we can convert every std::string to a string_view
    auto e = reset_and_bind(st, detail::view_if_string(bindings)...);
    if (e.is_error()) {
        return e;
    }
    return st->run_to_completion();
}

/**
 * @brief Reset and rebind a prepared statement. Returns a range of rows.
 *
 * @param st The statement to execute
 * @param bindings The parameter bindings for the prepared statement
 * @return A range of row objects.
 */
template <bindable... Ts>
[[nodiscard]] errable<iter_rows> exec_rows(statement& st, const Ts&... bindings) {
    auto e = reset_and_bind(st, bindings...);
    if (e.is_error()) {
        return e.errc();
    }
    return iter_rows(st);
}

/**
 * @brief Reset and rebind a prepared statement. Returns a range of tuples.
 *
 * @tparam OutTypes The types of the tuple elements to pull from the result rows
 * @param st The statement to execute
 * @param bindings The parameter bindings for the prepared statement
 * @return A range of tuples corresponding to the values in the rows
 */
template <typename... OutTypes, bindable... Ts>
[[nodiscard]] errable<iter_tuples<OutTypes...>> exec_tuples(statement& st, const Ts&... bindings) {
    auto e = reset_and_bind(st, bindings...);
    if (e.is_error()) {
        return e.errc();
    }
    return iter_tuples<OutTypes...>(st);
}

/**
 * @brief Execute a prepared statement once for each tuple of bindings in  the given range.
 *
 * Given a prepared statement that accepts bindable parameters and a range of tuples,
 * for each tuple `tup` in `tuples`, bind `tup` to the bindings of the prepared
 * statement, then execute that statement until completion.
 *
 * @param st A prepared statement to execute
 * @param tuples A range-of-tuples. The tuple type must meet `bindable_tuple`
 */
template <ranges::range TuplesRange>
requires bindable_tuple<ranges::range_value_t<TuplesRange>>  //
    errable<void> exec_each(statement_mutref st, TuplesRange&& tuples) {
    for (auto&& tup : tuples) {
        st->reset();
        st->bindings() = tup;
        auto res       = st->run_to_completion();
        if (res.is_error()) {
            return res;
        }
    }
    return errc::done;
}

/**
 * @brief Access the next row of the given prepared statement as a dynamically typed row
 */
[[nodiscard]] inline errable<row_access> next(statement& st) noexcept {
    auto e = st.step();
    if (e != errc::row) {
        return e.errc();
    }
    return row_access(st);
}

/**
 * @brief Access the next row of the given prepared statement as a typed row
 */
template <typename... Ts>
[[nodiscard]] inline errable<typed_row<Ts...>> next(statement& st) noexcept {
    auto e = st.step();
    if (e != errc::row) {
        return e.errc();
    }
    return typed_row<Ts...>(st);
}

/**
 * @brief Obtain the tuple of the values of the next row of the prepared statement. Resets the
 * statement after obtaining the result. Intended for use with statements that should return only a
 * single row.
 */
template <typename... Ts>
[[nodiscard]] inline errable<std::tuple<Ts...>> one_row(statement_mutref st) noexcept {
    auto rst = st->auto_reset();
    auto r   = next<Ts...>(st);
    if (!r.has_value()) {
        return r.errc();
    }
    return r->as_tuple();
}

/**
 * @brief Obtain the first column's value of the next row of the prepared statement. Resets the
 * statement after obtaining the result. Intended for use with statements that should return online
 * a single value in a single row.
 */
template <typename T>
[[nodiscard]] inline errable<T> one_cell(statement_mutref st) noexcept {
    auto rst = st->auto_reset();
    auto r   = next<T>(st);
    if (!r.has_value()) {
        return r.errc();
    }
    return r->template get<0>();
}

template <typename... Ts>
[[nodiscard, deprecated("use neo::sqlite3::next<...>()")]] errable<typed_row<Ts...>>
unpack_next(statement& st) {
    auto e = st.step();
    if (e != errc::row) {
        return e.errc();
    }
    return typed_row<Ts...>{st};
}

}  // namespace neo::sqlite3
