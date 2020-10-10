#include "./statement_cache.hpp"

#include "./c/sqlite3.h"
#include "./database.hpp"
#include "./statement.hpp"

#include <neo/ufmt.hpp>

#include <algorithm>

using namespace neo::sqlite3;

statement_cache::statement_cache(database& db) noexcept
    : _db(db.c_ptr()) {}

statement_cache::~statement_cache() = default;

statement& statement_cache::operator()(sql_string_literal key) {
    auto insert_point = std::partition_point(_statements.begin(),
                                             _statements.end(),
                                             [&](const detail::cached_statement_item& item) {
                                                 return key < item.key;
                                             });
    if (insert_point == _statements.end() || insert_point->key != key) {
        // Need to generate a new statement
        std::error_code ec;
        auto            new_st = statement::prepare_within(_db, key.string(), ec);
        if (ec) {
            throw_error(ec,
                        ufmt("Failed to prepare statement for caching [[{}]]", key.string()),
                        ::sqlite3_errmsg(_db));
        }
        auto iter = _statements.emplace(insert_point,
                                        detail::cached_statement_item{key,
                                                                      std::make_unique<statement>(
                                                                          std::move(*new_st))});
        return *iter->stmt;
    }
    return *insert_point->stmt;
}
