#pragma once

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

constexpr bool is_error_rc(errc rc) noexcept {
    return rc != errc::ok && rc != errc::done && rc != errc::row;
}

}  // namespace neo::sqlite3