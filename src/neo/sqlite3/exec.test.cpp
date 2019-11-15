#include "./exec.hpp"

#include <neo/sqlite3/iter_tuples.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Execute some queries") {
    auto db = neo::sqlite3::database::create_memory_db();
    db.exec("CREATE TABLE foo (value)");
    neo::sqlite3::exec(db, "INSERT INTO foo VALUES (?)", std::tuple(2));
    neo::sqlite3::exec(db, "INSERT INTO foo VALUES (?)", std::tuple(55));
    neo::sqlite3::exec(db, "UPDATE foo SET value = value + 2");
    auto get_values = db.prepare("SELECT value FROM foo");

    auto it         = neo::sqlite3::iter_tuples<int>(get_values).begin();
    CHECK(std::get<0>(*it) == 4);
    ++it;
    CHECK(std::get<0>(*it) == 57);
}