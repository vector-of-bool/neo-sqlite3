#pragma once

#include "./fwd.hpp"

#include <concepts>
#include <cstdint>
#include <optional>
#include <string_view>

struct sqlite3_value;

namespace neo::sqlite3 {

class row_access;

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

    template <std::integral I>
    I _as(type_tag<I>) const noexcept {
        return static_cast<I>(as_integer());
    }

    template <std::floating_point F>
    F _as(type_tag<F>) const noexcept {
        return static_cast<F>(as_real());
    }

    bool _as(type_tag<bool>) const noexcept { return as_integer() != 0; }

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
    [[nodiscard]] T as() const noexcept {
        return static_cast<T>(_as(type_tag<T>()));
    }

    std::string value_repr_string() const noexcept;

    friend constexpr void do_repr(auto out, const value_ref* self) noexcept {
        out.type("neo::sqlite3::value_ref");
        if (self) {
            out.append("{}", self->value_repr_string());
        }
    }
};

}  // namespace neo::sqlite3