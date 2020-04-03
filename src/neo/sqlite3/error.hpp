#pragma once

#include <system_error>

namespace neo::sqlite3 {

enum class errcond {
    abort          = 4,
    auth           = 23,
    busy           = 5,
    cant_open      = 14,
    constraint     = 19,
    corrupt        = 11,
    done           = 101,
    empty          = 16,
    error          = 1,
    format         = 24,
    full           = 13,
    internal       = 2,
    interrupt      = 9,
    ioerr          = 10,
    locked         = 6,
    mismatch       = 20,
    misuse         = 21,
    nolfs          = 22,
    no_memory      = 7,
    not_a_database = 26,
    not_found      = 12,
    notice         = 27,
    ok             = 0,
    perm           = 3,
    protocol       = 15,
    range          = 25,
    readonly       = 8,
    row            = 100,
    schema         = 17,
    too_big        = 18,
    warning        = 28,
};

/// Enumeration of SQLite error codes. See the SQLite documentation for an
/// explanation of each
enum class errc {
    // The base result codes are also still extended error codes.
    abort          = 4,
    auth           = 23,
    busy           = 5,
    cant_open      = 14,
    constraint     = 19,
    corrupt        = 11,
    done           = 101,
    empty          = 16,
    error          = 1,
    format         = 24,
    full           = 13,
    internal       = 2,
    interrupt      = 9,
    ioerr          = 10,
    locked         = 6,
    mismatch       = 20,
    misuse         = 21,
    nolfs          = 22,
    no_memory      = 7,
    not_a_database = 26,
    not_found      = 12,
    notice         = 27,
    ok             = 0,
    perm           = 3,
    protocol       = 15,
    range          = 25,
    readonly       = 8,
    row            = 100,
    schema         = 17,
    too_big        = 18,
    warning        = 28,

    // The extended result codes:
    abort_rollback              = 516,
    busy_recovery               = 261,
    busy_snapshot               = 517,
    cant_open_convert_path      = 1038,
    cant_open_full_path         = 782,
    cant_open_is_directory      = 526,
    cant_open_no_temp_directory = 270,
    constraint_check            = 275,
    constraint_commit_hook      = 531,
    constraint_foreign_key      = 787,
    constraint_function         = 1043,
    constraint_not_null         = 1299,
    constraint_primary_key      = 1555,
    constraint_rowid            = 2579,
    constraint_trigger          = 1811,
    constraint_unique           = 2067,
    constraint_vtab             = 2323,
    corrupt_vtab                = 267,
    ioerr_access                = 3338,
    ioerr_blocked               = 2826,
    ioerr_check_reserved_lock   = 3594,
    ioerr_close                 = 4106,
    ioerr_convert_path          = 6666,
    ioerr_delete                = 2570,
    ioerr_delete_noent          = 5898,
    ioerr_dir_close             = 4362,
    ioerr_dir_fsync             = 1290,
    ioerr_fstat                 = 1802,
    ioerr_fsync                 = 1034,
    ioerr_gettemppath           = 6410,
    ioerr_lock                  = 3850,
    ioerr_mmap                  = 6154,
    ioerr_nomem                 = 3082,
    ioerr_rdlock                = 2314,
    ioerr_read                  = 266,
    ioerr_seek                  = 5642,
    ioerr_shmlock               = 5130,
    ioerr_shmmap                = 5386,
    ioerr_shmopen               = 4618,
    ioerr_shmsize               = 4874,
    ioerr_short_read            = 522,
    ioerr_truncate              = 1546,
    ioerr_unlock                = 2058,
    ioerr_write                 = 778,
    locked_sharedcache          = 262,
    locked_vtab                 = 518,
    notive_recover_rollback     = 539,
    notice_recover_wal          = 283,
    readonly_cantlock           = 520,
    readonly_dbmoved            = 1032,
    readonly_recovery           = 264,
    readonly_rollback           = 776,
    warning_autoindex           = 284,
};

/// Obtain an instance of the SQLite error category.
const std::error_category& error_category() noexcept;

inline std::error_code make_error_code(errc e) noexcept {
    return std::error_code(static_cast<int>(e), error_category());
}

inline std::error_condition make_error_condition(errcond cond) noexcept {
    return std::error_condition(static_cast<int>(cond), error_category());
}

inline std::error_code to_error_code(int sqlite_errc) noexcept {
    return make_error_code(static_cast<errc>(sqlite_errc));
}
inline void set_error_code(std::error_code& ec, int sqlite_errc) {
    ec = to_error_code(sqlite_errc);
}

constexpr errcond error_code_condition(errc ec) {
    switch (ec) {
    case errc::ioerr:
    case errc::ioerr_access:
    case errc::ioerr_blocked:
    case errc::ioerr_check_reserved_lock:
    case errc::ioerr_close:
    case errc::ioerr_convert_path:
    case errc::ioerr_delete:
    case errc::ioerr_delete_noent:
    case errc::ioerr_dir_close:
    case errc::ioerr_dir_fsync:
    case errc::ioerr_fstat:
    case errc::ioerr_fsync:
    case errc::ioerr_gettemppath:
    case errc::ioerr_lock:
    case errc::ioerr_mmap:
    case errc::ioerr_nomem:
    case errc::ioerr_rdlock:
    case errc::ioerr_read:
    case errc::ioerr_seek:
    case errc::ioerr_shmlock:
    case errc::ioerr_shmmap:
    case errc::ioerr_shmopen:
    case errc::ioerr_shmsize:
    case errc::ioerr_short_read:
    case errc::ioerr_truncate:
    case errc::ioerr_unlock:
    case errc::ioerr_write:
        return errcond::ioerr;
    case errc::abort:
    case errc::abort_rollback:
        return errcond::abort;
    case errc::busy:
    case errc::busy_recovery:
    case errc::busy_snapshot:
        return errcond::busy;
    case errc::cant_open:
    case errc::cant_open_convert_path:
    case errc::cant_open_full_path:
    case errc::cant_open_is_directory:
    case errc::cant_open_no_temp_directory:
        return errcond::cant_open;
    case errc::constraint:
    case errc::constraint_check:
    case errc::constraint_commit_hook:
    case errc::constraint_foreign_key:
    case errc::constraint_function:
    case errc::constraint_not_null:
    case errc::constraint_primary_key:
    case errc::constraint_rowid:
    case errc::constraint_trigger:
    case errc::constraint_unique:
    case errc::constraint_vtab:
        return errcond::constraint;
    case errc::corrupt:
    case errc::corrupt_vtab:
        return errcond::corrupt;
    case errc::locked:
    case errc::locked_vtab:
    case errc::locked_sharedcache:
        return errcond::locked;
    case errc::readonly:
    case errc::readonly_cantlock:
    case errc::readonly_dbmoved:
    case errc::readonly_recovery:
    case errc::readonly_rollback:
        return errcond::readonly;
    case errc::warning:
    case errc::warning_autoindex:
        return errcond::warning;
    case errc::notice:
    case errc::notice_recover_wal:
    case errc::notive_recover_rollback:
        return errcond::notice;

    // Error codes that have no extended versions
    case errc::auth:
        return errcond::auth;
    case errc::done:
        return errcond::done;
    case errc::empty:
        return errcond::empty;
    case errc::error:
        return errcond::error;
    case errc::format:
        return errcond::format;
    case errc::full:
        return errcond::full;
    case errc::internal:
        return errcond::internal;
    case errc::interrupt:
        return errcond::interrupt;
    case errc::mismatch:
        return errcond::mismatch;
    case errc::misuse:
        return errcond::misuse;
    case errc::nolfs:
        return errcond::nolfs;
    case errc::no_memory:
        return errcond::no_memory;
    case errc::not_a_database:
        return errcond::not_a_database;
    case errc::not_found:
        return errcond::not_found;
    case errc::ok:
        return errcond::ok;
    case errc::perm:
        return errcond::perm;
    case errc::protocol:
        return errcond::protocol;
    case errc::range:
        return errcond::range;
    case errc::row:
        return errcond::row;
    case errc::schema:
        return errcond::schema;
    case errc::too_big:
        return errcond::too_big;
    }
    return static_cast<errcond>(ec);
}

class sqlite3_error : public std::system_error {
public:
    sqlite3_error(std::error_code ec, std::string_view message, std::string_view sup)
        : system_error(ec, std::string(message) + " [" + std::string(sup) + "]") {}

    virtual std::error_condition condition() const noexcept = 0;
    virtual std::error_code      code() const noexcept      = 0;
};

template <errcond Cond>
struct errcond_error : sqlite3_error {
    using sqlite3_error::sqlite3_error;

    std::error_condition condition() const noexcept override { return make_error_condition(Cond); }
};

template <errc Code>
struct errc_error : errcond_error<error_code_condition(Code)> {
    errc_error(std::string_view message, std::string_view sup)
        : errc_error::errcond_error(make_error_code(Code), message, sup) {}

    std::error_code code() const noexcept override { return make_error_code(Code); }
};

[[noreturn]] void
throw_error(const std::error_code& ec, std::string_view message, std::string_view sup);

inline void
throw_if_error(const std::error_code& ec, std::string_view message, std::string_view sup) {
    if (ec) {
        throw_error(ec, message, sup);
    }
}

using busy_error       = errcond_error<errcond::busy>;
using constraint_error = errcond_error<errcond::constraint>;

using constraint_unique_error      = errc_error<errc::constraint_unique>;
using constraint_not_null_error    = errc_error<errc::constraint_not_null>;
using constraint_foreign_key_error = errc_error<errc::constraint_foreign_key>;
using constraint_check_error       = errc_error<errc::constraint_check>;

}  // namespace neo::sqlite3

template <>
struct std::is_error_code_enum<neo::sqlite3::errc> : std::true_type {};

template <>
struct std::is_error_condition_enum<neo::sqlite3::errcond> : std::true_type {};
