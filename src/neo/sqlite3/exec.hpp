#pragma once

#include <neo/sqlite3/iter_rows.hpp>
#include <neo/sqlite3/iter_tuples.hpp>
#include <neo/sqlite3/statement.hpp>

#include <neo/fwd.hpp>

#include <tuple>

namespace neo::sqlite3 {

/**
 * @brief Reset, rebind, and execute a prepared statement to completion.
 *
 * Yielded rows are discarded.
 *
 * @param st The statement to execute
 * @param bindings The bindings to apply to the prepared statement
 */
template <typename... Ts>
void exec(statement& st, Ts&&... bindings) {
    st.reset();
    st.bindings.clear();
    st.bindings = std::forward_as_tuple(NEO_FWD(bindings)...);
    st.run_to_completion();
}

// R-value statement overload
template <typename... Ts>
void exec(statement&& st, Ts&&... bindings) {
    exec(st, NEO_FWD(bindings)...);
}

/**
 * @brief Reset and rebind a prepared statement. Returns a range of rows.
 *
 * @param st The statement to execute
 * @param bindings The parameter bindings for the prepared statement
 * @return A range of row objects.
 */
template <typename... Ts>
[[nodiscard]] auto exec_rows(statement& st, Ts&&... bindings) {
    st.reset();
    st.bindings.clear();
    st.bindings = std::forward_as_tuple(NEO_FWD(bindings)...);
    return iter_rows(st);
}

// R-value statement overload
template <typename... Ts>
[[nodiscard]] auto exec_rows(statement&& st, Ts&&... bindings) {
    return exec_rows(st, NEO_FWD(bindings)...);
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
[[nodiscard]] auto exec_tuples(statement& st, Ts&&... bindings) {
    st.reset();
    st.bindings.clear();
    st.bindings = std::forward_as_tuple(NEO_FWD(bindings)...);
    return iter_tuples<OutTypes...>(st);
}

}  // namespace neo::sqlite3
