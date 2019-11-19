#include <neo/sqlite3/database.hpp>

#include "./tests.inl"

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Open a simple database") {}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "We are not in a transaction by default") {
    CHECK_FALSE(db.is_transaction_active());
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "exec() some code") {
    db.exec(R"(
        CREATE TABLE stuff (foo);
        CREATE TABLE others (bar);
    )");
}
