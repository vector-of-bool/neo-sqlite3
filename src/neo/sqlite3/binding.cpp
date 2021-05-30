#include "./binding.hpp"

#include "./database.hpp"
#include "./statement.hpp"

#include <neo/sqlite3/error.hpp>

#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

#define OWNER_STMT_PTR (_owner->c_ptr())

errable<void> binding::_bind_double(double d) {
    auto rc = errc{::sqlite3_bind_double(OWNER_STMT_PTR, _index, d)};
    if (is_error_rc(rc)) {
        return {rc, "sqlite3_bind_double() failed", _owner->database()};
    }
    return rc;
}

errable<void> binding::_bind_i64(std::int64_t i) {
    auto rc = errc{::sqlite3_bind_int64(OWNER_STMT_PTR, _index, i)};
    if (is_error_rc(rc)) {
        return {rc, "sqlite3_bind_int64() failed", _owner->database()};
    }
    return rc;
}

errable<void> binding::_bind_str_copy(std::string_view str) {
    auto rc = errc{::sqlite3_bind_text64(OWNER_STMT_PTR,
                                         _index,
                                         str.data(),
                                         str.size(),
                                         SQLITE_TRANSIENT,
                                         SQLITE_UTF8)};
    if (is_error_rc(rc)) {
        return {rc, "sqlite3_bind_text64() failed", _owner->database()};
    }
    return rc;
}

errable<void> binding::_bind_str_nocopy(std::string_view str) {
    auto rc = errc{::sqlite3_bind_text64(OWNER_STMT_PTR,
                                         _index,
                                         str.data(),
                                         str.size(),
                                         nullptr,
                                         SQLITE_UTF8)};
    if (is_error_rc(rc)) {
        return {rc, "sqlite3_bind_text64() failed", _owner->database()};
    }
    return rc;
}

errable<void> binding::_bind_null() {
    auto rc = errc{::sqlite3_bind_null(OWNER_STMT_PTR, _index)};
    if (is_error_rc(rc)) {
        return {rc, "sqlite3_bind_null() failed", _owner->database()};
    }
    return rc;
}

errable<void> binding::_bind_zeroblob(zeroblob zb) {
    auto rc = errc{::sqlite3_bind_zeroblob64(OWNER_STMT_PTR, _index, zb.size)};
    if (is_error_rc(rc)) {
        return {rc, "sqlite3_bind_zeroblob64() failed", _owner->database()};
    }
    return rc;
}

void binding_access::clear() noexcept { ::sqlite3_clear_bindings(OWNER_STMT_PTR); }

int binding_access::named_parameter_index(const char* zstr) const noexcept {
    return ::sqlite3_bind_parameter_index(OWNER_STMT_PTR, zstr);
}