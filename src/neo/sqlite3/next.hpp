#pragma once

#include <neo/sqlite3/statement.hpp>

#include <stdexcept>

namespace neo::sqlite3 {

template <typename... Ts>
std::optional<std::tuple<Ts...>> unpack_next_opt(statement& st) {
    auto status = st.step();
    if (status != statement::more) {
        return std::nullopt;
    }
    return st.row.unpack<Ts...>();
}

template <typename... Ts>
std::tuple<Ts...> unpack_next(statement& st) {
    auto tup = unpack_next_opt<Ts...>(st);
    if (!tup) {
        throw std::runtime_error("Cannot pull next result from an exhausted query");
    }
    return std::move(*tup);
}

}  // namespace neo::sqlite3