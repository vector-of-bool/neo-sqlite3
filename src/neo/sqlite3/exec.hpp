#pragma once

#include <neo/sqlite3/database.hpp>
#include <neo/sqlite3/statement.hpp>

#include <tuple>

namespace neo::sqlite3 {

void exec(database& db, const std::string& query) { db.exec(query); }

template <typename... Ts>
void exec(database& db, std::string_view query, const std::tuple<Ts...>& bindings) {
    auto st     = db.prepare(query);
    st.bindings = bindings;
    st.run_to_completion();
}

void exec(statement& st) {
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

}  // namespace neo::sqlite3