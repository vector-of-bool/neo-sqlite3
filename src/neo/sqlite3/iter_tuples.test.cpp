#include <neo/sqlite3/iter_tuples.hpp>

#include "./statement.hpp"
#include "./tests.inl"

#include <neo/any_range.hpp>
#include <neo/memory.hpp>

#include <ranges>

static_assert(std::ranges::view<neo::sqlite3::iter_tuples<int, int>>);
static_assert(std::ranges::input_range<neo::sqlite3::iter_tuples<int, int>>);

struct thing {
    int         a;
    int         b;
    std::string str;

    bool operator==(const thing&) const = default;
};

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

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Create a transformed-view over some tuples") {
    db.exec(R"(
        CREATE TABLE stuff
        AS VALUES
            (1, 2, 'string'),
            (2, 5, 'other'),
            (5, 2, 'meow')
    )")
        .throw_if_error();
    auto st = *db.prepare("SELECT * FROM stuff");

    auto row_as_thing = [](auto row) {
        auto [a, b, str] = row;
        return thing{a, b, str};
    };

    {
        auto view = neo::sqlite3::iter_tuples<int, int, std::string>(st)
            | std::views::transform([&, rst = neo::copy_shared(st.auto_reset())](auto tup) {
                        return row_as_thing(tup);
                    });
        auto it = view.begin();
        CHECK(*it == thing{1, 2, "string"});
        ++it;
        CHECK(*it == thing{2, 5, "other"});
    }
    // We didn't advance to the end, but the statement will be reset by the auto-reset
    CHECK_FALSE(st.is_busy());
    {
        // Type-erase in view
        auto rst = neo::copy_shared(st.auto_reset());
        auto v   = neo::sqlite3::iter_tuples<int, int, std::string>(st)
            | std::views::transform([&, rst](auto tup) { return row_as_thing(tup); });
        neo::any_input_range<thing> view(v);
        auto                        it = view.begin();
        CHECK(*it == thing{1, 2, "string"});
        ++it;
        CHECK(*it == thing{2, 5, "other"});
    }
    CHECK_FALSE(st.is_busy());
}
