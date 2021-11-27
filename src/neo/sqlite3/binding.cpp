#include "./binding.hpp"

#include "./connection.hpp"
#include "./statement.hpp"

#include <neo/sqlite3/error.hpp>

#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

errable<void> binding::_make_error(errc rc, const char* message) const noexcept {
    return {rc, message, connection_ref{::sqlite3_db_handle(_owner)}};
}

int binding_access::named_parameter_index(const char* zstr) const noexcept {
    return ::sqlite3_bind_parameter_index(_owner, zstr);
}