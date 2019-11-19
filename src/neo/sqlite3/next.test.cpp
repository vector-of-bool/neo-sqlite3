#include <neo/sqlite3/next.hpp>

#include "./tests.inl"

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Pull the next result") {
    db.prepare("CREATE TABLE foo AS VALUES (1, 2, 3)").run_to_completion();
    auto st           = db.prepare("SELECT * FROM foo");
    auto [i1, i2, i3] = neo::sqlite3::unpack_next<int, int, int>(st);
    CHECK(i1 == 1);
    CHECK(i2 == 2);
    CHECK(i3 == 3);
    CHECK(st.is_busy());  // Still waiting
    CHECK(st.step() == neo::sqlite3::statement::done);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Multiple results causes failure") {
    db.prepare("CREATE TABLE foo AS VALUES (1, 2, 3), (4, 5, 6)").run_to_completion();
    auto st = db.prepare("SELECT * FROM foo");
}