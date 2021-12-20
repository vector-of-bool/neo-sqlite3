#pragma once

#include "./errable.hpp"
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
 * @brief Reset the given statement and bind each given argument to elements of the given tuple
 */
errable<void> reset_and_bind(statement& st, bindable_tuple auto const& tup) noexcept {
    st.reset();
    return st.bindings().bind_tuple(tup);
}

/**
 * @brief Match an argument list that is valid to be passed as the binding arguments to
 * reset_and_bind(). Either a pack of bindable objects, a single tuple of bindable objects, or
 * an empty pack.
 */
template <typename... Ts>
concept exec_bind_args = requires(statement& st, Ts&&... args) {
    reset_and_bind(st, NEO_FWD(args)...);
};

/**
 * @brief Reset, rebind, and execute a prepared statement to completion.
 *
 * Generated rows are discarded.
 *
 * @param st The statement to execute
 * @param bindings The bindings to apply to the prepared statement
 */
template <typename... Args>
errable<void> exec(statement_mutref st, const Args&... bindings) requires exec_bind_args<Args...> {
    // Because we are running to completion, we know that any strings will outlive the statement
    // execution, so we can convert every std::string to a string_view
    NEO_SQLITE3_CHECK(reset_and_bind(st, bindings...));
    return st->run_to_completion();
}

/**
 * @brief Reset and rebind a prepared statement. Returns a range of rows.
 *
 * @param st The statement to execute
 * @param bindings The parameter bindings for the prepared statement
 * @return A range of row objects.
 */
template <typename... Args>
[[nodiscard]] errable<iter_rows>
exec_rows(statement& st, const Args&... bindings) requires exec_bind_args<Args...> {
    NEO_SQLITE3_CHECK(reset_and_bind(st, detail::view_if_string(bindings)...));
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
template <typename... OutTypes, typename... Args>
[[nodiscard]] errable<iter_tuples<OutTypes...>>
exec_tuples(statement& st, const Args&... bindings) requires exec_bind_args<Args...> {
    NEO_SQLITE3_CHECK(reset_and_bind(st, detail::view_if_string(bindings)...));
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
        NEO_SQLITE3_CHECK(exec(st, tup));
    }
    return errc::done;
}

/**
 * @brief Access the next row of the given prepared statement as a dynamically typed row
 */
[[nodiscard]] inline errable<row_access> next(statement& st) noexcept {
    NEO_SQLITE3_CHECK_RC(st.step(), errc::row);
    return row_access(st.c_ptr());
}

/**
 * @brief Access the next row of the given prepared statement as a typed row
 */
template <typename... Ts>
[[nodiscard]] inline errable<typed_row<Ts...>> next(statement& st) noexcept {
    NEO_SQLITE3_CHECK_RC(st.step(), errc::row);
    return typed_row<Ts...>(st.c_ptr());
}

/**
 * @brief Obtain the tuple of the values of the first row of the prepared statement. Resets the
 * statement before and after obtaining the result, and binds the given tuple of bindable objects to
 * the prepared statement. Intended for use with statements that should return only a single row.
 */
template <typename... Ts, typename... Args>
[[nodiscard]] inline errable<std::tuple<Ts...>>
one_row(statement_mutref st, const Args&... args) noexcept requires exec_bind_args<Args...> {
    static_assert(((!std::same_as<Ts, blob_view> && !std::same_as<Ts, std::string_view>)&&...),
                  "View types will be immediately expired before returning from "
                  "neo::sqlite3::one_row(), and are therefore always undefined behavior.");
    NEO_SQLITE3_CHECK(reset_and_bind(st, args...));
    auto rst = st->auto_reset();
    NEO_SQLITE3_AUTO(r, next<Ts...>(st));
    return r.as_tuple();
}

/**
 * @brief Obtain the first column's value of the first row of the prepared statement. Resets the
 * statement before and after obtaining the result, and binds the given tuple of bindable objects to
 * the prepared statement. Intended for use with statements that should return online a single value
 * in a single row.
 */
template <typename T, typename... Args>
[[nodiscard]] inline errable<T>
one_cell(statement_mutref st, const Args&... args) noexcept requires exec_bind_args<Args...> {
    static_assert(!std::same_as<T, blob_view> && !std::same_as<T, std::string_view>,
                  "View types will be immediately expired before returning from "
                  "neo::sqlite3::one_cell(), and are therefore always undefined behavior.");
    NEO_SQLITE3_AUTO([v], one_row<T>(st, args...));
    return NEO_FWD(v);
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
