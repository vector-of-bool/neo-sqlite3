#include <neo/sqlite3/single.hpp>

#include "./tests.inl"

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Pull a single result") {
    db.prepare("CREATE TABLE foo AS VALUES (1, 2, 3)").run_to_completion();
    auto st           = db.prepare("SELECT * FROM foo");
    auto [i1, i2, i3] = neo::sqlite3::unpack_single<int, int, int>(st);
    CHECK(i1 == 1);
    CHECK(i2 == 2);
    CHECK(i3 == 3);
    CHECK_FALSE(st.is_busy());
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Multiple results cause failure") {
    db.prepare("CREATE TABLE foo AS VALUES (1, 2, 3), (4, 5, 6)").run_to_completion();
    auto st = db.prepare("SELECT * FROM foo");
    CHECK_THROWS_AS((neo::sqlite3::unpack_single<int, int, int>(st)), std::runtime_error);
}