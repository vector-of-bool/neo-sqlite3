#pragma once

#include <neo/assert.hpp>

#include <optional>
#include <string_view>
#include <system_error>
#include <utility>

struct sqlite3;

namespace neo::sqlite3 {

class blob;

class statement;

enum class fn_flags;

/**
 * @brief A non-owning reference to a database connection.
 */
class database_ref {
    database_ref() = default;

    ::sqlite3* _ptr;

protected:
    ::sqlite3* _exchange_ptr(::sqlite3* pt) noexcept { return std::exchange(_ptr, pt); }

public:
    /// Constructing from a null pointer is illegal
    explicit database_ref(decltype(nullptr)) = delete;

    /**
     * @brief Construct a new database object from the given SQLite C API pointer.
     *
     * The new database object OWNS the database pointer, and will close it at
     * the end of the object's lifetime unless .release() is called.
     *
     * @param ptr A pointer to an open SQLite database.
     */
    explicit database_ref(::sqlite3* ptr) noexcept
        : _ptr(ptr) {
        /// We cannot be constructed from null
        neo_assert(expects,
                   _ptr != nullptr,
                   "neo::sqlite3::database_ref was constructed from a null pointer.");
    }

    /// Obtain a copy of the SQLite C API pointer.
    [[nodiscard]] ::sqlite3* c_ptr() const noexcept { return _ptr; }

    /**
     * @brief Create a new prepared statement attached to this database.
     *
     * @param query The statement code to compile.
     * @param ec An output parameter for any error information
     * @return std::optional<statement> Returns nullopt on error, otherwise a new statement object
     */
    [[nodiscard]] std::optional<statement> prepare(std::string_view query,
                                                   std::error_code& ec) noexcept;
    /// Throwing variant of prepare()
    [[nodiscard]] statement prepare(std::string_view query);

    /**
     * @brief Execute a sequence of semicolon-separated SQL statements.
     *
     * @param code A SQL script to run.
     */
    void exec(const std::string& code);

    /// Determine whether there is an active transaction on the database
    [[nodiscard]] bool is_transaction_active() const noexcept;
    /// Obtain the most resent ROWID inserted by an INSERT statement.
    [[nodiscard]] std::int64_t last_insert_rowid() const noexcept;
    /// Obtain the number of rows added/deleted/modified by the most recent database statement.
    [[nodiscard]] int changes() const noexcept;
    /// Obtain the number of rows added/deleted/modified since the database connection was opened.
    [[nodiscard]] int total_changes() const noexcept;

    [[nodiscard]] blob
                       open_blob(const std::string& table, const std::string& column, std::int64_t rowid);
    [[nodiscard]] blob open_blob(const std::string& db,
                                 const std::string& table,
                                 const std::string& column,
                                 std::int64_t       rowid);

    [[nodiscard]] std::optional<blob> open_blob(const std::string& table,
                                                const std::string& column,
                                                std::int64_t       rowid,
                                                std::error_code&   ec);
    [[nodiscard]] std::optional<blob> open_blob(const std::string& db,
                                                const std::string& table,
                                                const std::string& column,
                                                std::int64_t       rowid,
                                                std::error_code&   ec);

    /**
     * @brief Obtain an error message string related to the most recent error
     *
     * @return std::string_view A view of an internal error string. This view is invalidated by the
     * next database operation!!
     */
    [[nodiscard]] std::string_view error_message() const noexcept;

    // To use: #include <neo/sqlite3/function.hpp>
    template <typename Func>
    void register_function(const std::string& name, Func&& fn);
    template <typename Func>
    void register_function(const std::string& name, fn_flags, Func&& fn);

    /**
     * @brief Interupt any currently in-progress database operation.
     *
     * This function is safe to call from any thread.
     */
    void interrupt() noexcept;
};

/**
 * @brief An open database connection. Inherits all APIs from database_ref
 *
 * When the object goes out of scope, the database will be closed.
 */
class database : public database_ref {
    database() = default;

    void _close() noexcept;

public:
    /// Constructing from a null pointer is illegal
    explicit database(decltype(nullptr)) = delete;

    explicit database(::sqlite3*&& ptr) noexcept
        : database_ref(std::exchange(ptr, nullptr)) {}

    /// We are move-only
    database(database&& other) noexcept
        : database_ref(other.release()) {}

    database& operator=(database&& other) noexcept {
        if (c_ptr()) {
            _close();
        }
        _exchange_ptr(other.release());
        return *this;
    }

    ~database() {
        /// Defined inline to aide DCE when destroying moved-from objects
        if (c_ptr()) {
            _close();
        }
    }

    /// Relinquish ownership of the SQLite database object, and return the pointer.
    [[nodiscard]] ::sqlite3* release() noexcept { return _exchange_ptr(nullptr); }

    /**
     * @brief Open a new SQLite database.
     *
     * @param s The name/path to the database. Refer to ::sqlite3_open.
     * @param ec If opening failed, 'ec' will be set to the error that occurred
     * @return std::optional<database> Returns nullopt if opening failed, otherwise a new database
     */
    [[nodiscard]] static std::optional<database> open(const std::string& s,
                                                      std::error_code&   ec) noexcept;
    /// Throwing variant of open()
    [[nodiscard]] static database open(const std::string& s);

    /// Create a new in-memory database
    [[nodiscard]] static database create_memory_db() { return open(":memory:"); }
};

[[nodiscard]] inline auto create_memory_db() { return database::create_memory_db(); }

[[nodiscard]] inline auto open(const std::string& path) { return database::open(path); }

[[nodiscard]] inline auto open(const std::string& path, std::error_code& ec) noexcept {
    return database::open(path, ec);
}

}  // namespace neo::sqlite3