#include "./statement.hpp"

#include <neo/sqlite3/error.hpp>

#include <neo/sqlite3/c/sqlite3.h>

#include <cassert>
#include <iostream>
#include <stdexcept>

#define MY_STMT_PTR static_cast<::sqlite3_stmt*>(_stmt_ptr)

using namespace neo::sqlite3;

void statement::_destroy() noexcept {
    ::sqlite3_finalize(MY_STMT_PTR);
    _stmt_ptr = nullptr;
}

void statement::reset() noexcept {
    // Ignore the error code: It will return the same error code as the prior step()
    ::sqlite3_reset(MY_STMT_PTR);
}

statement::state statement::step(std::error_code& ec) noexcept {
    auto result = ::sqlite3_step(MY_STMT_PTR);
    if (result == SQLITE_DONE) {
        return state::done;
    }
    if (result == SQLITE_ROW) {
        return state::more;
    }
    if (result == SQLITE_BUSY) {
        ec = make_error_code(errc::busy);
        return state::error;
    }
    if (result == SQLITE_MISUSE) {
        std::cerr << "neo::sqlite3 : The application has requested the advancement of a SQLite "
                     "statement while it is in an invalid state to do so. This is an issue in the "
                     "application or library, and it is not the fault of SQLite or of any user "
                     "action. We cannot safely continue, so the program will now be terminated.\n";
        std::terminate();
    }
    ec = to_error_code(result);
    return state::error;
}

#define OWNER_STMT_PTR static_cast<::sqlite3_stmt*>(_owner->_stmt_ptr)

std::size_t row_access::size() const noexcept { return ::sqlite3_column_count(OWNER_STMT_PTR); }

value_ref row_access::operator[](int idx) const noexcept {
    auto col_count = ::sqlite3_column_count(OWNER_STMT_PTR);
    assert(
        ::sqlite3_stmt_busy(OWNER_STMT_PTR)
        && "Attempt to access value from a row in an idle statement. "
           "`step()` was never called, or the statement needs to be `reset()`.");
    assert(idx < col_count && "Access to column beyond-the-end");
    auto val = ::sqlite3_column_value(OWNER_STMT_PTR, idx);
    return value_ref::from_ptr(val);
}

void binding_access::binding::bind(double d) {
    auto ec = to_error_code(::sqlite3_bind_double(OWNER_STMT_PTR, _index, d));
    throw_if_error(ec, "sqlite3_bind_double()");
}

void binding_access::binding::bind(std::int64_t i) {
    auto ec = to_error_code(::sqlite3_bind_int64(OWNER_STMT_PTR, _index, i));
    throw_if_error(ec, "sqlite3_bind_int64()");
}

void binding_access::binding::bind(std::string_view str) {
    auto ec = to_error_code(::sqlite3_bind_text64(OWNER_STMT_PTR,
                                                  _index,
                                                  str.data(),
                                                  str.size(),
                                                  nullptr,
                                                  SQLITE_UTF8));
    throw_if_error(ec, "sqlite3_bind_text64()");
}

void binding_access::binding::bind(null_t) {
    auto ec = to_error_code(::sqlite3_bind_null(OWNER_STMT_PTR, _index));
    throw_if_error(ec, "sqlite3_bind_null()");
}

void binding_access::binding::bind(zeroblob zb) {
    auto ec = to_error_code(::sqlite3_bind_zeroblob64(OWNER_STMT_PTR, _index, zb.size));
    throw_if_error(ec, "sqlite3_bind_zeroblob64()");
}

void binding_access::clear() noexcept { ::sqlite3_clear_bindings(OWNER_STMT_PTR); }

int binding_access::named_parameter_index(const char* zstr) const noexcept {
    return ::sqlite3_bind_parameter_index(OWNER_STMT_PTR, zstr);
}
