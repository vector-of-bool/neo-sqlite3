#pragma once

#include <cstddef>

struct sqlite3_value;

namespace neo::sqlite3 {

extern "C" namespace c_api {
    int         sqlite3_value_bytes(::sqlite3_value*);
    const void* sqlite3_value_blob(::sqlite3_value*);
}

/**
 * @brief View a sequence of bytes as a BLOB.
 */
class blob_view {
    std::size_t      _size;
    const std::byte* _data;

public:
    explicit blob_view(::sqlite3_value* ptr) noexcept
        : _size(c_api::sqlite3_value_bytes(ptr))
        , _data(static_cast<const std::byte*>(c_api::sqlite3_value_blob(ptr))) {}

    template <typename Data>
    explicit blob_view(Data&& data) noexcept requires requires {
        data.data();
        data.size();
    } : _size(data.size() * sizeof(decltype(*data.data()))),
        _data(reinterpret_cast<const std::byte*>(data.data())) {
    }

    [[nodiscard]] std::size_t byte_size() const noexcept { return _size; }
    [[nodiscard]] std::size_t size() const noexcept { return _size; }
    [[nodiscard]] auto        data() const noexcept { return _data; }

    [[nodiscard]] auto begin() const noexcept { return data(); }
    [[nodiscard]] auto end() const noexcept { return begin() + size(); }
};

}  // namespace neo::sqlite3