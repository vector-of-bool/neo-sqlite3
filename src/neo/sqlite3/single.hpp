#pragma once

#include <neo/sqlite3/next.hpp>

#include <optional>
#include <tuple>

namespace neo::sqlite3 {

/**
 * @brief Obtain a single tuple from a statement that returns a single row.
 *
 * If the statement fails to obtain a result, returns nullopt. The statement
 * will then be advanced again to check that there are no more results. If the
 * statement does not declare that it is done on the second step(), then an
 * exception will be thrown.
 *
 * @note If the first step() fails, 'ec' will contain the error/result code and
 * this function returns nullopt. If the second step() fails or returns another
 * row, an exception will be thrown with the error/result code of the second
 * step().
 *
 * @tparam Ts The types of elements of the result row
 * @param st The statement to execute
 * @param ec Output the result of executing the statement.
 * @return std::optional<typed_row<Ts...>> A tuple of results, or nullopt if
 * there are none, or nullopt if the first step() encountered an error.
 */
template <typename... Ts>
[[nodiscard]] std::optional<typed_row<Ts...>> unpack_single_opt(statement&       st,
                                                                std::error_code& ec) {
    return unpack_next_opt<Ts...>(st, ec);
}

/**
 * @brief Throwing variant of unpack_single_opt
 *
 * @note Both variants of unpack_single_opt throw an exception, but this one
 * will throw an exception if the first step() fails with an error instead of
 * errc::done.
 */
template <typename... Ts>
[[nodiscard]] std::optional<typed_row<Ts...>> unpack_single_opt(statement& st) {
    std::error_code ec;
    auto            res = unpack_single_opt<Ts...>(st, ec);
    if (ec != errc::row && ec != errc::done) {
        // No data was pulled, nor are we done. An error:
        throw_error(ec,
                    "Failed to pull a single element for a statement result for unpack_single_opt",
                    st.database());
    }
    return res;
}

/**
 * @brief Pull a single result from the given statement.
 *
 * Like unpack_single_opt, but if there is no result to pull, will instead
 * throw an `errc_error<errc::done>` exception.
 *
 * @tparam Ts The types of the row results
 * @param st The statement to execute
 * @return typed_row<Ts...> The resulting row as a tuple
 */
template <typename... Ts>
[[nodiscard]] typed_row<Ts...> unpack_single(statement& st) {
    auto tup = unpack_single_opt<Ts...>(st);
    if (!tup) {
        throw_error(make_error_code(errc::done),
                    "Attempt to pull a single item from a query that is empty/exhausted.",
                    st.database());
    }
    return std::move(*tup);
}

}  // namespace neo::sqlite3
