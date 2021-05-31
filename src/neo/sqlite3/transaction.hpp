#pragma once

#include <optional>
#include <utility>

struct sqlite3;

namespace neo::sqlite3 {

class database_ref;

/**
 * @brief Scope-guard for database transactions.
 *
 * When constructed, executes BEGIN on the database.
 *
 * When destroyed:
 *
 *  - If commit(), rollback(), or drop() was called, does nothing.
 *  - If there are no additional exceptions in-flight (i.e. destroyed normally),
 *    executes a COMMIT.
 *  - Otherwise (i.e. an exception was thrown), executes a ROLLBACK.
 *
 * @note This type MUST NOT be constructed while another transaction
 *       is in-progress. This is guarded with an assertion. For recursive
 *       transaction management, use recursive_transaction_guard.
 */
class [[nodiscard]] transaction_guard {
    int        _n_uncaught_exceptions = 0;
    ::sqlite3* _db                    = nullptr;

public:
    // Open a new transaction on the given database
    explicit transaction_guard(database_ref db);
    ~transaction_guard() noexcept(false);

    // Move a transaction-guard. The transaction now lives as long as the moved-to transaction
    transaction_guard(transaction_guard&& other) noexcept
        : _n_uncaught_exceptions(other._n_uncaught_exceptions)
        , _db(std::exchange(other._db, nullptr)) {}

    transaction_guard& operator=(transaction_guard&& other) noexcept {
        std::swap(_n_uncaught_exceptions, other._n_uncaught_exceptions);
        std::swap(_db, other._db);
        return *this;
    }

    /// Immediately COMMIT. (Expects: !dropped())
    void commit();
    /// Immediately ROLLBACK. (Expects: !dropped())
    void rollback();
    /// Drop ownership of the transaction. It is now up to the caller to COMMIT or ROLLBACK
    void drop() noexcept { _db = nullptr; }

    /**
     * @brief Check whether commit(), rollback(), or drop() has been called.
     */
    [[nodiscard]] bool dropped() const noexcept { return _db == nullptr; }
};

/**
 * @brief Like transaction_guard, but allows recursive transaction guarding.
 *
 * Behaves identically to transaction guard, except if a transaction is already
 * open on the database when this object is constructed, then this object's
 * method become no-ops.
 */
class [[nodiscard]] recursive_transaction_guard {
    // We just wrap a transaction guard. This optional will only be engaged if
    // we are the top-level transaction.
    std::optional<transaction_guard> _inner;

public:
    explicit recursive_transaction_guard(database_ref db);
    ~recursive_transaction_guard() noexcept(false) {}

    /// COMMIT if we are the top-level transaction
    void commit() {
        if (is_top_transaction()) {
            _inner->commit();
        }
    }

    /// ROLLBACK if we are the top-level transaction
    void rollback() {
        if (is_top_transaction()) {
            _inner->rollback();
        }
    }

    /// If we are the top-level transaction, the transaction must now be managed
    /// by the caller. If we are not the top-level transaction, there may still
    /// be a caller at a higher scope who is still managing the transaction!!
    void drop() noexcept {
        if (is_top_transaction()) {
            _inner->drop();
        }
    }

    /// If we are the top-level transaction, like transaction_guard::dropped(),
    /// otherwise returns 'false'
    [[nodiscard]] bool dropped() const noexcept { return !_inner.has_value() || _inner->dropped(); }

    /// Determine if this guard is managing the current transaction
    [[nodiscard]] bool is_top_transaction() const noexcept { return _inner.has_value(); }
};

}  // namespace neo::sqlite3
