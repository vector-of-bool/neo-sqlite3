#include "./transaction.hpp"

#include <neo/sqlite3/database.hpp>
#include <neo/sqlite3/statement.hpp>

#include <exception>
#include <iostream>

using namespace neo::sqlite3;

recursive_transaction_guard::recursive_transaction_guard(database& db) {
    if (!db.is_transaction_active()) {
        // There is no active transaction, so we are the top-most transaction
        // guard. Create an inner transaction guard to track the transaction
        _inner = transaction_guard(db);
    }
}

transaction_guard::transaction_guard(database& db) {
    _n_uncaught_exceptions = std::uncaught_exceptions();
    _db                    = &db;
    db.prepare("BEGIN").run_to_completion();
}

transaction_guard::~transaction_guard() {
    if (_db == nullptr) {
        return;
    }

    bool is_failing = std::uncaught_exceptions() > _n_uncaught_exceptions;
    if (is_failing) {
        try {
            rollback();
        } catch (const std::exception& e) {
            std::cerr << "An exception occurred while rolling back a SQLite transaction due to "
                         "another exception. The system may now be in an inconsistent state!\n";
            std::cerr << "The exception message: " << e.what() << '\n';
        }
    } else {
        commit();
    }
}

void transaction_guard::commit() {
    neo_assert_always(expects,
                      _db != nullptr,
                      "transaction_guard::commit() on ended (or dropped) transaction");
    _db->prepare("COMMIT").run_to_completion();
    drop();
}

void transaction_guard::rollback() {
    neo_assert_always(expects,
                      _db != nullptr,
                      "transaction_guard::rollback() on an ended (or dropped) transaction");
    _db->prepare("ROLLBACK").run_to_completion();
    drop();
}
