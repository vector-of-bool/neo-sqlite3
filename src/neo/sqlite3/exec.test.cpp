#include "./exec.hpp"

#include <neo/sqlite3/iter_tuples.hpp>
#include <neo/sqlite3/statement_cache.hpp>

#include "./tests.inl"

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Execute some queries") {
    db.exec("CREATE TABLE foo (value)");
    neo::sqlite3::exec(db.prepare("INSERT INTO foo VALUES (?)"), 2);
    neo::sqlite3::exec(db.prepare("INSERT INTO foo VALUES (?)"), 55);
    neo::sqlite3::exec(db.prepare("UPDATE foo SET value = value + 2"));
    auto get_values = db.prepare("SELECT value FROM foo");

    auto it = neo::sqlite3::iter_tuples<int>(get_values).begin();
    CHECK(std::get<0>(*it) == 4);
    ++it;
    CHECK(std::get<0>(*it) == 57);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Execute with a cache") {
    using namespace neo::sqlite3::literals;
    db.exec("CREATE TABLE foo (value)");
    neo::sqlite3::statement_cache stmt_cache{db};
    neo::sqlite3::exec(stmt_cache("INSERT INTO foo VALUES (12)"_sql));
    neo::sqlite3::exec(stmt_cache("INSERT INTO foo VALUES (12)"_sql));
    neo::sqlite3::exec(stmt_cache("INSERT INTO foo VALUES (12)"_sql));
    neo::sqlite3::exec(stmt_cache("INSERT INTO foo VALUES (?)"_sql), 24);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Iterate rows") {
    using namespace neo::sqlite3::literals;
    db.exec(R"(
        CREATE TABLE foo AS
        VALUES
            (1, 2, 3),
            (4, 5, 6)
    )");
    neo::sqlite3::statement_cache stmt_cache{db};
    auto rows = neo::sqlite3::exec_rows(stmt_cache("SELECT * FROM foo"_sql));
    auto it   = rows.begin();
    auto stop = rows.end();
    CHECK(it != stop);
    auto row1 = *it;
    CHECK((row1.unpack<int, int, int>()) == std::tuple(1, 2, 3));
    ++it;
    auto row2 = *it;
    CHECK((row2.unpack<int, int, int>()) == std::tuple(4, 5, 6));
    ++it;
    CHECK(it.at_end());
    CHECK(it == stop);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Iterate tuples") {
    using namespace neo::sqlite3::literals;
    auto db = neo::sqlite3::create_memory_db();
    db.exec(R"(
        CREATE TABLE foo AS
        VALUES
            (1, 2, 3),
            (4, 5, 6)
    )");
    neo::sqlite3::statement_cache stmt_cache{db};
    auto tups = neo::sqlite3::exec_tuples<int, int, int>(stmt_cache("SELECT * FROM foo"_sql));
    auto it   = tups.begin();
    auto stop = tups.end();

    auto tup1 = *it;
    CHECK(tup1 == std::tuple(1, 2, 3));
    auto tup2 = *++it;
    CHECK(tup2 == std::tuple(4, 5, 6));
    ++it;
    CHECK(it == stop);
}
