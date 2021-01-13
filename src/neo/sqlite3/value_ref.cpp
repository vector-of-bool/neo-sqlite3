#include "./value_ref.hpp"

#include <neo/assert.hpp>
#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

#define MY_VALUE_PTR reinterpret_cast<::sqlite3_value*>(_value_ptr)

value_type value_ref::type() const noexcept {
    switch (::sqlite3_value_type(MY_VALUE_PTR)) {
    case SQLITE_INTEGER:
        return value_type::integer;
    case SQLITE_FLOAT:
        return value_type::real;
    case SQLITE_BLOB:
        return value_type::blob;
    case SQLITE_NULL:
        return value_type::null;
    case SQLITE_TEXT:
        return value_type::text;
    }
    neo_assert_always(ensures,
                      false,
                      "Unknown value from ::sqlite3_value_type(). This is a bug in neo-sqlite3");
}

std::int64_t value_ref::as_integer() const noexcept { return ::sqlite3_value_int64(MY_VALUE_PTR); }
double       value_ref::as_real() const noexcept { return ::sqlite3_value_double(MY_VALUE_PTR); }
std::string_view value_ref::as_text() const noexcept {
    auto ptr = reinterpret_cast<const char*>(::sqlite3_value_text(MY_VALUE_PTR));
    return ptr;
}
