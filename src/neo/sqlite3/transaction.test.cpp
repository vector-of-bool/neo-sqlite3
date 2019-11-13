#include <neo/sqlite3/database.hpp>
#include <neo/sqlite3/transaction.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Create and drop a simple transaction") {
    auto db = neo::sqlite3::create_memory_db();
    CHECK_FALSE(db.is_transaction_active());
    {
        neo::sqlite3::transaction_guard tr{db};
        CHECK(db.is_transaction_active());
    }
    CHECK_FALSE(db.is_transaction_active());
}