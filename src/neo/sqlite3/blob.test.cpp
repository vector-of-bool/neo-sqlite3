#include <neo/sqlite3/blob.hpp>
#include <neo/sqlite3/database.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Generate a large zero-blob") {
    auto db = neo::sqlite3::create_memory_db();
    db.prepare("CREATE TABLE stuff (data BLOB NOT NULL)").run_to_completion();
    auto st        = db.prepare("INSERT INTO stuff VALUES (?)");
    st.bindings[1] = neo::sqlite3::zeroblob(1024 * 1024);
    st.run_to_completion();
}

TEST_CASE("Fill a zero blob") {
    //
    auto db = neo::sqlite3::create_memory_db();
    db.prepare("CREATE TABLE stuff (data)").run_to_completion();
    db.prepare("INSERT INTO stuff VALUES (zeroblob(1024 * 1024))").run_to_completion();
}
