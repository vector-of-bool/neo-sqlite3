#include "./iter_rows.hpp"

#include <neo/sqlite3/database.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Basic iteration") {
    auto db = neo::sqlite3::create_memory_db();
    db.prepare(R"(
        CREATE TABLE stuff
        AS VALUES
            (1, 2, 3),
            (4, 5, 6)
        )")
        .run_to_completion();
    auto st   = db.prepare("SELECT * FROM stuff");
    auto rng  = neo::sqlite3::iter_rows(st);
    auto iter = rng.begin();
    auto stop = rng.end();
    CHECK(stop != iter);
    CHECK(iter != stop);
    // Repeated access won't advance the iterator
    CHECK(iter->size() == 3);
    CHECK(iter->size() == 3);
    CHECK(iter->size() == 3);
    CHECK(iter->size() == 3);
    CHECK(iter->size() == 3);
    CHECK(iter->size() == 3);
    CHECK(iter->size() == 3);
    CHECK(iter != stop);
    {
        auto& row = *iter;
        CHECK(row[0].as_integer() == 1);
        CHECK(row[1].as_integer() == 2);
        CHECK(row[2].as_integer() == 3);
        ++iter;
    }
    {
        auto& row = *iter;
        CHECK(row[0].as_integer() == 4);
        CHECK(row[1].as_integer() == 5);
        CHECK(row[2].as_integer() == 6);
        ++iter;
    }
    CHECK(iter == stop);
}
