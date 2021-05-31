#include <neo/sqlite3/database.hpp>

#include <catch2/catch.hpp>

class sqlite3_memory_db_fixture {
public:
    neo::sqlite3::database db = *neo::sqlite3::create_memory_db();
};