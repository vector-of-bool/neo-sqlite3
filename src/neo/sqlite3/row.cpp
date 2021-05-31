#include "./row.hpp"

#include "./statement.hpp"

#include <neo/assert.hpp>
#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

value_ref row_access::operator[](int idx) const noexcept {
    auto col_count = ::sqlite3_column_count(_owner->c_ptr());
    neo_assert(expects,
               _owner->is_busy(),
               "Attempted to access value from a row in an idle statement. Either `step()` was "
               "never called, or the statement needs to be `reset()`",
               idx);
    neo_assert(expects, idx < col_count, "Access to column beyond-the-end", idx, col_count);
    auto val = ::sqlite3_column_value(_owner->c_ptr(), idx);
    return value_ref(val);
}
