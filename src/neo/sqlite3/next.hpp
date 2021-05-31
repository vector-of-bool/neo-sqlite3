#pragma once

#include "./errable.hpp"
#include "./row.hpp"

namespace neo::sqlite3 {

namespace detail {

errable<void> run_step(statement& st) noexcept;

}  // namespace detail

/**
 * @brief Obtain the next result from the statement as a tuple.
 *
 * @tparam Ts Element types of the result row
 * @param st The statement to execute
 * @return typed_row<Ts...> The statement result.
 */
template <typename... Ts>
[[nodiscard]] errable<typed_row<Ts...>> unpack_next(statement& st) {
    auto e = detail::run_step(st);
    if (e != errc::row) {
        return e.errc();
    }
    return typed_row<Ts...>{st};
}

}  // namespace neo::sqlite3
