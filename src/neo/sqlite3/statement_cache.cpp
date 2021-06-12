#include "./statement_cache.hpp"

#include "./connection.hpp"
#include "./statement.hpp"

#include <neo/event.hpp>
#include <neo/ufmt.hpp>
#include <sqlite3/sqlite3.h>

#include <algorithm>

using namespace neo::sqlite3;

statement_cache::statement_cache(connection_ref db) noexcept
    : _db(db.c_ptr()) {}

statement_cache::~statement_cache()                          = default;
statement_cache::statement_cache(statement_cache&&) noexcept = default;
statement_cache& statement_cache::operator=(statement_cache&&) noexcept = default;

connection_ref statement_cache::connection() const noexcept { return connection_ref{_db}; }

statement& statement_cache::operator()(sql_string_literal key) {
    auto insert_point = std::partition_point(_statements.begin(),
                                             _statements.end(),
                                             [&](const detail::cached_statement_item& item) {
                                                 return key < item.key;
                                             });
    if (insert_point == _statements.end() || insert_point->key != key) {
        neo::emit(event::statement_cache_miss{*this, key});
        // Need to generate a new statement
        auto new_st = *connection().prepare(key.string());
        auto iter   = _statements.emplace(insert_point,
                                        detail::cached_statement_item{key,
                                                                      std::make_unique<statement>(
                                                                          std::move(new_st))});
        return *iter->stmt;
    } else {
        neo::emit(event::statement_cache_hit{*this, key, *insert_point->stmt});
    }
    return *insert_point->stmt;
}
