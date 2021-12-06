#include "./iter_rows.hpp"

#include <neo/sqlite3/connection.hpp>
#include <neo/sqlite3/statement.hpp>

#include "./tests.inl"

#include <ranges>

static_assert(std::ranges::view<neo::sqlite3::iter_rows>);
static_assert(std::ranges::input_range<neo::sqlite3::iter_rows>);

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Basic iteration") {
    db.prepare(R"(
        CREATE TABLE stuff
        AS VALUES
            (1, 2, 3),
            (4, 5, 6)
        )")
        ->run_to_completion()
        .throw_if_error();
    auto st   = *db.prepare("SELECT * FROM stuff");
    auto rng  = neo::sqlite3::iter_rows(st);
    auto iter = rng.begin();
    auto stop = rng.end();
    CHECK(iter != stop);
    // Repeated access won't advance the iterator
    CHECK_NOTHROW(*iter);
    CHECK_NOTHROW(*iter);
    CHECK_NOTHROW(*iter);
    CHECK_NOTHROW(*iter);
    CHECK_NOTHROW(*iter);
    CHECK_NOTHROW(*iter);
    CHECK_NOTHROW(*iter);
    CHECK(iter != stop);
    {
        auto row = *iter;
        CHECK(row[0].as_integer() == 1);
        CHECK(row[1].as_integer() == 2);
        CHECK(row[2].as_integer() == 3);
        ++iter;
    }
    {
        auto row = *iter;
        CHECK(row[0].as_integer() == 4);
        CHECK(row[1].as_integer() == 5);
        CHECK(row[2].as_integer() == 6);
        ++iter;
    }
    CHECK(iter == stop);
}
