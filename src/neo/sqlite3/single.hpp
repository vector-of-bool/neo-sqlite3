#pragma once

#include <neo/sqlite3/next.hpp>
#include <neo/sqlite3/statement.hpp>

#include <optional>
#include <stdexcept>
#include <tuple>

namespace neo::sqlite3 {

template <typename... Ts>
std::optional<std::tuple<Ts...>> unpack_single_opt(statement& st) {
    auto opt_elem = unpack_next_opt<Ts...>(st);
    if (!opt_elem) {
        return std::nullopt;
    }
    if (st.step() == statement::more) {
        throw std::runtime_error(
            "More one element pulled from statement for unpack_single* functions");
    }
    return opt_elem;
}

template <typename... Ts>
std::tuple<Ts...> unpack_single(statement& st) {
    auto tup = unpack_single_opt<Ts...>(st);
    if (!tup) {
        throw std::runtime_error(
            "Attempt to pull a single item from a query that is empty/exhausted.");
    }
    return std::move(*tup);
}

}  // namespace neo::sqlite3