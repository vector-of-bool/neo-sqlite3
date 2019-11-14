#include <neo/sqlite3/column_access.hpp>

#include <neo/sqlite3/database.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Access column information") {
    auto db = neo::sqlite3::create_memory_db();
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
    REQUIRE(st.columns.count() == 6);
    CHECK(st.columns[0].name() == "person_name");
    CHECK(st.columns[0].origin_name() == "name");
    CHECK(st.columns[0].table_name() == "people");
    CHECK(st.columns[1].table_name() == "pets");
}
