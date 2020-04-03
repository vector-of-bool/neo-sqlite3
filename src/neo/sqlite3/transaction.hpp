#pragma once

#include <optional>
#include <utility>

namespace neo::sqlite3 {

class database;

class transaction_guard {
    int       _n_uncaught_exceptions = 0;
    database* _db                    = nullptr;

public:
    explicit transaction_guard(database& db);
    ~transaction_guard();

    transaction_guard(transaction_guard&& other) noexcept
        : _n_uncaught_exceptions(other._n_uncaught_exceptions)
        , _db(std::exchange(other._db, nullptr)) {}

    transaction_guard& operator=(transaction_guard&& other) noexcept {
        std::swap(_n_uncaught_exceptions, other._n_uncaught_exceptions);
        std::swap(_db, other._db);
        return *this;
    }

    void commit();
    void rollback();
    void drop() noexcept { _db = nullptr; }

    bool dropped() const noexcept { return _db == nullptr; }
};

class recursive_transaction_guard {
    // We just wrap a transaction guard. This optional will only be engaged if
    // we are the top-level transaction.
    std::optional<transaction_guard> _inner;

public:
    explicit recursive_transaction_guard(database& db);

    void commit() {
        if (is_top_transaction()) {
            _inner->commit();
        }
    }

    void rollback() {
        if (is_top_transaction()) {
            _inner->rollback();
        }
    }

    void drop() noexcept {
        if (is_top_transaction()) {
            _inner->drop();
        }
    }

    bool dropped() const noexcept { return !_inner.has_value() || _inner->dropped(); }

    bool is_top_transaction() const noexcept { return _inner.has_value(); }
};

}  // namespace neo::sqlite3
