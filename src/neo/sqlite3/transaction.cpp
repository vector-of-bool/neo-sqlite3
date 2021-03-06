#include "./transaction.hpp"

#include <neo/sqlite3/connection.hpp>
#include <neo/sqlite3/statement.hpp>

#include <neo/event.hpp>

#include <cstdio>
#include <exception>

using namespace neo::sqlite3;

recursive_transaction_guard::recursive_transaction_guard(connection_ref db) {
    if (!db.is_transaction_active()) {
        // There is no active transaction, so we are the top-most transaction
        // guard. Create an inner transaction guard to track the transaction
        _inner = transaction_guard(db);
    }
}

transaction_guard::transaction_guard(connection_ref db)
    : _db(db.c_ptr()) {
    _n_uncaught_exceptions = std::uncaught_exceptions();
    neo::emit(event::transaction_guard_begin{db});
    db.prepare("BEGIN")->run_to_completion().throw_if_error();
}

transaction_guard::~transaction_guard() noexcept(false) {
    if (_db == nullptr) {
        return;
    }

    bool is_failing = std::uncaught_exceptions() > _n_uncaught_exceptions;
    if (is_failing) {
        try {
            rollback();
        } catch (const std::exception& e) {
            std::fputs(
                "An exception occurred while rolling back a SQLite transaction due to another "
                "exception. The system may now be in an inconsistent state!\n",
                stderr);
            std::fputs("The exception message is '", stderr);
            std::fputs(e.what(), stderr);
            std::fputs("'\n", stderr);
        }
    } else {
        commit();
    }
}

void transaction_guard::commit() {
    neo_assert_always(expects,
                      _db != nullptr,
                      "transaction_guard::commit() on ended (or dropped) transaction");
    connection_ref db{_db};
    neo::emit(event::transaction_guard_commit{db});
    db.prepare("COMMIT")->run_to_completion().throw_if_error();
    drop();
}

void transaction_guard::rollback() {
    neo_assert_always(expects,
                      _db != nullptr,
                      "transaction_guard::rollback() on an ended (or dropped) transaction");
    connection_ref db{_db};
    neo::emit(event::transaction_guard_rollback{db});
    db.prepare("ROLLBACK")->run_to_completion().throw_if_error();
    drop();
}
