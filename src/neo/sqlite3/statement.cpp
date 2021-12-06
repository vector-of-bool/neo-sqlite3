#include "./statement.hpp"

#include "./connection.hpp"
#include "./errable.hpp"
#include "./error.hpp"

#include <neo/assert.hpp>
#include <neo/event.hpp>
#include <neo/scope.hpp>

#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

void statement::_destroy() noexcept {
    ::sqlite3_finalize(c_ptr());
    _stmt_ptr = nullptr;
}

void statement::reset() noexcept {
    // Ignore the error code: It will return the same error code as the prior step()
    ::sqlite3_reset(c_ptr());
}

errable<void> statement::run_to_completion() noexcept {
    auto res = step();
    while (res == errc::row) {
        res = step();
    }
    return res;
}

errable<void> statement::step() noexcept {
    if (neo::get_event_subscriber<event::step_first>() && !is_busy()) {
        neo::emit(event::step_first{*this});
    }
    auto result = ::sqlite3_step(c_ptr());
    neo_assert_always(expects,
                      result != SQLITE_MISUSE,
                      "neo::sqlite3 : The application has requested the advancement of a SQLite "
                      "statement while it is in an invalid state to do so. This is an issue in the "
                      "application or library, and it is not the fault of SQLite or of any user "
                      "action. We cannot safely continue, so the program will now be terminated.");
    neo::emit(event::step{*this, errc{result}});
    return errc{result};
}

bool statement::is_busy() const noexcept { return ::sqlite3_stmt_busy(_stmt_ptr) != 0; }

connection_ref statement::connection() noexcept {
    return connection_ref(::sqlite3_db_handle(c_ptr()));
}

connection_ref statement::database() noexcept {
    return connection_ref(::sqlite3_db_handle(c_ptr()));
}

std::string_view statement::sql_string() const noexcept {
    auto ptr = ::sqlite3_sql(c_ptr());
    return ptr;
}

std::string statement::expanded_sql_string() const noexcept {
    auto ptr = ::sqlite3_expanded_sql(c_ptr());
    neo_defer { ::sqlite3_free(ptr); };
    return std::string(ptr);
}

#define OWNER_STMT_PTR (_owner->c_ptr())

int column_access::count() const noexcept { return ::sqlite3_column_count(OWNER_STMT_PTR); }

std::string_view column::name() const noexcept {
    return ::sqlite3_column_name(OWNER_STMT_PTR, _index);
}

column column_access::operator[](int idx) const noexcept {
    neo_assert(expects, idx < count(), "Column index is out-of-range", idx, count());
    return column{*_owner, idx};
}

#ifdef SQLITE_ENABLE_COLUMN_METADATA
std::string_view column::origin_name() const noexcept {
    return ::sqlite3_column_origin_name(OWNER_STMT_PTR, _index);
}

std::string_view column::table_name() const noexcept {
    return ::sqlite3_column_table_name(OWNER_STMT_PTR, _index);
}

std::string_view column::database_name() const noexcept {
    return ::sqlite3_column_database_name(OWNER_STMT_PTR, _index);
}
#endif
