#include <neo/sqlite3/database.hpp>
#include <neo/sqlite3/iter_tuples.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Iterate over some tuples") {
    auto db = neo::sqlite3::database::create_memory_db();
    db.prepare(R"(
        CREATE TABLE stuff
        AS VALUES
            (1, 2, 'STRING'),
            (2, 5, 'other')
        )")
        .run_to_completion();

    auto st = db.prepare("SELECT * FROM stuff");

    auto tup_iter = neo::sqlite3::iter_tuples<int, int, std::string>(st);
    auto iter     = tup_iter.begin();
    auto stop     = tup_iter.end();
    CHECK(iter != stop);
    {
        auto [a, b, c] = *iter;
        CHECK(a == 1);
        CHECK(b == 2);
        CHECK(c == "STRING");
        ++iter;
    }
    CHECK(iter != stop);
    {
        auto [a, b, c] = *iter;
        CHECK(a == 2);
        CHECK(b == 5);
        CHECK(c == "other");
        ++iter;
    }
    CHECK(iter == stop);
}
