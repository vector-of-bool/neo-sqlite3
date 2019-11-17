#include "./literal.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Create literals") {
    using namespace neo::sqlite3::literals;
    auto lit1 = "foo"_sql;
    auto lit2 = "bar"_sql;
    CHECK(lit1 != lit2);
    CHECK((lit1 < lit2 || lit2 < lit1));
}