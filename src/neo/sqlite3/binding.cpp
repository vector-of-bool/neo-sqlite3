#include "./binding.hpp"

#include "./statement.hpp"

#include <neo/sqlite3/error.hpp>

#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

#define OWNER_STMT_PTR (_owner.get().c_ptr())

void binding::_bind_double(double d) {
    auto ec = to_error_code(::sqlite3_bind_double(OWNER_STMT_PTR, _index, d));
    throw_if_error(ec,
                   "sqlite3_bind_double()",
                   ::sqlite3_errmsg(::sqlite3_db_handle(OWNER_STMT_PTR)));
}

void binding::_bind_i64(std::int64_t i) {
    auto ec = to_error_code(::sqlite3_bind_int64(OWNER_STMT_PTR, _index, i));
    throw_if_error(ec,
                   "sqlite3_bind_int64()",
                   ::sqlite3_errmsg(::sqlite3_db_handle(OWNER_STMT_PTR)));
}

void binding::_bind_str_copy(std::string_view str) {
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

void binding::_bind_str_nocopy(std::string_view str) {
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

void binding::_bind_null() {
    auto ec = to_error_code(::sqlite3_bind_null(OWNER_STMT_PTR, _index));
    throw_if_error(ec,
                   "sqlite3_bind_null()",
                   ::sqlite3_errmsg(::sqlite3_db_handle(OWNER_STMT_PTR)));
}

void binding::_bind_zeroblob(zeroblob zb) {
    auto ec = to_error_code(::sqlite3_bind_zeroblob64(OWNER_STMT_PTR, _index, zb.size));
    throw_if_error(ec,
                   "sqlite3_bind_zeroblob64()",
                   ::sqlite3_errmsg(::sqlite3_db_handle(OWNER_STMT_PTR)));
}

void binding_access::clear() noexcept { ::sqlite3_clear_bindings(OWNER_STMT_PTR); }

int binding_access::named_parameter_index(const char* zstr) const noexcept {
    return ::sqlite3_bind_parameter_index(OWNER_STMT_PTR, zstr);
}