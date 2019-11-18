#include "./column_access.hpp"

#include <neo/sqlite3/c/sqlite3.h>

#include <neo/sqlite3/statement.hpp>

using namespace neo::sqlite3;

#define OWNER_STMT_PTR static_cast<::sqlite3_stmt*>(_owner._stmt_ptr)

int column_access::count() const noexcept { return ::sqlite3_column_count(OWNER_STMT_PTR); }

std::string_view column_access::column::name() const noexcept {
    return ::sqlite3_column_name(OWNER_STMT_PTR, _index);
}

std::string_view column_access::column::origin_name() const noexcept {
    return ::sqlite3_column_origin_name(OWNER_STMT_PTR, _index);
}

std::string_view column_access::column::table_name() const noexcept {
    return ::sqlite3_column_table_name(OWNER_STMT_PTR, _index);
}

std::string_view column_access::column::database_name() const noexcept {
    return ::sqlite3_column_database_name(OWNER_STMT_PTR, _index);
}