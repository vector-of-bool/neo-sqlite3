#include "./errable.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Create an simple errable") {
    neo::sqlite3::errable<int> e = neo::sqlite3::errc::cant_open;
    CHECK(e == neo::sqlite3::errc::cant_open);
}
