#pragma once

#include <neo/sqlite3/error.hpp>
#include <neo/sqlite3/statement.hpp>

#include <optional>
#include <string_view>
#include <system_error>
#include <utility>

struct sqlite3;

namespace neo::sqlite3 {

class blob;

enum class fn_flags;

/**
 * @brief A SQLite database connection.
 *
 */
class database {
    ::sqlite3* _ptr;

    database() = default;

    void _close() noexcept;

public:
    database(database&& other) noexcept
        : _ptr(std::exchange(other._ptr, nullptr)) {}

    explicit database(::sqlite3*&& ptr) noexcept
        : _ptr(std::exchange(ptr, nullptr)) {}

    database& operator=(database&& other) noexcept {
        if (_ptr) {
            _close();
        }
        _ptr = std::exchange(other._ptr, nullptr);
        return *this;
    }

    ~database() {
        if (_ptr) {
            _close();
        }
    }

    static std::optional<database> open(const std::string& s, std::error_code& ec) noexcept;
    static database                open(const std::string& s);

    static database create_memory_db() { return open(":memory:"); }

    std::optional<statement> prepare(std::string_view query, std::error_code& ec) noexcept;
    statement                prepare(std::string_view query);

    void exec(const std::string& code);

    bool         is_transaction_active() const noexcept;
    std::int64_t last_insert_rowid() const noexcept;

    blob open_blob(const std::string& table, const std::string& column, std::int64_t rowid);
    blob open_blob(const std::string& db,
                   const std::string& table,
                   const std::string& column,
                   std::int64_t       rowid);
    std::optional<blob> open_blob(const std::string& table,
                                  const std::string& column,
                                  std::int64_t       rowid,
                                  std::error_code&   ec);
    std::optional<blob> open_blob(const std::string& db,
                                  const std::string& table,
                                  const std::string& column,
                                  std::int64_t       rowid,
                                  std::error_code&   ec);

    std::string_view error_message() const noexcept;

    // To use: #include <neo/sqlite3/function.hpp>
    template <typename Func>
    void register_function(const std::string& name, Func&& fn);
    template <typename Func>
    void register_function(const std::string& name, fn_flags, Func&& fn);
};

[[nodiscard]] inline auto create_memory_db() { return database::create_memory_db(); }

[[nodiscard]] inline auto open(const std::string& path) { return database::open(path); }

[[nodiscard]] inline auto open(const std::string& path, std::error_code& ec) noexcept {
    return database::open(path, ec);
}

}  // namespace neo::sqlite3