#pragma once

#include <utility>

namespace neo::sqlite3 {

class database;

class transaction_guard {
    int       _n_uncaught_exceptions = 0;
    bool      _dropped               = false;
    database* _db                    = nullptr;
    bool      _is_topmost            = false;

public:
    explicit transaction_guard(database& db);
    ~transaction_guard();

    transaction_guard(transaction_guard&& other)
        : _n_uncaught_exceptions(other._n_uncaught_exceptions)
        , _dropped(other._dropped)
        , _db(std::exchange(other._db, nullptr)) {}

    transaction_guard& operator=(transaction_guard&& other) noexcept {
        std::swap(_n_uncaught_exceptions, other._n_uncaught_exceptions);
        std::swap(_dropped, other._dropped);
        std::swap(_db, other._db);
        return *this;
    }

    void commit();
    void rollback();
    void drop() noexcept { _dropped = true; }

    bool is_top_transaction() const noexcept { return _is_topmost; }
};

}  // namespace neo::sqlite3