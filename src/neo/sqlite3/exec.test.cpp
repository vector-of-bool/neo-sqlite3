#include "./exec.hpp"

#include <neo/sqlite3/iter_tuples.hpp>
#include <neo/sqlite3/next.hpp>
#include <neo/sqlite3/statement_cache.hpp>

#include "./tests.inl"

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Execute some queries") {
    db.exec("CREATE TABLE foo (value)");
    neo::sqlite3::exec(db.prepare("INSERT INTO foo VALUES (?)"), std::tuple(2));
    neo::sqlite3::exec(db.prepare("INSERT INTO foo VALUES (?)"), std::tuple(55));
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
    neo::sqlite3::exec(stmt_cache("INSERT INTO foo VALUES (?)"_sql), std::tuple(24));
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
    auto rows = neo::sqlite3::iter_rows(stmt_cache("SELECT * FROM foo"_sql));
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
    db.exec(R"(
        CREATE TABLE foo AS
        VALUES
            (1, 2, 3),
            (4, 5, 6)
    )");
    neo::sqlite3::statement_cache stmt_cache{db};
    auto tups = neo::sqlite3::iter_tuples<int, int, int>(stmt_cache("SELECT * FROM foo"_sql));
    auto it   = tups.begin();
    auto stop = tups.end();

    auto tup1 = *it;
    CHECK(tup1 == std::tuple(1, 2, 3));
    auto tup2 = *++it;
    CHECK(tup2 == std::tuple(4, 5, 6));
    ++it;
    CHECK(it == stop);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "exec with a range of tuples as bindings") {
    db.exec(R"(
        CREATE TABLE foo(age, name, score)
    )");
    std::vector<std::tuple<int, std::string, double>> values;
    values.emplace_back(24, "Joe", 6.3);
    values.emplace_back(18, "Amy", 42.1);
    values.emplace_back(99, "George", 0.2);
    auto before = db.total_changes();
    neo::sqlite3::exec_each(db.prepare("INSERT INTO foo VALUES(?, ?, ?)"), values);
    CHECK((db.total_changes() - before) == 3);

    auto [sum] = neo::sqlite3::unpack_next<int>(db.prepare("SELECT sum(age) FROM foo"));
    CHECK(sum == (24 + 18 + 99));
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture,
                 "exec() with a std::string that converts to a string_view") {
    db.exec("CREATE TABLE foo(name)");
    neo::sqlite3::exec(db.prepare("INSERT INTO foo VALUES (?)"),
                       std::forward_as_tuple(std::string("Joey")));
    auto [name] = neo::sqlite3::unpack_next<std::string>(db.prepare("SELECT * FROM foo"));
    CHECK(name == "Joey");
}
