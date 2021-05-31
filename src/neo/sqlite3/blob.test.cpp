#include <neo/sqlite3/blob.hpp>
#include <neo/sqlite3/statement.hpp>

#include "./tests.inl"

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Generate a large zero-blob") {
    db.prepare("CREATE TABLE stuff (data BLOB NOT NULL)")->run_to_completion().throw_if_error();
    auto st          = *db.prepare("INSERT INTO stuff VALUES (?)");
    st.bindings()[1] = neo::sqlite3::zeroblob{1024 * 1024};
    st.run_to_completion().throw_if_error();
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Fill a zero blob") {
    db.prepare("CREATE TABLE stuff (data)")->run_to_completion().throw_if_error();
    db.prepare("INSERT INTO stuff VALUES (zeroblob(1024 * 1024))")
        ->run_to_completion()
        .throw_if_error();
}
