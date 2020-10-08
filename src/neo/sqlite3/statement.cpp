#include "./statement.hpp"

#include <neo/sqlite3/error.hpp>

#include <neo/sqlite3/c/sqlite3.h>

#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace neo::sqlite3;

void statement::_destroy() noexcept {
    ::sqlite3_finalize(c_ptr());
    _stmt_ptr = nullptr;
}

void statement::reset() noexcept {
    // Ignore the error code: It will return the same error code as the prior step()
    ::sqlite3_reset(c_ptr());
}

statement::state statement::step() {
    std::error_code ec;
    auto            result = step(ec);
    if (ec) {
        auto db = ::sqlite3_db_handle(c_ptr());
        throw_error(ec, "Failure while executing statement", ::sqlite3_errmsg(db));
    }
    return result;
}

statement::state statement::step(std::error_code& ec) noexcept {
    auto result = ::sqlite3_step(c_ptr());
    if (result == SQLITE_DONE) {
        return state::done;
    }
    if (result == SQLITE_ROW) {
        return state::more;
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

bool statement::is_busy() const noexcept { return ::sqlite3_stmt_busy(_stmt_ptr) != 0; }

#define OWNER_STMT_PTR (_owner.get().c_ptr())

value_ref row_access::operator[](int idx) const noexcept {
    auto col_count = ::sqlite3_column_count(OWNER_STMT_PTR);
    assert(
        _owner.get().is_busy()
        && "Attempt to access value from a row in an idle statement. "
           "`step()` was never called, or the statement needs to be `reset()`.");
    assert(idx < col_count && "Access to column beyond-the-end");
    auto val = ::sqlite3_column_value(OWNER_STMT_PTR, idx);
    return value_ref::from_ptr(reinterpret_cast<raw::sqlite3_value*>(val));
}

void binding_access::binding::bind(double d) {
    auto ec = to_error_code(::sqlite3_bind_double(OWNER_STMT_PTR, _index, d));
    throw_if_error(ec,
                   "sqlite3_bind_double()",
                   ::sqlite3_errmsg(::sqlite3_db_handle(OWNER_STMT_PTR)));
}

void binding_access::binding::bind(std::int64_t i) {
    auto ec = to_error_code(::sqlite3_bind_int64(OWNER_STMT_PTR, _index, i));
    throw_if_error(ec,
                   "sqlite3_bind_int64()",
                   ::sqlite3_errmsg(::sqlite3_db_handle(OWNER_STMT_PTR)));
}

void binding_access::binding::bind(const std::string& str) {
    auto ec = to_error_code(::sqlite3_bind_text64(OWNER_STMT_PTR,
                                                  _index,
                                                  str.data(),
                                                  str.size(),
                                                  SQLITE_TRANSIENT,
                                                  SQLITE_UTF8));
    throw_if_error(ec,
                   "sqlite3_bind_text64()",
                   ::sqlite3_errmsg(::sqlite3_db_handle(OWNER_STMT_PTR)));
}

void binding_access::binding::_bind_nocopy(std::string_view str) {
    auto ec = to_error_code(::sqlite3_bind_text64(OWNER_STMT_PTR,
                                                  _index,
                                                  str.data(),
                                                  str.size(),
                                                  nullptr,
                                                  SQLITE_UTF8));
    throw_if_error(ec,
                   "sqlite3_bind_text64()",
                   ::sqlite3_errmsg(::sqlite3_db_handle(OWNER_STMT_PTR)));
}

void binding_access::binding::bind(null_t) {
    auto ec = to_error_code(::sqlite3_bind_null(OWNER_STMT_PTR, _index));
    throw_if_error(ec,
                   "sqlite3_bind_null()",
                   ::sqlite3_errmsg(::sqlite3_db_handle(OWNER_STMT_PTR)));
}

void binding_access::binding::bind(zeroblob zb) {
    auto ec = to_error_code(::sqlite3_bind_zeroblob64(OWNER_STMT_PTR, _index, zb.size));
    throw_if_error(ec,
                   "sqlite3_bind_zeroblob64()",
                   ::sqlite3_errmsg(::sqlite3_db_handle(OWNER_STMT_PTR)));
}

void binding_access::clear() noexcept { ::sqlite3_clear_bindings(OWNER_STMT_PTR); }

int binding_access::named_parameter_index(const char* zstr) const noexcept {
    return ::sqlite3_bind_parameter_index(OWNER_STMT_PTR, zstr);
}

int column_access::count() const noexcept { return ::sqlite3_column_count(OWNER_STMT_PTR); }

std::string_view column::name() const noexcept {
    return ::sqlite3_column_name(OWNER_STMT_PTR, _index);
}

std::string_view column::origin_name() const noexcept {
    return ::sqlite3_column_origin_name(OWNER_STMT_PTR, _index);
}

std::string_view column::table_name() const noexcept {
    return ::sqlite3_column_table_name(OWNER_STMT_PTR, _index);
}

std::string_view column::database_name() const noexcept {
    return ::sqlite3_column_database_name(OWNER_STMT_PTR, _index);
}