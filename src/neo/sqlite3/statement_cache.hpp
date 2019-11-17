#pragma once

#include <neo/sqlite3/database.hpp>
#include <neo/sqlite3/literal.hpp>
#include <neo/sqlite3/statement.hpp>

#include <memory>
#include <vector>

namespace neo::sqlite3 {

namespace detail {

struct cached_statement_item {
    sql_string_literal         key;
    std::unique_ptr<statement> stmt;
};

}  // namespace detail

class statement_cache {
    database*                                  _db;
    std::vector<detail::cached_statement_item> _statements;

public:
    statement_cache(database& db)
        : _db(&db) {}

    statement_cache(const statement_cache& cache) = delete;

    statement& operator()(sql_string_literal);
};

}  // namespace neo::sqlite3
