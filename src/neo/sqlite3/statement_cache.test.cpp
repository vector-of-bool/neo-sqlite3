#include <neo/sqlite3/statement_cache.hpp>

#include <catch2/catch.hpp>

using namespace neo::sqlite3::literals;

TEST_CASE("Basic statement caching") {
    auto                          db = neo::sqlite3::create_memory_db();
    neo::sqlite3::statement_cache cache{db};

    auto  q   = "SELECT 12"_sql;
    auto& st1 = cache(q);
    auto& st2 = cache("VALUES (1)"_sql);
    auto& st3 = cache(q);
    CHECK(&st1 != &st2);
    CHECK(&st1 == &st3);
}
