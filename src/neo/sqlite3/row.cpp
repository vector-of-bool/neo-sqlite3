#include "./row.hpp"

#include "./statement.hpp"

#include <neo/assert.hpp>
#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

void row_access::_assert_running() const noexcept {
    neo_assert(expects,
               ::sqlite3_get_autocommit(::sqlite3_db_handle(_owner)) == 1,
               "Attempted to access value from a row in an idle statement. Either `step()` was "
               "never called, or the statement needs to be `reset()`");
}

void row_access::_assert_colcount(int idx) const noexcept {
    neo_assert(expects,
               idx < column_count(),
               "Access to column beyond-the-end",
               idx,
               column_count());
    std::terminate();
}
