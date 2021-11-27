#pragma once

#include "./errable.hpp"

#include <ranges>
#include <utility>

struct sqlite3_blob;

namespace neo::sqlite3 {

extern "C" namespace c_api {
    int sqlite3_blob_close(::sqlite3_blob*);
    int sqlite3_blob_bytes(::sqlite3_blob*);
    int sqlite3_blob_read(::sqlite3_blob*, void* Z, int N, int iOffset);
    int sqlite3_blob_write(::sqlite3_blob*, const void* z, int n, int iOffset);
    int sqlite3_blob_reopen(::sqlite3_blob*, std::int64_t rowid);
}

class blob_io {
    sqlite3_blob* _ptr = nullptr;

public:
    ~blob_io() { c_api::sqlite3_blob_close(release()); }

    explicit blob_io(sqlite3_blob*&& ptr) noexcept
        : _ptr(std::exchange(ptr, nullptr)) {}

    blob_io(blob_io&& o) noexcept
        : _ptr(o.release()) {}

    blob_io& operator=(blob_io&& other) noexcept {
        if (_ptr) {
            c_api::sqlite3_blob_close(release());
        }
        _ptr = other.release();
        return *this;
    }

    [[nodiscard]] sqlite3_blob* c_ptr() const noexcept { return _ptr; }
    [[nodiscard]] sqlite3_blob* release() noexcept { return std::exchange(_ptr, nullptr); }

    [[nodiscard]] std::size_t byte_size() const noexcept {
        return static_cast<std::size_t>(c_api::sqlite3_blob_bytes(c_ptr()));
    }

    template <typename T>
    requires std::ranges::contiguous_range<T>  //
        && std::ranges::output_range<T, std::ranges::range_value_t<T>> && std::
            is_trivially_copyable_v<std::ranges::range_value_t<T>>
    [[nodiscard]] errable<void> read_into(std::size_t offset, T&& buf) const noexcept {
        const auto out_byte_size = sizeof(std::ranges::range_value_t<T>) * std::ranges::size(buf);
        auto       rc            = errc{c_api::sqlite3_blob_read(c_ptr(),
                                                std::ranges::data(out_byte_size),
                                                out_byte_size,
                                                static_cast<int>(offset))};
        if (rc != errc::ok) {
            return {rc, "Failed to read from BLOB"};
        }
        return errc::ok;
    }

    template <typename T>
    requires std::ranges::contiguous_range<T>  //
        && std::is_trivially_copyable_v<std::ranges::range_value_t<T>>
    [[nodiscard]] errable<void> write(std::size_t offset, T&& data) noexcept {
        const auto data_byte_size = sizeof(std::ranges::range_value_t<T>) * std::ranges::size(data);
        auto       rc             = errc{c_api::sqlite3_blob_write(c_ptr(),
                                                 std::ranges::data(data),
                                                 data_byte_size,
                                                 static_cast<int>(offset))};
        if (rc != errc::ok) {
            return {rc, "Failed to write to BLOB"};
        }
        return errc::ok;
    }

    errable<void> reopen(std::int64_t rowid) noexcept {
        auto rc = errc{c_api::sqlite3_blob_reopen(c_ptr(), rowid)};
        if (rc != errc::ok) {
            return {rc, "Failed to reopen BLOB handle"};
        }
        return errc::ok;
    }
};

}  // namespace neo::sqlite3
