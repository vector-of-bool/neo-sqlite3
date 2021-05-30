#include <neo/sqlite3/single.hpp>

#include "./tests.inl"

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Pull a single result") {
    db.prepare("CREATE TABLE foo AS VALUES (1, 2, 3)")->run_to_completion().throw_if_error();
    auto st           = *db.prepare("SELECT * FROM foo");
    auto [i1, i2, i3] = neo::sqlite3::unpack_single<int, int, int>(st);
    CHECK(i1 == 1);
    CHECK(i2 == 2);
    CHECK(i3 == 3);
}
