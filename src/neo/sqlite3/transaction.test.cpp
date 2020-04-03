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

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Nested transaction guards") {
    CHECK_FALSE(db.is_transaction_active());
    {
        neo::sqlite3::recursive_transaction_guard tr1{db};
        CHECK(db.is_transaction_active());
        {
            neo::sqlite3::recursive_transaction_guard tr2{db};
            CHECK(db.is_transaction_active());
        }
        CHECK(db.is_transaction_active());
    }
    CHECK_FALSE(db.is_transaction_active());
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Manual commit") {
    CHECK_FALSE(db.is_transaction_active());
    neo::sqlite3::transaction_guard tr{db};
    CHECK(db.is_transaction_active());
    // Commit the transaction, which will end the transaction
    tr.commit();
    CHECK_FALSE(db.is_transaction_active());
    CHECK(tr.dropped());
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Drop transaction guard") {
    CHECK_FALSE(db.is_transaction_active());
    neo::sqlite3::transaction_guard tr{db};
    CHECK(db.is_transaction_active());
    tr.drop();
    // Transaction is still active, we've just detatched the guard
    CHECK(db.is_transaction_active());
    // No-op, as it's been detatched
    CHECK(tr.dropped());
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Nested non-recursive transactions throws") {
    CHECK_FALSE(db.is_transaction_active());
    {
        neo::sqlite3::transaction_guard tr1{db};
        CHECK(db.is_transaction_active());
        CHECK_THROWS_AS(neo::sqlite3::transaction_guard(db),
                        neo::sqlite3::errc_error<neo::sqlite3::errc::error>);
        CHECK_NOTHROW(neo::sqlite3::recursive_transaction_guard(db));
        CHECK(db.is_transaction_active());
    }
    CHECK_FALSE(db.is_transaction_active());
}
