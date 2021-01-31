#pragma once

#include <neo/sqlite3/iter_rows.hpp>
#include <neo/sqlite3/iter_tuples.hpp>
#include <neo/sqlite3/statement.hpp>

#include <neo/concepts.hpp>
#include <neo/fwd.hpp>
#include <neo/range_concepts.hpp>

#include <tuple>

namespace neo::sqlite3 {

/**
 * @brief Reset and execute the prepared statement to completion.
 *
 * No parameters are bound. Generated rows are discarded
 *
 * @param st
 */
inline void exec(statement_mutref st) {
    st->reset();
    st->run_to_completion();
}

/**
 * @brief Reset, rebind, and execute a prepared statement to completion.
 *
 * Generated rows are discarded.
 *
 * @param st The statement to execute
 * @param bindings The bindings to apply to the prepared statement
 */
template <typename... Ts>
void exec(statement_mutref st, const std::tuple<Ts...>& bindings) {
    st->reset();
    // Because we are running to completion, we know that any strings will outlive the statement
    // execution, so we can convert every std::string to a string_view
    st->bindings() = detail::view_strings(bindings);
    st->run_to_completion();
}

/**
 * @brief Reset and rebind a prepared statement. Returns a range of rows.
 *
 * @param st The statement to execute
 * @param bindings The parameter bindings for the prepared statement
 * @return A range of row objects.
 */
template <typename... Ts>
[[nodiscard]] auto exec_rows(statement& st, const std::tuple<Ts...>& bindings) {
    st.reset();
    st.bindings() = bindings;
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
template <typename... OutTypes, typename... Ts>
[[nodiscard]] auto exec_tuples(statement& st, const std::tuple<Ts...>& bindings) {
    st.reset();
    st.bindings() = bindings;
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
    void exec_each(statement_mutref st, TuplesRange&& tuples) {
    for (auto&& tup : tuples) {
        st->reset();
        st->bindings() = tup;
        st->run_to_completion();
    }
}

}  // namespace neo::sqlite3
