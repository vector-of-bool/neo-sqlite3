#pragma once

#include <neo/sqlite3/error.hpp>
#include <neo/sqlite3/statement.hpp>

namespace neo::sqlite3 {

/**
 * @brief Obtain the next result from the statement, expanded in a tuple, or nullopt if execution
 * fails/stops.
 *
 * The 'ec' parameter receives an error code corresponding to the result of executing the statement.
 * If the statement finishes normally, it will hold errc::done.
 *
 * @tparam Ts The types of the row elements.
 * @param st The statement to execute
 * @param ec Output parameter to receive the error/result of execution
 * @return std::optional<typed_row<Ts...>> A tuple, or nullopt if the row finishes executing or
 * encounters an error
 */
template <typename... Ts>
[[nodiscard]] std::optional<typed_row<Ts...>> unpack_next_opt(statement&       st,
                                                              std::error_code& ec) noexcept {
    auto status = st.step(ec);
    if (status != statement::more) {
        return std::nullopt;
    }
    return st.row().unpack<Ts...>();
}

/**
 * @brief Throwing variant of unpack_next_opt
 *
 * This function will throw an exception if the statement finishes with any condition
 * other than errcond::done.
 */
template <typename... Ts>
[[nodiscard]] std::optional<typed_row<Ts...>> unpack_next_opt(statement& st) noexcept {
    std::error_code ec;
    auto            r = unpack_next_opt(st, ec);
    if (!r) {
        if (ec != errcond::done) {
            throw_error(ec,
                        "Unexpected error while pulling next result from a prepared statement",
                        st.database());
        }
        return std::nullopt;
    }
    return st.row().unpack<Ts...>();
}

/**
 * @brief Obtain the next result from the statement as a tuple.
 *
 * @tparam Ts Element types of the result row
 * @param st The statement to execute
 * @return typed_row<Ts...> The statement result.
 */
template <typename... Ts>
[[nodiscard]] typed_row<Ts...> unpack_next(statement& st) {
    auto r = unpack_next_opt<Ts...>(st);
    if (!r) {
        throw_error(make_error_code(errc::done),
                    "Cannot unpack next value from the database",
                    st.database());
    }
    return st.row().unpack<Ts...>();
}

}  // namespace neo::sqlite3
