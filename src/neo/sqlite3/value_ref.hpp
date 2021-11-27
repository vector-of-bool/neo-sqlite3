#pragma once

#include "./fwd.hpp"

#include "./blob_view.hpp"

#include <concepts>
#include <cstdint>
#include <optional>
#include <string_view>

struct sqlite3_value;

namespace neo::sqlite3 {

extern "C" namespace c_api {
    int          sqlite3_value_type(::sqlite3_value*);
    std::int64_t sqlite3_value_int64(::sqlite3_value*);
    double       sqlite3_value_double(::sqlite3_value*);

    const unsigned char* sqlite3_value_text(::sqlite3_value*);
}

class row_access;

enum class value_type {
    integer = 1,
    real    = 2,
    blob    = 4,
    null    = 5,
    text    = 3,
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

    auto _as(type_tag<blob_view>) const noexcept { return as_blob(); }

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

    [[nodiscard]] value_type type() const noexcept {
        return value_type{c_api::sqlite3_value_type(c_ptr())};
    }

    [[nodiscard]] bool         is_integer() const noexcept { return type() == value_type::integer; }
    [[nodiscard]] std::int64_t as_integer() const noexcept {
        return c_api::sqlite3_value_int64(c_ptr());
    }

    [[nodiscard]] bool   is_real() const noexcept { return type() == value_type::real; }
    [[nodiscard]] double as_real() const noexcept { return c_api::sqlite3_value_double(c_ptr()); }

    [[nodiscard]] bool             is_text() const noexcept { return type() == value_type::text; }
    [[nodiscard]] std::string_view as_text() const noexcept {
        auto uptr = c_api::sqlite3_value_text(c_ptr());
        auto ptr  = reinterpret_cast<const char*>(uptr);
        return std::string_view(ptr);
    }

    [[nodiscard]] bool      is_blob() const noexcept { return type() == value_type::blob; }
    [[nodiscard]] blob_view as_blob() const noexcept { return blob_view{c_ptr()}; }

    [[nodiscard]] bool is_null() const noexcept { return type() == value_type::null; }
    explicit           operator bool() const noexcept { return !is_null(); }

    template <typename T>
    [[nodiscard]] T as() const noexcept {
        return static_cast<T>(_as(type_tag<T>()));
    }

    std::string value_repr_string() const noexcept;

    ::sqlite3_value* c_ptr() const noexcept { return _value_ptr; }

    friend constexpr void do_repr(auto out, const value_ref* self) noexcept {
        out.type("neo::sqlite3::value_ref");
        if (self) {
            out.append("{}", self->value_repr_string());
        }
    }
};

}  // namespace neo::sqlite3
