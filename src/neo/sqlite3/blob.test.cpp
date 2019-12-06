#include <neo/sqlite3/blob.hpp>

#include "./tests.inl"

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Generate a large zero-blob") {
    db.prepare("CREATE TABLE stuff (data BLOB NOT NULL)").run_to_completion();
    auto st        = db.prepare("INSERT INTO stuff VALUES (?)");
    st.bindings[1] = neo::sqlite3::zeroblob(1024 * 1024);
    st.run_to_completion();
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Fill a zero blob") {
    db.prepare("CREATE TABLE stuff (data)").run_to_completion();
    db.prepare("INSERT INTO stuff VALUES (zeroblob(1024 * 1024))").run_to_completion();
}
