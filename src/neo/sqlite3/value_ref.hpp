#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

struct sqlite3_value;

namespace neo::sqlite3 {

class row_access;

inline struct null_t {
} null;

enum class value_type {
    blob,
    real,
    integer,
    pointer,
    null,
    text,
};

class value_ref {
    sqlite3_value* _value_ptr = nullptr;

    value_ref() = default;

    template <typename T>
    struct type_tag {};

    auto _as(type_tag<int>) const noexcept { return as_integer(); }
    auto _as(type_tag<unsigned>) const noexcept { return as_integer(); }
    auto _as(type_tag<std::int64_t>) const noexcept { return as_integer(); }

    auto _as(type_tag<float>) const noexcept { return as_real(); }
    auto _as(type_tag<double>) const noexcept { return as_real(); }

    template <typename Char, typename Traits, typename Allocator>
    auto _as(type_tag<std::basic_string<Char, Traits, Allocator>>) const noexcept {
        return as_text();
    }
    template <typename Char, typename Traits>
    auto _as(type_tag<std::basic_string_view<Char, Traits>>) const noexcept {
        return as_text();
    }

    template <typename T>
    std::optional<T> _as(type_tag<std::optional<T>>) const noexcept {
        if (is_null()) {
            return std::nullopt;
        }
        return std::make_optional<T>(static_cast<T>(_as(type_tag<T>())));
    }

public:
    explicit value_ref(::sqlite3_value* ptr) noexcept
        : _value_ptr(ptr) {}

    [[nodiscard]] value_type type() const noexcept;

    [[nodiscard]] bool         is_integer() const noexcept { return type() == value_type::integer; }
    [[nodiscard]] std::int64_t as_integer() const noexcept;

    [[nodiscard]] bool   is_real() const noexcept { return type() == value_type::real; }
    [[nodiscard]] double as_real() const noexcept;

    [[nodiscard]] bool             is_text() const noexcept { return type() == value_type::text; }
    [[nodiscard]] std::string_view as_text() const noexcept;

    [[nodiscard]] bool is_null() const noexcept { return type() == value_type::null; }
    explicit           operator bool() const noexcept { return !is_null(); }

    template <typename T>
    T as() const noexcept {
        return static_cast<T>(_as(type_tag<T>()));
    }
};

}  // namespace neo::sqlite3