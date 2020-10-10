#include "./database.hpp"

#include <neo/sqlite3/blob.hpp>
#include <neo/sqlite3/c/sqlite3.h>

#include <neo/ufmt.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

using namespace neo;
using namespace neo::sqlite3;
using std::string;
using std::string_view;

#define MY_DB_PTR reinterpret_cast<::sqlite3*>(this->_ptr)

std::optional<database> database::open(const string& db_name, std::error_code& ec) noexcept {
    ec                = {};
    ::sqlite3* new_db = nullptr;
    set_error_code(ec, ::sqlite3_open(db_name.data(), &new_db));
    if (ec) {
        return std::nullopt;
    }
    // Enabled extended result codes on our new database
    ::sqlite3_extended_result_codes(new_db, 1);

    database ret;
    ret._ptr = reinterpret_cast<raw::sqlite3*>(new_db);
    return ret;
}

database database::open(const std::string& s) {
    std::error_code         ec;
    std::optional<database> ret = open(s, ec);
    if (ec) {
        throw_error(ec, ufmt("Failed to open SQLite database [{}]", s), "[failed]");
    }
    return std::move(*ret);
}

std::optional<statement> database::prepare(string_view query, std::error_code& ec) noexcept {
    ec                       = {};
    const char*     str_tail = nullptr;
    ::sqlite3_stmt* stmt     = nullptr;
    set_error_code(ec,
                   ::sqlite3_prepare_v2(MY_DB_PTR,
                                        query.data(),
                                        static_cast<int>(query.size()),
                                        &stmt,
                                        &str_tail));
    if (ec) {
        return std::nullopt;
    }
    return statement(std::move(stmt));
}

statement database::prepare(std::string_view query) {
    std::error_code ec;
    auto            ret = prepare(query, ec);
    if (ec) {
        throw_error(ec, ufmt("Failed to prepare statement [[{}]]", query), error_message());
    }
    return std::move(*ret);
}

void database::exec(const std::string& code) {
    char* errmsg = nullptr;
    auto  rc     = ::sqlite3_exec(MY_DB_PTR, code.data(), nullptr, nullptr, &errmsg);
    if (rc) {
        throw_error(to_error_code(rc), "::sqlite3_exec() failed", errmsg);
    }
}

database::~database() { ::sqlite3_close(MY_DB_PTR); }

bool database::is_transaction_active() const noexcept {
    return ::sqlite3_get_autocommit(MY_DB_PTR) == 0;
}

std::int64_t database::last_insert_rowid() const noexcept {
    return ::sqlite3_last_insert_rowid(MY_DB_PTR);
}

blob database::open_blob(const string& table, const string& column, std::int64_t rowid) {
    return open_blob("main", table, column, rowid);
}

std::optional<blob> database::open_blob(const string&    table,
                                        const string&    column,
                                        std::int64_t     rowid,
                                        std::error_code& ec) {
    return open_blob("main", table, column, rowid, ec);
}

blob database::open_blob(const string& db,
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

std::optional<blob> database::open_blob(const string&    db,
                                        const string&    table,
                                        const string&    column,
                                        std::int64_t     rowid,
                                        std::error_code& ec) {
    ::sqlite3_blob* ret_ptr = nullptr;
    // TODO: Expose options for read-only blobs
    auto rc = ::sqlite3_blob_open(MY_DB_PTR,
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

std::string_view database::error_message() const noexcept { return ::sqlite3_errmsg(MY_DB_PTR); }