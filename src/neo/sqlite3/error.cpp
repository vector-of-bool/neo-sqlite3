#include "./error.hpp"

#include <neo/sqlite3/c/sqlite3.h>

#include <neo/assert.hpp>

#include <cassert>
#include <iostream>

namespace {

class sqlite3_category : public std::error_category {
    const char* name() const noexcept override { return "neo::sqlite3"; }
    std::string message(int e) const override { return ::sqlite3_errstr(e); }
    /**
     * Given an extended result code from the SQLite API, convert it to the more generic forms in a
     * error_condition
     */
    std::error_condition default_error_condition(int errcode) const noexcept override {
        auto cond = neo::sqlite3::error_code_condition(static_cast<neo::sqlite3::errc>(errcode));
        return std::error_condition(static_cast<int>(cond), *this);
    }
};

}  // namespace

const std::error_category& neo::sqlite3::error_category() noexcept {
    static sqlite3_category inst;
    return inst;
}

[[noreturn]] void neo::sqlite3::throw_error(const std::error_code& ec,
                                            std::string_view       message,
                                            std::string_view       sup) {
    neo_assert_always(expects,
                      ec.category() == sqlite3::error_category(),
                      "neo::sqlite3::throw_error() should only be given errors codes of the "
                      "neo::sqlite3::error_category() category",
                      ec.message(),
                      ec.value(),
                      ec.category().name());
    auto e_ec = static_cast<errc>(ec.value());
    switch (e_ec) {
#define CASE(Code)                                                                                 \
    case errc::Code:                                                                               \
        throw errc_error<errc::Code>(message, sup)

        CASE(abort);
        CASE(auth);
        CASE(busy);
        CASE(cant_open);
        CASE(constraint);
        CASE(corrupt);
        CASE(done);
        CASE(empty);
        CASE(error);
        CASE(format);
        CASE(full);
        CASE(internal);
        CASE(interrupt);
        CASE(ioerr);
        CASE(locked);
        CASE(mismatch);
        CASE(misuse);
        CASE(nolfs);
        CASE(no_memory);
        CASE(not_a_database);
        CASE(not_found);
        CASE(notice);
        CASE(ok);
        CASE(perm);
        CASE(protocol);
        CASE(range);
        CASE(readonly);
        CASE(row);
        CASE(schema);
        CASE(too_big);
        CASE(warning);
        CASE(abort_rollback);
        CASE(busy_recovery);
        CASE(busy_snapshot);
        CASE(cant_open_convert_path);
        CASE(cant_open_full_path);
        CASE(cant_open_is_directory);
        CASE(cant_open_no_temp_directory);
        CASE(constraint_check);
        CASE(constraint_commit_hook);
        CASE(constraint_foreign_key);
        CASE(constraint_function);
        CASE(constraint_not_null);
        CASE(constraint_primary_key);
        CASE(constraint_rowid);
        CASE(constraint_trigger);
        CASE(constraint_unique);
        CASE(constraint_vtab);
        CASE(corrupt_vtab);
        CASE(ioerr_access);
        CASE(ioerr_blocked);
        CASE(ioerr_check_reserved_lock);
        CASE(ioerr_close);
        CASE(ioerr_convert_path);
        CASE(ioerr_delete);
        CASE(ioerr_delete_noent);
        CASE(ioerr_dir_close);
        CASE(ioerr_dir_fsync);
        CASE(ioerr_fstat);
        CASE(ioerr_fsync);
        CASE(ioerr_gettemppath);
        CASE(ioerr_lock);
        CASE(ioerr_mmap);
        CASE(ioerr_nomem);
        CASE(ioerr_rdlock);
        CASE(ioerr_read);
        CASE(ioerr_seek);
        CASE(ioerr_shmlock);
        CASE(ioerr_shmmap);
        CASE(ioerr_shmopen);
        CASE(ioerr_shmsize);
        CASE(ioerr_short_read);
        CASE(ioerr_truncate);
        CASE(ioerr_unlock);
        CASE(ioerr_write);
        CASE(locked_sharedcache);
        CASE(locked_vtab);
        CASE(notive_recover_rollback);
        CASE(notice_recover_wal);
        CASE(readonly_cantlock);
        CASE(readonly_dbmoved);
        CASE(readonly_recovery);
        CASE(readonly_rollback);
        CASE(warning_autoindex);
#undef CASE
    }
    // throw sqlite3_error(ec, std::move(message), sup);
    std::cerr << "Invalid neo::sqlite3::errc value passed to neo::sqlite3::throw_error (Value is '"
              << ec.message() << "'\n";
    assert(false && "Invalid errc value passed to neo::sqlite3::throw_error");
    std::terminate();
}
