#include "./binding.hpp"

#include "./connection.hpp"
#include "./statement.hpp"

#include <neo/sqlite3/error.hpp>

#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

errable<void> binding::_make_error(errc rc, const char* message) noexcept {
    return {rc, message, connection_ref{::sqlite3_db_handle(_owner)}};
}

errable<void> binding::bind_double(double d) {
    auto rc = errc{::sqlite3_bind_double(_owner, _index, d)};
    if (is_error_rc(rc)) {
        return _make_error(rc, "sqlite3_bind_double() failed");
    }
    return rc;
}

errable<void> binding::bind_i64(std::int64_t i) {
    auto rc = errc{::sqlite3_bind_int64(_owner, _index, i)};
    if (is_error_rc(rc)) {
        return _make_error(rc, "sqlite3_bind_int64() failed");
    }
    return rc;
}

errable<void> binding::bind_str_copy(std::string_view str) {
    auto rc = errc{::sqlite3_bind_text64(_owner,
                                         _index,
                                         str.data(),
                                         str.size(),
                                         SQLITE_TRANSIENT,
                                         SQLITE_UTF8)};
    if (is_error_rc(rc)) {
        return _make_error(rc, "sqlite3_bind_text64() failed");
    }
    return rc;
}

errable<void> binding::bind_str_nocopy(std::string_view str) {
    auto rc
        = errc{::sqlite3_bind_text64(_owner, _index, str.data(), str.size(), nullptr, SQLITE_UTF8)};
    if (is_error_rc(rc)) {
        return _make_error(rc, "sqlite3_bind_text64() failed");
    }
    return rc;
}

errable<void> binding::bind_null() {
    auto rc = errc{::sqlite3_bind_null(_owner, _index)};
    if (is_error_rc(rc)) {
        return _make_error(rc, "sqlite3_bind_null() failed");
    }
    return rc;
}

errable<void> binding::bind_zeroblob(zeroblob zb) {
    auto rc = errc{::sqlite3_bind_zeroblob64(_owner, _index, zb.size)};
    if (is_error_rc(rc)) {
        return _make_error(rc, "sqlite3_bind_zeroblob64() failed");
    }
    return rc;
}

void binding_access::clear() noexcept { ::sqlite3_clear_bindings(_owner); }

int binding_access::named_parameter_index(const char* zstr) const noexcept {
    return ::sqlite3_bind_parameter_index(_owner, zstr);
}