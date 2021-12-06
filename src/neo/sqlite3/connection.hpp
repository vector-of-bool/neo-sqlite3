#pragma once

#include "./errable.hpp"

#include "./connection_ref.hpp"

#include <neo/enum.hpp>
#include <neo/zstring_view.hpp>

#include <string_view>
#include <utility>

struct sqlite3;

namespace neo::sqlite3 {

extern "C" namespace c_api { int sqlite3_close(::sqlite3*); }

/**
 * @brief Bit flag options for opening a database connection.
 *
 * These values are taken directly from the SQLite open operations.
 */
enum class openmode : int {
    /// Open the database in readonly mode
    readonly = 0x01,
    /// Open the database in read+write mode (The default)
    readwrite = 0x02,
    /// Create the database file if it does not exists (Default true)
    create = 0x04,

    deleteonclose = 0x08,
    exclusive     = 0x10,
    autoproxy     = 0x20,
    uri           = 0x40,
    memory        = 0x80,
    main_db       = 0x100,
    temp_db       = 0x200,
    transient_db  = 0x400,
    main_journal  = 0x1000,
    subjournal    = 0x2000,
    super_journal = 0x4000,
    nomutex       = 0x8000,
    fullmutex     = 0x10000,
    sharedcache   = 0x20000,
    privatecate   = 0x40000,
    wal           = 0x80000,
    nofollow      = 0x100000,
};

NEO_DECL_ENUM_BITOPS(openmode);

namespace event {

struct open_before {
    std::string_view filename;
    openmode         mode;
};

struct open_error {
    std::string_view filename;
    errc             ec;
};

struct open_after {
    std::string_view filename;
    connection_ref&  db;
};

}  // namespace event

/**
 * @brief An open connection connection. Inherits all APIs from connection_ref
 *
 * When the object goes out of scope, the connection will be closed.
 */
class connection : public connection_ref {
    connection() = default;

    void _close() noexcept { c_api::sqlite3_close(_exchange_ptr(nullptr)); }

public:
    /// Constructing from a null pointer is illegal
    explicit connection(decltype(nullptr)) = delete;

    explicit connection(::sqlite3*&& ptr) noexcept
        : connection_ref(std::exchange(ptr, nullptr)) {}

    /// We are move-only
    connection(connection&& other) noexcept
        : connection_ref(other.release()) {}

    connection& operator=(connection&& other) noexcept {
        if (c_ptr()) {
            _close();
        }
        _exchange_ptr(other.release());
        return *this;
    }

    ~connection() {
        if (c_ptr()) {
            _close();
        }
    }

    /// Relinquish ownership of the SQLite connection object, and return the pointer.
    [[nodiscard]] ::sqlite3* release() noexcept { return _exchange_ptr(nullptr); }

    /**
     * @brief Open a new SQLite connection.
     *
     * @param s The name/path to the connection. Refer to ::sqlite3_open.
     * @param ec If opening failed, 'ec' will be set to the error that occurred
     * @return errable<connection> Returns nullopt if opening failed, otherwise a new
     * connection
     */
    [[nodiscard]] static errable<connection> open(neo::zstring_view s, openmode mode) noexcept;
    [[nodiscard]] static errable<connection> open(neo::zstring_view s) noexcept {
        return open(s, openmode::readwrite | openmode::create);
    }
    /// Create a new in-memory database
    [[nodiscard]] static errable<connection> create_memory_db() noexcept {
        return open(":memory:");
    }
    /// Create a new temporary database
    [[nodiscard]] static errable<connection> create_temporary_db() noexcept { return open(""); }
};

[[nodiscard]] inline auto create_memory_db() { return connection::create_memory_db(); }

[[nodiscard]] inline auto open(neo::zstring_view path) { return connection::open(path); }

}  // namespace neo::sqlite3
