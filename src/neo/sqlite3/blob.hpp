#pragma once

#include <utility>

namespace neo::sqlite3 {

class connection;

class blob {
    friend class connection_ref;

    void* _ptr = nullptr;

    struct from_raw {};
    blob(from_raw, void* ptr)
        : _ptr(ptr) {}

public:
    ~blob();

    blob(blob&& o)
        : _ptr(std::exchange(o._ptr, nullptr)) {}
    blob& operator=(blob&& other) noexcept {
        std::swap(_ptr, other._ptr);
        return *this;
    }
};

}  // namespace neo::sqlite3