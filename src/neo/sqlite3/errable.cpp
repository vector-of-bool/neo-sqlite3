#include "./errable.hpp"

#include "./error.hpp"

#include "./connection_ref.hpp"

#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

namespace ns = neo::sqlite3;

void errable_base::throw_error() const {
    ns::throw_error(make_error_code(errc()),
                    error().context() ? error().context() : "[invalid errable<T> access]",
                    error().dbptr() ? ::sqlite3_errmsg(error().dbptr()) : "");
}

errable_base::errable_base(enum errc rc, const connection_ref& db) noexcept
    : _error(rc, nullptr, db.c_ptr()) {}

errable_base::errable_base(enum errc rc, const char* context, const connection_ref& db) noexcept
    : _error(rc, context, db.c_ptr()) {}
