#pragma once

#include <neo/sqlite3/error.hpp>
#include <neo/sqlite3/statement.hpp>

#include <optional>
#include <string_view>
#include <system_error>
#include <utility>

namespace neo::sqlite3 {

class blob;

class database {
    void* _database_void_ptr = nullptr;

    database() = default;

public:
    ~database();
    database(database&& other) noexcept
        : _database_void_ptr(std::exchange(other._database_void_ptr, nullptr)) {}

    database& operator=(database&& other) noexcept {
        std::swap(other._database_void_ptr, _database_void_ptr);
        return *this;
    }

    static std::optional<database> open(const std::string& s, std::error_code& ec) noexcept;
    static database                open(const std::string& s) {
        std::error_code         ec;
        std::optional<database> ret = open(s, ec);
        if (ec) {
            throw_error(ec,
                        "Failed to open SQLite database [" + std::string(s) + "]", "[failed]");
        }
        return std::move(*ret);
    }

    static database create_memory_db() { return open(":memory:"); }

    std::optional<statement> prepare(std::string_view query, std::error_code& ec) noexcept;
    statement                prepare(std::string_view query) {
        std::error_code ec;
        auto            ret = prepare(query, ec);
        if (ec) {
            throw_error(ec, "Failed to prepare statement: " + std::string(query), error_message());
        }
        return std::move(*ret);
    }

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
};

[[nodiscard]] inline auto create_memory_db() { return database::create_memory_db(); }

[[nodiscard]] inline auto open(const std::string& path) { return database::open(path); }

[[nodiscard]] inline auto open(const std::string& path, std::error_code& ec) noexcept {
    return database::open(path, ec);
}

}  // namespace neo::sqlite3