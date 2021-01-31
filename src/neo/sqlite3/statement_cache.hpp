#pragma once

#include <neo/sqlite3/literal.hpp>

#include <memory>
#include <vector>

struct sqlite3;

namespace neo::sqlite3 {

class database_ref;
class statement;

namespace detail {

struct cached_statement_item {
    sql_string_literal         key;
    std::unique_ptr<statement> stmt;
};

}  // namespace detail

/**
 * @brief Implements basic caching of prepared statements for string literals.
 *
 * The statement cache is associated with an open database, and that database
 * MUST outlive any database caches created from it. (Moving the database object
 * is safe.)
 */
class statement_cache {
    ::sqlite3*                                 _db;
    std::vector<detail::cached_statement_item> _statements;

public:
    explicit statement_cache(database_ref db) noexcept;
    ~statement_cache();
    statement_cache(statement_cache&&) noexcept;
    statement_cache& operator=(statement_cache&&) noexcept;

    /**
     * @brief Obtain a reference to a prepared statement for the given SQL
     * string literal.
     *
     * The first time a SQL string is passed for the lifetime of this object,
     * the statement will be prepared and cached, and subsequent accesses will
     * look up the value in the cache.
     */
    [[nodiscard]] statement& operator()(sql_string_literal);
};

}  // namespace neo::sqlite3
