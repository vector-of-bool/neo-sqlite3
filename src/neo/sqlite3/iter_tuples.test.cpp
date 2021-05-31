#include <neo/sqlite3/iter_tuples.hpp>

#include "./tests.inl"
#include "./statement.hpp"

#if __has_include(<ranges>)
#include <ranges>

static_assert(std::ranges::view<neo::sqlite3::iter_tuples<int, int>>);
static_assert(std::ranges::input_range<neo::sqlite3::iter_tuples<int, int>>);
#endif

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Iterate over some tuples") {
    db.prepare(R"(
        CREATE TABLE stuff
        AS VALUES
            (1, 2, 'STRING'),
            (2, 5, 'other')
        )")
        ->run_to_completion()
        .throw_if_error();

    auto st = *db.prepare("SELECT * FROM stuff");

    auto tup_iter = neo::sqlite3::iter_tuples<int, int, std::string>(st);
    auto iter     = tup_iter.begin();
    auto stop     = tup_iter.end();
    CHECK(iter != stop);
    {
        auto [a, b, c] = *iter;
        CHECK(a == 1);
        CHECK(b == 2);
        CHECK(c == "STRING");
        ++iter;
    }
    CHECK(iter != stop);
    {
        auto [a, b, c] = *iter;
        CHECK(a == 2);
        CHECK(b == 5);
        CHECK(c == "other");
        ++iter;
    }
    CHECK(iter == stop);
}
