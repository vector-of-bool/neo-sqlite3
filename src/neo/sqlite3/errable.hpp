#pragma once

#include "./errc.hpp"

#include <neo/optional.hpp>

#include <concepts>
#include <string_view>

struct sqlite3;

namespace neo::sqlite3 {

class database_ref;

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
    constexpr explicit errable_base(enum errc rc) noexcept
        : _rc(rc) {}
    errable_base(enum errc rc, const database_ref& db) noexcept;
    errable_base(enum errc rc, const char* context, const database_ref& db) noexcept;
    constexpr errable_base(enum errc rc, const char* context) noexcept
        : _rc(rc)
        , _context{context} {}

    constexpr enum errc errc() const noexcept { return _rc; }

    [[noreturn]] void throw_error() const;
};

inline constexpr struct errant_t {
} errant;

template <>
class [[nodiscard]] errable<void> : errable_base {
    bool _is_error = false;

public:
    constexpr errable() noexcept
        : errable_base(neo::sqlite3::errc::ok) {}

    constexpr errable(errant_t, enum errc rc) noexcept
        : errable_base(rc)
        , _is_error(true) {}

    constexpr errable(errant_t, enum errc rc, const char* context) noexcept
        : errable_base(rc, context)
        , _is_error(true) {}

    errable(errant_t, enum errc rc, const database_ref& db) noexcept
        : errable_base(rc, db)
        , _is_error(true) {}

    errable(errant_t, enum errc rc, const char* context, const database_ref& db) noexcept
        : errable_base(rc, context, db)
        , _is_error(true) {}

    using errable_base::errc;
    using errable_base::throw_error;

    constexpr explicit operator bool() const noexcept { return bool(_is_error); }

    void throw_if_error() const {
        if (_is_error) {
            throw_error();
        }
    }
};

template <typename T>
class errable : errable_base {
    // The return value:
    neo::nano_opt<T> _value;

public:
    template <std::convertible_to<T> Arg>
    constexpr errable(Arg&& arg) noexcept(noexcept(std::is_nothrow_convertible_v<Arg, T>))
        : errable_base(neo::sqlite3::errc::ok)
        , _value((Arg &&)(arg)) {}

    template <std::convertible_to<T> Arg>
    constexpr errable(enum errc           rc,
                      const database_ref& db,
                      Arg&& arg) noexcept(noexcept(std::is_nothrow_convertible_v<Arg, T>))
        : errable_base(rc)
        , _value((Arg &&)(arg)) {}

    using errable_base::errable_base;
    using errable_base::errc;
    using errable_base::throw_error;

    void throw_if_error() const {
        if (!_value.has_value()) {
            throw_error();
        }
    }

    constexpr explicit operator bool() const noexcept { return bool(_value); }

    constexpr T* operator->() noexcept {
        throw_if_error();
        return &**this;
    }

    constexpr const T* operator->() const noexcept {
        throw_if_error();
        return &**this;
    }

#define DECL_GETTER(Name, ArgList, CallList)                                                       \
    constexpr T& Name ArgList& {                                                                   \
        throw_if_error CallList;                                                                   \
        return _value.get();                                                                       \
    }                                                                                              \
    constexpr const T& Name ArgList const& {                                                       \
        throw_if_error CallList;                                                                   \
        return _value.get();                                                                       \
    }                                                                                              \
    constexpr T&& Name ArgList&& {                                                                 \
        throw_if_error CallList;                                                                   \
        return std::move(_value.get());                                                            \
    }                                                                                              \
    constexpr const T&& Name ArgList const&& {                                                     \
        throw_if_error CallList;                                                                   \
        return std::move(_value.get());                                                            \
    }                                                                                              \
    static_assert(true)

    DECL_GETTER(value, (), ());
    DECL_GETTER(operator*,(), ());
};

}  // namespace neo::sqlite3
