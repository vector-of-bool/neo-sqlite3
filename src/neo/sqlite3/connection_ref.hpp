#pragma once

#include "./errable_fwd.hpp"
#include "./errc.hpp"

#include <neo/zstring_view.hpp>

#include <string_view>

struct sqlite3;

namespace neo::sqlite3 {

extern "C" namespace c_api {
    int sqlite3_get_autocommit(::sqlite3*);
    int sqlite3_changes(::sqlite3*);
    int sqlite3_total_changes(::sqlite3*);
    int sqlite3_db_readonly(::sqlite3*, const char*);

    std::int64_t sqlite3_last_insert_rowid(::sqlite3*);

    const char* sqlite3_errmsg(::sqlite3*);
    const char* sqlite3_db_filename(::sqlite3*, const char*);

    void sqlite3_interrupt(::sqlite3*);
}

class statement;
class blob_io;
enum class fn_flags;

class connection_ref;

namespace event {

struct prepare_before {
    connection_ref&  db;
    std::string_view code;
};

struct prepare_after {
    connection_ref&  db;
    std::string_view code;
    statement&       stmt;
};

struct prepare_error {
    connection_ref&  db;
    std::string_view code;
    errc             ec;
};

struct exec_before {
    connection_ref&  db;
    std::string_view code;
};

struct exec_after {
    connection_ref&  db;
    std::string_view code;
    errc             ec;
};

}  // namespace event

/**
 * @brief A non-owning reference to a database connection.
 */
class connection_ref {
    connection_ref() = default;

    ::sqlite3* _ptr;

protected:
    ::sqlite3* _exchange_ptr(::sqlite3* pt) noexcept {
        auto p = _ptr;
        _ptr   = pt;
        return p;
    }

    void _assert_open() noexcept;

public:
    /// Constructing from a null pointer is illegal
    explicit connection_ref(decltype(nullptr)) = delete;

    /**
     * @brief Construct a new connection object from the given SQLite C API pointer.
     *
     * The new connection object OWNS the connection pointer, and will close it at
     * the end of the object's lifetime unless .release() is called.
     *
     * @param ptr A pointer to an open SQLite connection.
     */
    explicit connection_ref(::sqlite3* ptr) noexcept
        : _ptr(ptr) {
        /// We cannot be constructed from null
        if (!_ptr) {
            _assert_open();
        }
    }

    /// Obtain a copy of the SQLite C API pointer.
    [[nodiscard]] ::sqlite3* c_ptr() const noexcept { return _ptr; }

    /**
     * @brief Create a new prepared statement attached to this connection.
     *
     * @param query The statement code to compile.
     * @param ec An output parameter for any error information
     * @return std::optional<statement> Returns nullopt on error, otherwise a new statement object
     */
    [[nodiscard]] errable<statement> prepare(std::string_view query) noexcept;

    /**
     * @brief Execute a sequence of semicolon-separated SQL statements.
     *
     * @param code A SQL script to run.
     */
    errable<void> exec(neo::zstring_view code);

    /// Determine whether there is an active transaction on the connection
    [[nodiscard]] bool is_transaction_active() const noexcept {
        return c_api::sqlite3_get_autocommit(c_ptr()) == 0;
    }
    /// Obtain the most resent ROWID inserted by an INSERT statement.
    [[nodiscard]] std::int64_t last_insert_rowid() const noexcept {
        return c_api::sqlite3_last_insert_rowid(c_ptr());
    }
    /// Obtain the number of rows added/deleted/modified by the most recent statement.
    [[nodiscard]] int changes() const noexcept { return c_api::sqlite3_changes(c_ptr()); }
    /// Obtain the number of rows added/deleted/modified since the connection was opened.
    [[nodiscard]] int total_changes() const noexcept {
        return c_api::sqlite3_total_changes(c_ptr());
    }

    [[nodiscard]] errable<blob_io>
    open_blob(neo::zstring_view table, neo::zstring_view column, std::int64_t rowid);

    [[nodiscard]] errable<blob_io> open_blob(neo::zstring_view db,
                                             neo::zstring_view table,
                                             neo::zstring_view column,
                                             std::int64_t      rowid);

    /**
     * @brief Obtain an error message string related to the most recent error
     *
     * @return std::string_view A view of an internal error string. This view is invalidated by the
     * next connection operation!!
     */
    [[nodiscard]] neo::zstring_view error_message() const noexcept {
        return c_api::sqlite3_errmsg(c_ptr());
    }

    /**
     * @brief Determine whether the "main" database has been opened as read-only
     */
    [[nodiscard]] bool is_readonly() const noexcept { return is_readonly("main"); }

    /**
     * @brief Determine whether the named database has been opened as read-only
     */
    [[nodiscard]] bool is_readonly(neo::zstring_view name) const noexcept {
        return c_api::sqlite3_db_readonly(c_ptr(), name.data());
    }

    /**
     * @brief Obtain the filename that was used to open the "main" database
     */
    [[nodiscard]] neo::zstring_view filename() const noexcept { return filename("main"); }

    /**
     * @brief Obtain the filename that was used to open the database of the given name
     */
    [[nodiscard]] neo::zstring_view filename(neo::zstring_view name) const noexcept {
        return c_api::sqlite3_db_filename(c_ptr(), name.data());
    }

    errable<void> attach(std::string_view db_name, std::string_view db_filename_or_uri) noexcept;
    errable<void> detach(std::string_view db_name) noexcept;

    // To use: #include <neo/sqlite3/function.hpp>
    template <typename Func>
    void register_function(neo::zstring_view, Func&& fn);
    template <typename Func>
    void register_function(neo::zstring_view, fn_flags, Func&& fn);

    /**
     * @brief Interupt any currently in-progress database operation.
     *
     * This function is safe to call from any thread.
     */
    void interrupt() noexcept { c_api::sqlite3_interrupt(c_ptr()); }

    friend constexpr void do_repr(auto out, const connection_ref* self) noexcept {
        out.type("neo::sqlite3::connection");
        if (self) {
            out.bracket_value(
                "{}, filename={}, changes={}, total_changes={}, "
                "is_transaction_active={}, last_insert_rowid={}, "
                "readonly={}, error_message={}",
                out.repr_value(self->c_ptr()),
                out.repr_value(self->filename()),
                self->changes(),
                self->total_changes(),
                self->is_transaction_active(),
                self->last_insert_rowid(),
                self->is_readonly(),
                out.repr_value(self->error_message()));
        }
    }
};

}  // namespace neo::sqlite3