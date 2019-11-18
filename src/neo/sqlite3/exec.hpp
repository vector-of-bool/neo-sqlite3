#pragma once

#include <neo/sqlite3/database.hpp>
#include <neo/sqlite3/iter_rows.hpp>
#include <neo/sqlite3/iter_tuples.hpp>
#include <neo/sqlite3/literal.hpp>
#include <neo/sqlite3/statement.hpp>
#include <neo/sqlite3/statement_cache.hpp>

#include <tuple>

namespace neo::sqlite3 {

inline void exec(database& db, const std::string& query) { db.exec(query); }

template <typename... Ts>
void exec(database& db, std::string_view query, const std::tuple<Ts...>& bindings) {
    auto st     = db.prepare(query);
    st.bindings = bindings;
    st.run_to_completion();
}

inline void exec(statement& st) {
    st.reset();
    st.bindings.clear();
    st.run_to_completion();
}

template <typename... Ts>
void exec(statement& st, const std::tuple<Ts...>& bindings) {
    st.reset();
    st.bindings.clear();
    st.bindings = bindings;
    st.run_to_completion();
}

template <typename... Ts>
void exec(statement_cache& stc, sql_string_literal sql, const std::tuple<Ts...>& bindings) {
    statement& st = stc(sql);
    st.reset();
    st.bindings.clear();
    st.bindings = bindings;
    st.run_to_completion();
}

inline void exec(statement_cache& stc, sql_string_literal sql) { exec(stc, sql, std::tuple<>()); }

template <typename... Ts>
auto exec_iter(statement_cache& stc, sql_string_literal sql, const std::tuple<Ts...>& bindings) {
    statement& st = stc(sql);
    st.reset();
    st.bindings.clear();
    st.bindings = bindings;
    return iter_rows(st);
}

template <typename... OutTypes, typename... Ts, typename = std::enable_if_t<sizeof...(OutTypes)>>
auto exec_iter(statement_cache& stc, sql_string_literal sql, const std::tuple<Ts...>& bindings) {
    statement& st = stc(sql);
    st.reset();
    st.bindings.clear();
    st.bindings = bindings;
    return iter_tuples<OutTypes...>(st);
}

inline auto exec_iter(statement_cache& stc, sql_string_literal sql) {
    return exec_iter(stc, sql, std::tuple<>());
}

template <typename... OutTypes, typename = std::enable_if_t<sizeof...(OutTypes)>>
auto exec_iter(statement_cache& stc, sql_string_literal sql) {
    return exec_iter<OutTypes...>(stc, sql, std::tuple<>());
}

}  // namespace neo::sqlite3