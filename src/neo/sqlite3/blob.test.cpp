#include <neo/sqlite3/blob.hpp>
#include <neo/sqlite3/exec.hpp>
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

    auto st     = *db.prepare("SELECT * FROM stuff");
    auto [blob] = *neo::sqlite3::next<neo::sqlite3::blob_view>(st);

    std::string zero;
    zero.resize(1024 * 1024);
    REQUIRE(blob.size() == zero.size());
    CHECK(zero == std::string_view(reinterpret_cast<const char*>(blob.data()), blob.size()));
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Create a blob of text") {
    db.exec("CREATE TABLE stuff (data BLOB)").throw_if_error();

    std::string s = "I am a text string";
    neo::sqlite3::exec(*db.prepare("INSERT INTO stuff(data) values(?)"), neo::sqlite3::blob_view(s))
        .throw_if_error();
}