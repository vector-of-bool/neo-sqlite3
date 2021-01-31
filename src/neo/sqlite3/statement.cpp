#include "./statement.hpp"

#include <neo/sqlite3/error.hpp>

#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

void statement::_destroy() noexcept {
    ::sqlite3_finalize(c_ptr());
    _stmt_ptr = nullptr;
}

void statement::reset() noexcept {
    // Ignore the error code: It will return the same error code as the prior step()
    ::sqlite3_reset(c_ptr());
}

errc statement::step() {
    auto result = step(std::nothrow);
    if (result != errc::done && result != errc::row) {
        auto db = ::sqlite3_db_handle(c_ptr());
        auto ec = make_error_code(result);
        throw_error(ec, "Failure while executing statement", ::sqlite3_errmsg(db));
    }
    return errc{result};
}

errc statement::step(std::error_code& ec) noexcept {
    auto rc = step(std::nothrow);
    ec      = make_error_code(rc);
    return rc;
}

errc statement::step(std::nothrow_t) noexcept {
    auto result = ::sqlite3_step(c_ptr());
    neo_assert_always(expects,
                      result != SQLITE_MISUSE,
                      "neo::sqlite3 : The application has requested the advancement of a SQLite "
                      "statement while it is in an invalid state to do so. This is an issue in the "
                      "application or library, and it is not the fault of SQLite or of any user "
                      "action. We cannot safely continue, so the program will now be terminated.");
    return errc{result};
}

bool statement::is_busy() const noexcept { return ::sqlite3_stmt_busy(_stmt_ptr) != 0; }

#define OWNER_STMT_PTR (_owner.get().c_ptr())

value_ref row_access::operator[](int idx) const noexcept {
    auto col_count = ::sqlite3_column_count(OWNER_STMT_PTR);
    neo_assert(expects,
               _owner.get().is_busy(),
               "Attemptd to access value from a row in and idle statement. Either `step()` was "
               "never called, or the statement needs to be `reset()`",
               idx);
    neo_assert(expects, idx < col_count, "Access to column beyond-the-end", idx, col_count);
    auto val = ::sqlite3_column_value(OWNER_STMT_PTR, idx);
    return value_ref(val);
}

int column_access::count() const noexcept { return ::sqlite3_column_count(OWNER_STMT_PTR); }

std::string_view column::name() const noexcept {
    return ::sqlite3_column_name(OWNER_STMT_PTR, _index);
}

#ifdef SQLITE_ENABLE_COLUMN_METADATA
std::string_view column::origin_name() const noexcept {
    return ::sqlite3_column_origin_name(OWNER_STMT_PTR, _index);
}

std::string_view column::table_name() const noexcept {
    return ::sqlite3_column_table_name(OWNER_STMT_PTR, _index);
}

std::string_view column::database_name() const noexcept {
    return ::sqlite3_column_database_name(OWNER_STMT_PTR, _index);
}
#endif
