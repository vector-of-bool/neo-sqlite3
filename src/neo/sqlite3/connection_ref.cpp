#include "./connection_ref.hpp"

#include "./blob.hpp"
#include "./exec.hpp"
#include "./statement.hpp"

#include <neo/assert.hpp>
#include <neo/event.hpp>

#include <sqlite3/sqlite3.h>

using std::string_view;
using namespace neo;
using namespace neo::sqlite3;

void connection_ref::_assert_open() noexcept {
    neo_assert(expects,
               _ptr != nullptr,
               "neo::sqlite3::connection_ref was constructed from a null pointer.");
}

errable<statement> connection_ref::prepare(string_view query) noexcept {
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

errable<void> connection_ref::exec(zstring_view code) {
    neo::emit(event::exec_before{*this, code});
    auto rc = errc{::sqlite3_exec(c_ptr(), code.data(), nullptr, nullptr, nullptr)};
    neo::emit(event::exec_after{*this, code, rc});
    return {rc, "::sqlite3_exec() failed", *this};
}

errable<void> connection_ref::attach(std::string_view db_name, std::string_view filename) noexcept {
    NEO_SQLITE3_AUTO(st, prepare("ATTACH DATABASE ? AS ?"));
    return sqlite3::exec(st, filename, db_name);
}

errable<void> connection_ref::detach(std::string_view db_name) noexcept {
    NEO_SQLITE3_AUTO(st, prepare("DETACH DATABASE ?"));
    return sqlite3::exec(st, db_name);
}

errable<blob_io>
connection_ref::open_blob(zstring_view table, zstring_view column, std::int64_t rowid) {
    return open_blob("main", table, column, rowid);
}

errable<blob_io> connection_ref::open_blob(zstring_view db,
                                           zstring_view table,
                                           zstring_view column,
                                           std::int64_t rowid) {
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

    return blob_io(std::move(ret_ptr));
}
