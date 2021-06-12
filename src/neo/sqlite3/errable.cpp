#include "./errable.hpp"

#include "./error.hpp"

#include "./connection.hpp"

#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

namespace ns = neo::sqlite3;

void errable_base::throw_error() const {
    ns::throw_error(make_error_code(errc()),
                    _context ? _context : "[invalid errable<T> access]",
                    _dbptr ? ::sqlite3_errmsg(_dbptr) : "");
}

errable_base::errable_base(enum errc rc, const connection_ref& db) noexcept
    : _rc{rc}
    , _dbptr{db.c_ptr()} {}

errable_base::errable_base(enum errc rc, const char* context, const connection_ref& db) noexcept
    : _rc{rc}
    , _context{context}
    , _dbptr{db.c_ptr()} {}
