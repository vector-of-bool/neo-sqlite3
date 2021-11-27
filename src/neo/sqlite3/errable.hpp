#pragma once

#include "./errc.hpp"

#include <neo/optional.hpp>

#include <concepts>
#include <string_view>

struct sqlite3;

namespace neo::sqlite3 {

class connection_ref;

/**
 * @brief A product type that represents either a value of `T` or a SQLite error.
 */
template <typename T>
class errable;

class errable_base {
    enum errc   _rc      = neo::sqlite3::errc::ok;
    const char* _context = nullptr;
    ::sqlite3*  _dbptr   = nullptr;

public:
    constexpr errable_base(enum errc rc) noexcept
        : _rc(rc) {}
    errable_base(enum errc rc, const connection_ref& db) noexcept;
    errable_base(enum errc rc, const char* context, const connection_ref& db) noexcept;
    constexpr errable_base(enum errc rc, const char* context) noexcept
        : _rc(rc)
        , _context{context} {}

    constexpr bool is_error() const noexcept {
        using e = neo::sqlite3::errc;
        return _rc != e::ok && _rc != e::row && _rc != e::done;
    }

    constexpr explicit operator bool() const noexcept { return !is_error(); }

    constexpr enum errc errc() const noexcept { return _rc; }

    [[noreturn]] void throw_error() const;

    constexpr void throw_if_error() const {
        if (is_error()) {
            throw_error();
        }
    }

    constexpr bool operator==(enum errc ec) const noexcept { return errc() == ec; }
};

template <>
class [[nodiscard]] errable<void> : public errable_base {

public:
    using errable_base::errable_base;
    using errable_base::errc;
    using errable_base::throw_error;
};

template <typename T>
class errable : errable_base {
    // The return value:
    neo::nano_opt<T> _value;
    struct in_place_t {};

public:
    template <std::convertible_to<T> Arg>
    constexpr errable(Arg&& arg) noexcept(std::is_nothrow_convertible_v<Arg, T>)
        : errable_base(neo::sqlite3::errc::ok)
        , _value((Arg &&)(arg)) {}

    template <typename... Args>
    requires std::constructible_from<T, Args&&...> &&(sizeof...(Args) > 0)  //
        constexpr errable(enum errc rc,
                          Args&&... args)  //
        noexcept(std::is_nothrow_constructible_v<T, Args&&...>)
        : errable_base(rc)
        , _value(in_place_t{}, (Args &&)(args)...) {}

    using errable_base::errable_base;
    using errable_base::errc;
    using errable_base::throw_error;

    constexpr bool has_value() const noexcept { return _value.has_value(); }

    constexpr T* operator->() noexcept {
        if (!has_value()) {
            throw_error();
        };
        return &**this;
    }

    constexpr const T* operator->() const noexcept {
        if (!has_value()) {
            throw_error();
        };
        return &**this;
    }

#define DECL_GETTER(Name)                                                                          \
    constexpr T& Name()& {                                                                         \
        if (!has_value()) {                                                                        \
            throw_error();                                                                         \
        }                                                                                          \
        return _value.get();                                                                       \
    }                                                                                              \
    constexpr const T& Name() const& {                                                             \
        if (!has_value()) {                                                                        \
            throw_error();                                                                         \
        }                                                                                          \
        return _value.get();                                                                       \
    }                                                                                              \
    constexpr T&& Name()&& {                                                                       \
        if (!has_value()) {                                                                        \
            throw_error();                                                                         \
        }                                                                                          \
        return std::move(_value.get());                                                            \
    }                                                                                              \
    constexpr const T&& Name() const&& {                                                           \
        if (!has_value()) {                                                                        \
            throw_error();                                                                         \
        }                                                                                          \
        return std::move(_value.get());                                                            \
    }                                                                                              \
    static_assert(true)

    DECL_GETTER(value);
    DECL_GETTER(operator*);

#undef DECL_GETTER
};

}  // namespace neo::sqlite3
