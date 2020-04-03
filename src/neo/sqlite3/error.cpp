#include "./error.hpp"

#include <neo/sqlite3/c/sqlite3.h>

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
    if (ec.category() != sqlite3::error_category()) {
        assert(false && "neo::sqlite3::throw_error() should only be given error codes of the neo::sqlite3::error_category()");
        std::terminate();
    }
    auto e_ec = static_cast<errc>(ec.value());
    switch (e_ec) {
    case errc::abort:
        throw errc_error<errc::abort>(message, sup);
    case errc::auth:
        throw errc_error<errc::auth>(message, sup);
    case errc::busy:
        throw errc_error<errc::busy>(message, sup);
    case errc::cant_open:
        throw errc_error<errc::cant_open>(message, sup);
    case errc::constraint:
        throw errc_error<errc::constraint>(message, sup);
    case errc::corrupt:
        throw errc_error<errc::corrupt>(message, sup);
    case errc::done:
        throw errc_error<errc::done>(message, sup);
    case errc::empty:
        throw errc_error<errc::empty>(message, sup);
    case errc::error:
        throw errc_error<errc::error>(message, sup);
    case errc::format:
        throw errc_error<errc::format>(message, sup);
    case errc::full:
        throw errc_error<errc::full>(message, sup);
    case errc::internal:
        throw errc_error<errc::internal>(message, sup);
    case errc::interrupt:
        throw errc_error<errc::interrupt>(message, sup);
    case errc::ioerr:
        throw errc_error<errc::ioerr>(message, sup);
    case errc::locked:
        throw errc_error<errc::locked>(message, sup);
    case errc::mismatch:
        throw errc_error<errc::mismatch>(message, sup);
    case errc::misuse:
        throw errc_error<errc::misuse>(message, sup);
    case errc::nolfs:
        throw errc_error<errc::nolfs>(message, sup);
    case errc::no_memory:
        throw errc_error<errc::no_memory>(message, sup);
    case errc::not_a_database:
        throw errc_error<errc::not_a_database>(message, sup);
    case errc::not_found:
        throw errc_error<errc::not_found>(message, sup);
    case errc::notice:
        throw errc_error<errc::notice>(message, sup);
    case errc::ok:
        throw errc_error<errc::ok>(message, sup);
    case errc::perm:
        throw errc_error<errc::perm>(message, sup);
    case errc::protocol:
        throw errc_error<errc::protocol>(message, sup);
    case errc::range:
        throw errc_error<errc::range>(message, sup);
    case errc::readonly:
        throw errc_error<errc::readonly>(message, sup);
    case errc::row:
        throw errc_error<errc::row>(message, sup);
    case errc::schema:
        throw errc_error<errc::schema>(message, sup);
    case errc::too_big:
        throw errc_error<errc::too_big>(message, sup);
    case errc::warning:
        throw errc_error<errc::warning>(message, sup);

    case errc::abort_rollback:
        throw errc_error<errc::abort_rollback>(message, sup);
    case errc::busy_recovery:
        throw errc_error<errc::busy_recovery>(message, sup);
    case errc::busy_snapshot:
        throw errc_error<errc::busy_snapshot>(message, sup);
    case errc::cant_open_convert_path:
        throw errc_error<errc::cant_open_convert_path>(message, sup);
    case errc::cant_open_full_path:
        throw errc_error<errc::cant_open_full_path>(message, sup);
    case errc::cant_open_is_directory:
        throw errc_error<errc::cant_open_is_directory>(message, sup);
    case errc::cant_open_no_temp_directory:
        throw errc_error<errc::cant_open_no_temp_directory>(message, sup);
    case errc::constraint_check:
        throw errc_error<errc::constraint_check>(message, sup);
    case errc::constraint_commit_hook:
        throw errc_error<errc::constraint_commit_hook>(message, sup);
    case errc::constraint_foreign_key:
        throw errc_error<errc::constraint_foreign_key>(message, sup);
    case errc::constraint_function:
        throw errc_error<errc::constraint_function>(message, sup);
    case errc::constraint_not_null:
        throw errc_error<errc::constraint_not_null>(message, sup);
    case errc::constraint_primary_key:
        throw errc_error<errc::constraint_primary_key>(message, sup);
    case errc::constraint_rowid:
        throw errc_error<errc::constraint_rowid>(message, sup);
    case errc::constraint_trigger:
        throw errc_error<errc::constraint_trigger>(message, sup);
    case errc::constraint_unique:
        throw errc_error<errc::constraint_unique>(message, sup);
    case errc::constraint_vtab:
        throw errc_error<errc::constraint_vtab>(message, sup);
    case errc::corrupt_vtab:
        throw errc_error<errc::corrupt_vtab>(message, sup);
    case errc::ioerr_access:
        throw errc_error<errc::ioerr_access>(message, sup);
    case errc::ioerr_blocked:
        throw errc_error<errc::ioerr_blocked>(message, sup);
    case errc::ioerr_check_reserved_lock:
        throw errc_error<errc::ioerr_check_reserved_lock>(message, sup);
    case errc::ioerr_close:
        throw errc_error<errc::ioerr_close>(message, sup);
    case errc::ioerr_convert_path:
        throw errc_error<errc::ioerr_convert_path>(message, sup);
    case errc::ioerr_delete:
        throw errc_error<errc::ioerr_delete>(message, sup);
    case errc::ioerr_delete_noent:
        throw errc_error<errc::ioerr_delete_noent>(message, sup);
    case errc::ioerr_dir_close:
        throw errc_error<errc::ioerr_dir_close>(message, sup);
    case errc::ioerr_dir_fsync:
        throw errc_error<errc::ioerr_dir_fsync>(message, sup);
    case errc::ioerr_fstat:
        throw errc_error<errc::ioerr_fstat>(message, sup);
    case errc::ioerr_fsync:
        throw errc_error<errc::ioerr_fsync>(message, sup);
    case errc::ioerr_gettemppath:
        throw errc_error<errc::ioerr_gettemppath>(message, sup);
    case errc::ioerr_lock:
        throw errc_error<errc::ioerr_lock>(message, sup);
    case errc::ioerr_mmap:
        throw errc_error<errc::ioerr_mmap>(message, sup);
    case errc::ioerr_nomem:
        throw errc_error<errc::ioerr_nomem>(message, sup);
    case errc::ioerr_rdlock:
        throw errc_error<errc::ioerr_rdlock>(message, sup);
    case errc::ioerr_read:
        throw errc_error<errc::ioerr_read>(message, sup);
    case errc::ioerr_seek:
        throw errc_error<errc::ioerr_seek>(message, sup);
    case errc::ioerr_shmlock:
        throw errc_error<errc::ioerr_shmlock>(message, sup);
    case errc::ioerr_shmmap:
        throw errc_error<errc::ioerr_shmmap>(message, sup);
    case errc::ioerr_shmopen:
        throw errc_error<errc::ioerr_shmopen>(message, sup);
    case errc::ioerr_shmsize:
        throw errc_error<errc::ioerr_shmsize>(message, sup);
    case errc::ioerr_short_read:
        throw errc_error<errc::ioerr_short_read>(message, sup);
    case errc::ioerr_truncate:
        throw errc_error<errc::ioerr_truncate>(message, sup);
    case errc::ioerr_unlock:
        throw errc_error<errc::ioerr_unlock>(message, sup);
    case errc::ioerr_write:
        throw errc_error<errc::ioerr_write>(message, sup);
    case errc::locked_sharedcache:
        throw errc_error<errc::locked_sharedcache>(message, sup);
    case errc::locked_vtab:
        throw errc_error<errc::locked_vtab>(message, sup);
    case errc::notive_recover_rollback:
        throw errc_error<errc::notive_recover_rollback>(message, sup);
    case errc::notice_recover_wal:
        throw errc_error<errc::notice_recover_wal>(message, sup);
    case errc::readonly_cantlock:
        throw errc_error<errc::readonly_cantlock>(message, sup);
    case errc::readonly_dbmoved:
        throw errc_error<errc::readonly_dbmoved>(message, sup);
    case errc::readonly_recovery:
        throw errc_error<errc::readonly_recovery>(message, sup);
    case errc::readonly_rollback:
        throw errc_error<errc::readonly_rollback>(message, sup);
    case errc::warning_autoindex:
        throw errc_error<errc::warning_autoindex>(message, sup);
    }
    // throw sqlite3_error(ec, std::move(message), sup);
    std::cerr << "Invalid neo::sqlite3::errc value passed to neo::sqlite3::throw_error (Value is '"
              << ec.message() << "'\n";
    assert(false && "Invalid errc value passed to neo::sqlite3::throw_error");
    std::terminate();
}
