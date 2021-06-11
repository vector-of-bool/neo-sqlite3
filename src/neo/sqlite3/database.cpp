#include "./database.hpp"

#include <neo/sqlite3/blob.hpp>
#include <neo/sqlite3/statement.hpp>

#include <neo/event.hpp>
#include <neo/ufmt.hpp>
#include <sqlite3/sqlite3.h>

using namespace neo;
using namespace neo::sqlite3;
using std::string;
using std::string_view;

errable<database> database::open(const string& db_name) noexcept {
    neo::emit(event::open_before{db_name});
    ::sqlite3* new_db = nullptr;
    auto       rc     = errc{::sqlite3_open(db_name.data(), &new_db)};
    if (rc != errc::ok) {
        neo::emit(event::open_error{db_name, rc});
        return {rc, "Failed to open database"};
    }
    // Enabled extended result codes on our new database
    ::sqlite3_extended_result_codes(new_db, 1);

    auto db = database(std::move(new_db));
    neo::emit(event::open_after{db_name, db});
    return db;
}

errable<statement> database_ref::prepare(string_view query) noexcept {
    const char*     str_tail = nullptr;
    ::sqlite3_stmt* stmt     = nullptr;

    neo::emit(event::prepare_before{*this, query});
    auto rc = errc{::sqlite3_prepare_v2(c_ptr(),
                                        query.data(),
                                        static_cast<int>(query.size()),
                                        &stmt,
                                        &str_tail)};
    if (rc != errc::ok) {
        neo::emit(event::prepare_error{*this, query, rc});
        return {rc, "Failure while preparing database statement", *this};
    }
    auto st = statement(std::move(stmt));
    neo::emit(event::prepare_after{*this, query, st});
    return st;
}

errable<void> database_ref::exec(const std::string& code) {
    neo::emit(event::exec_before{*this, code});
    auto rc = errc{::sqlite3_exec(c_ptr(), code.data(), nullptr, nullptr, nullptr)};
    neo::emit(event::exec_after{*this, code, rc});
    return {rc, "::sqlite3_exec() failed", *this};
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

errable<blob>
database_ref::open_blob(const string& table, const string& column, std::int64_t rowid) {
    return open_blob("main", table, column, rowid);
}

errable<blob> database_ref::open_blob(const string& db,
                                      const string& table,
                                      const string& column,
                                      std::int64_t  rowid) {
    ::sqlite3_blob* ret_ptr = nullptr;
    // TODO: Expose options for read-only blobs
    auto rc = errc{::sqlite3_blob_open(c_ptr(),
                                       db.data(),
                                       table.data(),
                                       column.data(),
                                       rowid,
                                       1,  // Do read-write
                                       &ret_ptr)};

    if (is_error_rc(rc)) {
        return {rc, "sqlite3_blob_open() failed", *this};
    }

    return blob(blob::from_raw(), ret_ptr);
}

std::string_view database_ref::error_message() const noexcept { return ::sqlite3_errmsg(c_ptr()); }

std::string_view database_ref::filename(const std::string& name) const noexcept {
    return ::sqlite3_db_filename(c_ptr(), name.c_str());
}

bool database_ref::is_readonly(const std::string& name) const noexcept {
    return ::sqlite3_db_readonly(c_ptr(), name.c_str());
}

void database_ref::interrupt() noexcept { ::sqlite3_interrupt(c_ptr()); }

errable<void> database_ref::attach(std::string_view db_name, std::string_view filename) noexcept {
    auto st = prepare("ATTACH DATABASE ? AS ?");
    if (!st.has_value()) {
        return st.errc();
    }
    auto err = st->bindings().bind_all(filename, db_name);
    if (err.is_error()) {
        return err;
    }
    return st->run_to_completion();
}

errable<void> database_ref::detach(std::string_view db_name) noexcept {
    auto st = prepare("DETACH DATABASE ?");
    if (!st.has_value()) {
        return st.errc();
    }
    auto err = st->bindings().bind_all(db_name);
    if (err.is_error()) {
        return err;
    }
    return st->run_to_completion();
}
