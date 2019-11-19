#include <neo/sqlite3/column_access.hpp>

#include "./tests.inl"

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
    REQUIRE(st.columns.count() == 6);
    CHECK(st.columns[0].name() == "person_name");
    CHECK(st.columns[0].origin_name() == "name");
    CHECK(st.columns[0].table_name() == "people");
    CHECK(st.columns[1].table_name() == "pets");
}
