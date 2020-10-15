#include <neo/sqlite3/statement.hpp>

#include "./tests.inl"

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Run a simple statement") {
    auto st = db.prepare("CREATE TABLE mine(name, age)");
    CHECK(st.step() == neo::sqlite3::statement::done);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Obtain a single value") {
    auto st = db.prepare("VALUES (42)");
    REQUIRE(st.columns().count() == 1);
    CHECK(st.step() == neo::sqlite3::statement::more);
    CHECK(st.row()[0].is_integer());
    CHECK(st.row()[0].as_integer() == 42);
    CHECK(st.step() == neo::sqlite3::statement::done);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Check values") {
    auto st = db.prepare("VALUES (42, 42.1, '42')");
    REQUIRE(st.columns().count() == 3);
    REQUIRE(st.step() == neo::sqlite3::statement::more);
    SECTION("With true types") {
        CHECK(st.row()[0].is_integer());
        CHECK(st.row()[0].as_integer() == 42);
        CHECK(st.row()[1].is_real());
        CHECK(st.row()[1].as_real() == 42.1);
        CHECK(st.row()[2].is_text());
        CHECK(st.row()[2].as_text() == "42");
    }
    SECTION("Implicit value casts") {
        CHECK(st.row()[0].is_integer());
        CHECK(st.row()[0].as_real() == 42);
        CHECK(st.row()[1].is_real());
        CHECK(st.row()[1].as_integer() == 42);
        CHECK(st.row()[2].is_text());
        CHECK(st.row()[2].as_integer() == 42);
    }
    CHECK(st.step() == neo::sqlite3::statement::done);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Unpack tuples") {
    auto st = db.prepare("VALUES (1, 2, 'I am a string')");
    REQUIRE(st.step() == neo::sqlite3::statement::more);
    REQUIRE(st.columns().count() == 3);
    auto [i1, i2, str] = st.row().unpack<int, int, std::string_view>();
    CHECK(i1 == 1);
    CHECK(i2 == 2);
    CHECK(str == "I am a string");
    CHECK(st.step() == neo::sqlite3::statement::done);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Numeric bind") {
    auto st = db.prepare("VALUES (?, ?)");
    // st.bind(0, "cat");
    // st.bind(1, 9);
    st.bindings()[1] = 42;
    st.bindings()[2] = "cat";
    CHECK(st.step() == neo::sqlite3::statement::more);
    CHECK(st.row()[0].as_integer() == 42);
    CHECK(st.row()[1].as_text() == "cat");
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Named bind") {
    auto st               = db.prepare("VALUES (:foo, :bar)");
    st.bindings()[":foo"] = 22;
    st.bindings()[":bar"] = "Cats";
    CHECK(st.step() == neo::sqlite3::statement::more);
    CHECK(st.row().unpack<int, std::string_view>() == std::tuple(22, "Cats"));
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Tuple bind") {
    auto       st = db.prepare("VALUES (?, ?, ?, ?)");
    std::tuple tup{1, 2, "string", -9.2};
    st.bindings() = tup;
    CHECK(st.step() == neo::sqlite3::statement::more);
    CHECK(st.row().unpack<int, int, std::string_view, double>() == tup);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Access column information") {
    db.prepare("CREATE TABLE people (name, age, job)").run_to_completion();
    db.prepare("CREATE TABLE pets (name, species, owner)").run_to_completion();
    auto st = db.prepare(R"(
        SELECT
            people.name AS person_name,
            pets.name AS pet_name,
            12,
            age,
            job,
            species
        FROM people
        JOIN pets ON owner = people.name
    )");
    REQUIRE(st.columns().count() == 6);
    CHECK(st.columns()[0].name() == "person_name");
    CHECK(st.columns()[0].origin_name() == "name");
    CHECK(st.columns()[0].table_name() == "people");
    CHECK(st.columns()[1].table_name() == "pets");
}
