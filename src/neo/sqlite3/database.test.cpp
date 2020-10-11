#include <neo/sqlite3/database.hpp>

#include "./statement.hpp"

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

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Interrupt an operation") {
    db.exec(R"(
        CREATE TABLE stuff (foo, bar);
        INSERT INTO stuff VALUES
            (1, 2),
            (1, 3),
            (1, 4),
            (1, 5)
        ;
    )");
    auto p  = db.prepare("SELECT * FROM stuff");
    auto rc = p.step();
    CHECK(rc == neo::sqlite3::errc::row);
    rc = p.step();
    CHECK(rc == neo::sqlite3::errc::row);
    db.interrupt();
    rc = p.step(std::nothrow);
    CHECK(rc == neo::sqlite3::errc::interrupt);
    rc = p.step();
    CHECK(rc == neo::sqlite3::errc::row);
}
