#pragma once

#include <neo/sqlite3/literal.hpp>

#include <memory>
#include <vector>

struct sqlite3;

namespace neo::sqlite3 {

class connection_ref;
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
 * The statement cache is associated with an open connection, and that connection
 * MUST outlive any caches created from it. (Moving the connection object
 * is safe.)
 */
class statement_cache {
    ::sqlite3*                                 _db;
    std::vector<detail::cached_statement_item> _statements;

public:
    explicit statement_cache(connection_ref db) noexcept;
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

    /**
     * @brief Obtain the SQLite connection associated with this cache
     */
    [[nodiscard]] connection_ref connection() const noexcept;
};

namespace event {

/**
 * @brief Event fired when a cache-lookup fails to find a previously-prepared statement
 */
struct statement_cache_miss {
    statement_cache&   cache;
    sql_string_literal sql;
};

/**
 * @brief Event fired when a cache-lookup finds a previously-prepared statement to reuse
 */
struct statement_cache_hit {
    statement_cache&   cache;
    sql_string_literal sql;
    statement&         stmt;
};

}  // namespace event

}  // namespace neo::sqlite3
