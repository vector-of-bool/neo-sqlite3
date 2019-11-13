#include <neo/sqlite3/database.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Run a simple statement") {
    auto db = neo::sqlite3::create_memory_db();
    auto st = db.prepare("CREATE TABLE mine(name, age)");
    CHECK(st.step() == neo::sqlite3::statement::done);
}

TEST_CASE("Obtain a single value") {
    auto db = neo::sqlite3::create_memory_db();
    auto st = db.prepare("VALUES (42)");
    CHECK(st.row.size() == 1);
    CHECK(st.step() == neo::sqlite3::statement::more);
    CHECK(st.row[0].is_integer());
    CHECK(st.row[0].as_integer() == 42);
    CHECK(st.step() == neo::sqlite3::statement::done);
}

TEST_CASE("Check values") {
    auto db = neo::sqlite3::create_memory_db();
    auto st = db.prepare("VALUES (42, 42.1, '42')");
    CHECK(st.row.size() == 3);
    REQUIRE(st.step() == neo::sqlite3::statement::more);
    SECTION("With true types") {
        CHECK(st.row[0].is_integer());
        CHECK(st.row[0].as_integer() == 42);
        CHECK(st.row[1].is_real());
        CHECK(st.row[1].as_real() == 42.1);
        CHECK(st.row[2].is_text());
        CHECK(st.row[2].as_text() == "42");
    }
    SECTION("Implicit value casts") {
        CHECK(st.row[0].is_integer());
        CHECK(st.row[0].as_real() == 42);
        CHECK(st.row[1].is_real());
        CHECK(st.row[1].as_integer() == 42);
        CHECK(st.row[2].is_text());
        CHECK(st.row[2].as_integer() == 42);
    }
    CHECK(st.step() == neo::sqlite3::statement::done);
}

TEST_CASE("Unpack tuples") {
    auto db = neo::sqlite3::create_memory_db();
    auto st = db.prepare("VALUES (1, 2, 'I am a string')");
    REQUIRE(st.step() == neo::sqlite3::statement::more);
    REQUIRE(st.row.size() == 3);
    auto [i1, i2, str] = st.row.unpack<int, int, std::string_view>();
    CHECK(i1 == 1);
    CHECK(i2 == 2);
    CHECK(str == "I am a string");
    CHECK(st.step() == neo::sqlite3::statement::done);
}

TEST_CASE("Numeric bind") {
    auto db = neo::sqlite3::create_memory_db();
    auto st = db.prepare("VALUES (?, ?)");
    // st.bind(0, "cat");
    // st.bind(1, 9);
    st.bindings[1] = 42;
    st.bindings[2] = "cat";
    CHECK(st.step() == neo::sqlite3::statement::more);
    CHECK(st.row[0].as_integer() == 42);
    CHECK(st.row[1].as_text() == "cat");
}

TEST_CASE("Named bind") {
    auto db             = neo::sqlite3::create_memory_db();
    auto st             = db.prepare("VALUES (:foo, :bar)");
    st.bindings[":foo"] = 22;
    st.bindings[":bar"] = "Cats";
    CHECK(st.step() == neo::sqlite3::statement::more);
    CHECK(st.row.unpack<int, std::string_view>() == std::tuple(22, "Cats"));
}

TEST_CASE("Tuple bind") {
    auto db = neo::sqlite3::create_memory_db();
    auto st = db.prepare("VALUES (?, ?, ?, ?)");
    std::tuple tup{1, 2, "string", -9.2};
    st.bindings = tup;
    CHECK(st.step() == neo::sqlite3::statement::more);
    CHECK(st.row.unpack<int, int, std::string_view, double>() == tup);
}