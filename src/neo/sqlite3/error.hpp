#pragma once

#include "./errc.hpp"

#include <system_error>

namespace neo::sqlite3 {

class database_ref;

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
class error : public std::runtime_error {
    std::string _db_message;

public:
    error(std::error_code ec, std::string_view message, std::string_view db_message) noexcept;
    [[nodiscard]] virtual std::error_condition condition() const noexcept = 0;
    [[nodiscard]] virtual std::error_code      code() const noexcept      = 0;

    [[nodiscard]] const std::string& db_message() const noexcept { return _db_message; }
};

using sqlite3_error [[deprecated("Use neo::sqlite3::error instead")]] = error;

/**
 * @brief Base class template of all exceptions originating from the SQLite library
 *
 * This is an abstract class to represent the more generic form of the extended
 * error codes.
 *
 * @tparam Cond The error condition to represent
 */
template <errcond Cond>
struct errcond_error : error {
    using error::error;

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
[[noreturn]] void
throw_error(const std::error_code& ec, std::string_view message, const database_ref& db);

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

inline void
throw_if_error(const std::error_code& ec, std::string_view message, const database_ref& db) {
    if (ec) {
        throw_error(ec, message, db);
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

constexpr void do_repr(auto out, const errcond* ec) noexcept {
    out.type("neo::sqlite3::errcond");
    if (ec) {
        out.append("{value={}, message={}}",
                   int(*ec),
                   out.repr_value(make_error_condition(*ec).message()));
    }
}

constexpr void do_repr(auto out, const errc* ec) noexcept {
    out.type("neo::sqlite3::errc");
    if (ec) {
        out.append("{value={}, message={}}",
                   int(*ec),
                   out.repr_value(make_error_code(*ec).message()));
    }
}

}  // namespace neo::sqlite3

template <>
struct std::is_error_code_enum<neo::sqlite3::errc> : std::true_type {};

template <>
struct std::is_error_condition_enum<neo::sqlite3::errcond> : std::true_type {};
