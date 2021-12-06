#include "./exec.hpp"

#include <neo/sqlite3/error.hpp>
#include <neo/sqlite3/exec.hpp>
#include <neo/sqlite3/iter_tuples.hpp>
#include <neo/sqlite3/statement_cache.hpp>

#include "./tests.inl"

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Execute some queries") {
    db.exec("CREATE TABLE foo (value)").throw_if_error();
    neo::sqlite3::exec(*db.prepare("INSERT INTO foo VALUES (?)"), 2).throw_if_error();
    neo::sqlite3::exec(*db.prepare("INSERT INTO foo VALUES (?)"), 55).throw_if_error();
    neo::sqlite3::exec(*db.prepare("UPDATE foo SET value = value + 2")).throw_if_error();
    auto get_values = *db.prepare("SELECT value FROM foo");

    auto it = neo::sqlite3::iter_tuples<int>(get_values).begin();
    CHECK(it->get<0>() == 4);
    ++it;
    CHECK(it->get<0>() == 57);
    ++it;
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Execute with a cache") {
    using namespace neo::sqlite3::literals;
    db.exec("CREATE TABLE foo (value)").throw_if_error();
    neo::sqlite3::statement_cache stmt_cache{db};
    neo::sqlite3::exec(stmt_cache("INSERT INTO foo VALUES (12)"_sql)).throw_if_error();
    neo::sqlite3::exec(stmt_cache("INSERT INTO foo VALUES (12)"_sql)).throw_if_error();
    neo::sqlite3::exec(stmt_cache("INSERT INTO foo VALUES (12)"_sql)).throw_if_error();
    neo::sqlite3::exec(stmt_cache("INSERT INTO foo VALUES (?)"_sql), 24).throw_if_error();
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Iterate rows") {
    using namespace neo::sqlite3::literals;
    db.exec(R"(
        CREATE TABLE foo AS
        VALUES
            (1, 2, 3),
            (4, 5, 6)
    )")
        .throw_if_error();
    neo::sqlite3::statement_cache stmt_cache{db};
    auto rows = neo::sqlite3::iter_rows(stmt_cache("SELECT * FROM foo"_sql));
    auto it   = rows.begin();
    auto stop = rows.end();
    CHECK(it != stop);
    auto row1 = *it;
    CHECK((row1.unpack<int, int, int>()).as_tuple() == std::tuple(1, 2, 3));
    ++it;
    auto row2 = *it;
    CHECK((row2.unpack<int, int, int>()).as_tuple() == std::tuple(4, 5, 6));
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
    )")
        .throw_if_error();
    neo::sqlite3::statement_cache stmt_cache{db};
    auto tups = neo::sqlite3::iter_tuples<int, int, int>(stmt_cache("SELECT * FROM foo"_sql));
    auto it   = tups.begin();
    auto stop = tups.end();

    auto tup1 = *it;
    CHECK(tup1.as_tuple() == std::tuple(1, 2, 3));
    auto tup2 = *++it;
    CHECK(tup2.as_tuple() == std::tuple(4, 5, 6));
    ++it;
    CHECK(it == stop);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "exec with a range of tuples as bindings") {
    db.exec(R"(
        CREATE TABLE foo(age, name, score)
    )")
        .throw_if_error();
    std::vector<std::tuple<int, std::string, double>> values;
    values.emplace_back(24, "Joe", 6.3);
    values.emplace_back(18, "Amy", 42.1);
    values.emplace_back(99, "George", 0.2);
    auto before = db.total_changes();
    neo::sqlite3::exec_each(*db.prepare("INSERT INTO foo VALUES(?, ?, ?)"), values)
        .throw_if_error();
    CHECK((db.total_changes() - before) == 3);

    auto st    = *db.prepare("SELECT sum(age) FROM foo");
    auto [sum] = neo::sqlite3::next<int>(st)->as_tuple();
    CHECK(sum == (24 + 18 + 99));
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture,
                 "exec() with a std::string that converts to a string_view") {
    db.exec("CREATE TABLE foo(name)").throw_if_error();
    neo::sqlite3::exec(*db.prepare("INSERT INTO foo VALUES (?)"), "Joey").throw_if_error();
    auto st     = *db.prepare("SELECT * FROM foo");
    auto [name] = *neo::sqlite3::next<std::string>(st);
    CHECK(name == "Joey");
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Pull the next result") {
    db.prepare("CREATE TABLE foo AS VALUES (1, 2, 3)")->run_to_completion().throw_if_error();
    auto st           = *db.prepare("SELECT * FROM foo");
    auto [i1, i2, i3] = *neo::sqlite3::next<int, int, int>(st);
    CHECK(i1 == 1);
    CHECK(i2 == 2);
    CHECK(i3 == 3);
    CHECK(st.is_busy());  // Still waiting
    CHECK_THROWS_AS(*neo::sqlite3::next<int>(st),
                    neo::sqlite3::errc_error<neo::sqlite3::errc::done>);

    db.exec("DELETE FROM foo").throw_if_error();
    CHECK_NOTHROW(neo::sqlite3::one_row<int>(st));
    CHECK_NOTHROW(neo::sqlite3::one_cell<int>(st));
    CHECK_THROWS_AS(*neo::sqlite3::one_row<int>(st),
                    neo::sqlite3::errc_error<neo::sqlite3::errc::done>);
    CHECK_THROWS_AS(*neo::sqlite3::one_cell<int>(st),
                    neo::sqlite3::errc_error<neo::sqlite3::errc::done>);
}
