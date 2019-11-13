#pragma once

#include <neo/sqlite3/value_ref.hpp>

#include <tuple>

namespace neo::sqlite3 {

class statement;

class row_access {
    friend class neo::sqlite3::statement;
    statement* _owner = nullptr;
    row_access(statement& o)
        : _owner(&o) {}
    row_access(const row_access&) = delete;
    row_access& operator=(const row_access&) = delete;

    template <typename... Ts, std::size_t... Is>
    std::tuple<Ts...> _unpack(std::index_sequence<Is...>) const noexcept {
        return std::tuple<Ts...>((*this)[Is].as<Ts>()...);
    }

public:
    std::size_t size() const noexcept;

    value_ref operator[](int idx) const noexcept;

    template <typename... Ts>
    std::tuple<Ts...> unpack() const noexcept {
        return _unpack<Ts...>(std::index_sequence_for<Ts...>());
    }
};

}  // namespace neo::sqlite3