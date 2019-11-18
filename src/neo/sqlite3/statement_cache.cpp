#include "./statement_cache.hpp"

#include <algorithm>

using namespace neo::sqlite3;

statement& statement_cache::operator()(sql_string_literal key) {
    auto insert_point = std::partition_point(_statements.begin(),
                                             _statements.end(),
                                             [&](const detail::cached_statement_item& item) {
                                                 return key < item.key;
                                             });
    if (insert_point == _statements.end() || insert_point->key != key) {
        // Need to generate a new statement
        auto new_st = _db->prepare(key.string());
        auto iter   = _statements.emplace(insert_point,
                                        detail::cached_statement_item{key,
                                                                      std::make_unique<statement>(
                                                                          std::move(new_st))});
        return *iter->stmt;
    }
    return *insert_point->stmt;
}