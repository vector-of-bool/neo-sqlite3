#include "./transaction.hpp"

#include <neo/sqlite3/database.hpp>

#include <exception>
#include <iostream>

using namespace neo::sqlite3;

transaction_guard::transaction_guard(database& db) {
    _is_topmost            = !db.is_transaction_active();
    _n_uncaught_exceptions = std::uncaught_exceptions();
    _db                    = &db;
    db.prepare("BEGIN").run_to_completion();
}

transaction_guard::~transaction_guard() {
    if (_dropped || !_is_topmost) {
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
    if (_db == nullptr) {
        return;
    }

    _db->prepare("COMMIT").run_to_completion();
}

void transaction_guard::rollback() {
    if (_db == nullptr) {
        return;
    }

    _db->prepare("ROLLBACK").run_to_completion();
}