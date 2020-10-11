#pragma once

#include <system_error>

namespace neo::sqlite3 {

/**
 * @brief Error conditions from SQLite.
 *
 * These are error *conditions* and are therefore more generic than specific
 * error codes. Multiple error codes may belong to a single error condition,
 * and comparing an std::error_code with std::error_condition with the
 * sqlite3_cateogry() will comparing basd on that mapping.
 *
 * The official SQLite documentation explains the relationship between between
 * these error enumeration types: https://sqlite.org/rescode.html
 *
 * In this library, 'errcond' corresponds to the "primary result codes".
 */
enum class errcond : int {
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

/**
 * @brief Error codes from SQLite.
 *
 * These are error *conditions* and are therefore more generic than specific
 * error codes. Multiple error codes may belong to a single error condition,
 * and comparing an std::error_code with std::error_condition with the
 * sqlite3_cateogry() will comparing basd on that mapping.
 *
 * The official SQLite documentation explains the relationship between between
 * these error enumeration types: https://sqlite.org/rescode.html
 *
 * In this library, 'errc' corresponds to the "extended result codes".
 */
enum class errc : int {
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
[[nodiscard]] const std::error_category& error_category() noexcept;

[[nodiscard]] inline std::error_code make_error_code(errc e) noexcept {
    static auto& cat = error_category();
    return std::error_code(static_cast<int>(e), cat);
}

[[nodiscard]] inline std::error_condition make_error_condition(errcond cond) noexcept {
    static auto& cat = error_category();
    return std::error_condition(static_cast<int>(cond), cat);
}

/**
 * @brief Create a std::error_code from an integral result code value returned from a SQLite API
 *
 * @param sqlite_errc An integral result code returned by a SQLite C API
 * @return std::error_code A new error_code object of the sqlite3_category
 */
[[nodiscard]] inline std::error_code to_error_code(int sqlite_errc) noexcept {
    return make_error_code(static_cast<errc>(sqlite_errc));
}

/**
 * @brief Replace the value of 'ec' with the result of a SQLite C API.
 *
 * This is often used while calling the C API inline, ex:
 *
 *  std::error_code ec;
 *  set_error_code(ec, ::sqlite3_prepare_v2(...));
 *
 * @param ec The error code object to replace
 * @param sqlite_errc A SQLite C API result code value
 */
inline void set_error_code(std::error_code& ec, int sqlite_errc) {
    ec = to_error_code(sqlite_errc);
}

/**
 * @brief Map a SQLite extended error code (errc) to its more generic error
 * condition
 *
 * @param ec An extended error code from errc
 * @return errcond The generic condition that contains the given error code.
 */
[[nodiscard]] constexpr errcond error_code_condition(errc ec) {
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

/**
 * @brief Base class of all exceptions thrown by neo-sqlite3.
 */
class sqlite3_error : public std::system_error {
public:
    sqlite3_error(std::error_code ec, std::string_view message, std::string_view sup) noexcept;

    [[nodiscard]] virtual std::error_condition condition() const noexcept = 0;
    [[nodiscard]] virtual std::error_code      code() const noexcept      = 0;
};

/**
 * @brief Base class template of all exceptions originating from the SQLite library
 *
 * This is an abstract class to represent the more generic form of the extended
 * error codes.
 *
 * @tparam Cond The error condition to represent
 */
template <errcond Cond>
struct errcond_error : sqlite3_error {
    using sqlite3_error::sqlite3_error;

    [[nodiscard]] std::error_condition condition() const noexcept override {
        return make_error_condition(Cond);
    }
};

/**
 * @brief Concrete class template of all exceptions originating from the SQLite library.
 *
 * This class inherits from 'errcond_error<Cond>', where 'Cond' is the error
 * condition that contains the 'Code' of this class.
 *
 * @tparam Code The error code represented by this exception.
 */
template <errc Code>
struct errc_error : errcond_error<error_code_condition(Code)> {
    errc_error(std::string_view message, std::string_view sup)
        : errc_error::errcond_error(make_error_code(Code), message, sup) {}

    [[nodiscard]] std::error_code code() const noexcept override { return make_error_code(Code); }
};

/**
 * @brief Given an error code in the sqlite3_category(), throw a corresponding exception.
 *
 * The 'errc_error' exception class template can be instantiated for any errc. This function is a
 * utility that will dynamically select the correct 'errc_error' class type and throw a new instance
 * of that class.
 *
 * @param ec The error code that should be thrown
 * @param message The main message of the error
 * @param sup Supplementary details of the error
 */
[[noreturn]] void
throw_error(const std::error_code& ec, std::string_view message, std::string_view sup);

/**
 * @brief Conditionally throw an exception with 'throw_error'
 *
 * An exception will only be thrown if the given error_code is non-zero.
 */
inline void
throw_if_error(const std::error_code& ec, std::string_view message, std::string_view sup) {
    if (ec) {
        throw_error(ec, message, sup);
    }
}

/// Convenience alias for errcond_error<errcond::busy> (Database is busy and cannot be written)
using busy_error = errcond_error<errcond::busy>;
/// Convenience alias for errcond_error<errcond::constraint> (Any generic constraint violation)
using constraint_error = errcond_error<errcond::constraint>;

/// Convenience alias for errc_error representing a UNIQUE constraint violation
using constraint_unique_error = errc_error<errc::constraint_unique>;
/// Convenience alias for errc_error representing a NOT NULL constraint violation
using constraint_not_null_error = errc_error<errc::constraint_not_null>;
/// Convenience alias for errc_error representing a REFERENCES constraint violation
using constraint_foreign_key_error = errc_error<errc::constraint_foreign_key>;
/// Convenience alias for errc_error representing a CHECK constraint violation
using constraint_check_error = errc_error<errc::constraint_check>;

}  // namespace neo::sqlite3

template <>
struct std::is_error_code_enum<neo::sqlite3::errc> : std::true_type {};

template <>
struct std::is_error_condition_enum<neo::sqlite3::errcond> : std::true_type {};
