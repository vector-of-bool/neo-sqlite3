#include <neo/sqlite3/database.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Open simple database") { auto db = neo::sqlite3::database::open(":memory:"); }

TEST_CASE("We are not in a transaction by default") {
    auto db = neo::sqlite3::create_memory_db();
    CHECK_FALSE(db.is_transaction_active());
}

TEST_CASE("exec() some code") {
    auto db = neo::sqlite3::create_memory_db();
    db.exec(R"(
        CREATE TABLE stuff (foo);
        CREATE TABLE others (bar);
    )");
}