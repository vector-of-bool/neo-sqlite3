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
    )")
        .throw_if_error();
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
    )")
        .throw_if_error();
    auto p  = *db.prepare("SELECT * FROM stuff");
    auto rc = p.step();
    CHECK(rc == neo::sqlite3::errc::row);
    rc = p.step();
    CHECK(rc == neo::sqlite3::errc::row);
    db.interrupt();
    rc = p.step();
    CHECK(rc == neo::sqlite3::errc::interrupt);
    rc = p.step();
    CHECK(rc == neo::sqlite3::errc::row);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Count changes") {
    db.exec(R"(
        CREATE TABLE stuff (foo, bar);
        INSERT INTO stuff VALUES
            (1, 2),
            (1, 3),
            (1, 4),
            (1, 5)
        ;
    )")
        .throw_if_error();
    CHECK(db.changes() == 4);
    db.exec("DELETE FROM stuff WHERE foo = 1 AND bar == 3").throw_if_error();
    CHECK(db.changes() == 1);
    CHECK(db.total_changes() == 5);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Attach another database") {
    db.attach("new_temp", ":memory:").throw_if_error();
    db.exec("CREATE TABLE new_temp.foo (name TEXT)").throw_if_error();
    db.detach("new_temp").throw_if_error();
}
