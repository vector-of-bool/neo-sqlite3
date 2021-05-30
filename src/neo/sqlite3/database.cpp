#include "./database.hpp"

#include <neo/sqlite3/blob.hpp>
#include <neo/sqlite3/statement.hpp>

#include <neo/ufmt.hpp>
#include <sqlite3/sqlite3.h>

using namespace neo;
using namespace neo::sqlite3;
using std::string;
using std::string_view;

std::optional<database> database::open(const string& db_name, std::error_code& ec) noexcept {
    ::sqlite3* new_db = nullptr;
    set_error_code(ec, ::sqlite3_open(db_name.data(), &new_db));
    if (!new_db) {
        return std::nullopt;
    }
    // Enabled extended result codes on our new database
    ::sqlite3_extended_result_codes(new_db, 1);

    return database(std::move(new_db));
}

database database::open(const std::string& s) {
    std::error_code         ec;
    std::optional<database> ret = open(s, ec);
    if (ec) {
        throw_error(ec, ufmt("Failed to open SQLite database [{}]", s), "[failed]");
    }
    return std::move(*ret);
}

errable<statement> database_ref::prepare(string_view query) noexcept {
    const char*     str_tail = nullptr;
    ::sqlite3_stmt* stmt     = nullptr;

    auto rc = errc{::sqlite3_prepare_v2(c_ptr(),
                                        query.data(),
                                        static_cast<int>(query.size()),
                                        &stmt,
                                        &str_tail)};
    if (rc != errc::ok) {
        return {rc, "Failure while preparing database statement", *this};
    }
    return statement(std::move(stmt));
}

errable<void> database_ref::exec(const std::string& code) {
    char* errmsg = nullptr;
    auto  rc     = errc{::sqlite3_exec(c_ptr(), code.data(), nullptr, nullptr, &errmsg)};
    if (rc != errc::ok) {
        return {errant, rc, "::sqlite3_exec() failed", *this};
    }
    return {};
}

void database::_close() noexcept { ::sqlite3_close(_exchange_ptr(nullptr)); }

bool database_ref::is_transaction_active() const noexcept {
    return ::sqlite3_get_autocommit(c_ptr()) == 0;
}

std::int64_t database_ref::last_insert_rowid() const noexcept {
    return ::sqlite3_last_insert_rowid(c_ptr());
}

int database_ref::changes() const noexcept { return ::sqlite3_changes(c_ptr()); }
int database_ref::total_changes() const noexcept { return ::sqlite3_total_changes(c_ptr()); }

blob database_ref::open_blob(const string& table, const string& column, std::int64_t rowid) {
    return open_blob("main", table, column, rowid);
}

std::optional<blob> database_ref::open_blob(const string&    table,
                                            const string&    column,
                                            std::int64_t     rowid,
                                            std::error_code& ec) {
    return open_blob("main", table, column, rowid, ec);
}

blob database_ref::open_blob(const string& db,
                             const string& table,
                             const string& column,
                             std::int64_t  rowid) {
    std::error_code ec;
    auto            ret = open_blob(db, table, column, rowid, ec);
    if (ec) {
        throw_error(ec,
                    ufmt("Failed to open BLOB in {}.{}.{} at rowid {}", db, table, column, rowid),
                    error_message());
    }
    return std::move(*ret);
}

std::optional<blob> database_ref::open_blob(const string&    db,
                                            const string&    table,
                                            const string&    column,
                                            std::int64_t     rowid,
                                            std::error_code& ec) {
    ::sqlite3_blob* ret_ptr = nullptr;
    // TODO: Expose options for read-only blobs
    auto rc = ::sqlite3_blob_open(c_ptr(),
                                  db.data(),
                                  table.data(),
                                  column.data(),
                                  rowid,
                                  1,  // Do read-write
                                  &ret_ptr);

    ec = to_error_code(rc);
    if (ec) {
        return std::nullopt;
    }

    return blob(blob::from_raw(), ret_ptr);
}

std::string_view database_ref::error_message() const noexcept { return ::sqlite3_errmsg(c_ptr()); }

void database_ref::interrupt() noexcept { ::sqlite3_interrupt(c_ptr()); }
