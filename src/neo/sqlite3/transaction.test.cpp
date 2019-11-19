#include <neo/sqlite3/transaction.hpp>

#include "./tests.inl"

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Create and drop a simple transaction") {
    CHECK_FALSE(db.is_transaction_active());
    {
        neo::sqlite3::transaction_guard tr{db};
        CHECK(db.is_transaction_active());
    }
    CHECK_FALSE(db.is_transaction_active());
}