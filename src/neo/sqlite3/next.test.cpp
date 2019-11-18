#include <neo/sqlite3/database.hpp>
#include <neo/sqlite3/next.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Pull the next result") {
    auto db = neo::sqlite3::create_memory_db();
    db.prepare("CREATE TABLE foo AS VALUES (1, 2, 3)").run_to_completion();
    auto st           = db.prepare("SELECT * FROM foo");
    auto [i1, i2, i3] = neo::sqlite3::unpack_next<int, int, int>(st);
    CHECK(i1 == 1);
    CHECK(i2 == 2);
    CHECK(i3 == 3);
    CHECK(st.is_busy());  // Still waiting
    CHECK(st.step() == neo::sqlite3::statement::done);
}

TEST_CASE("Multiple results causes failure") {
    auto db = neo::sqlite3::create_memory_db();
    db.prepare("CREATE TABLE foo AS VALUES (1, 2, 3), (4, 5, 6)").run_to_completion();
    auto st = db.prepare("SELECT * FROM foo");
}