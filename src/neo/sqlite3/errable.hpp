#pragma once

#include "./errc.hpp"

#include <neo/fwd.hpp>
#include <neo/optional.hpp>
#include <neo/pp.hpp>

#include <concepts>

struct sqlite3;

namespace neo::sqlite3 {

class connection_ref;

class error_info {
    enum errc   _rc      = neo::sqlite3::errc::ok;
    const char* _context = nullptr;
    ::sqlite3*  _dbptr   = nullptr;

public:
    constexpr error_info(enum errc e, const char* ctx, ::sqlite3* dbptr) noexcept
        : _rc(e)
        , _context(ctx)
        , _dbptr(dbptr) {}

    constexpr enum errc   errc() const noexcept { return _rc; }
    constexpr const char* context() const noexcept { return _context; }
    constexpr ::sqlite3*  dbptr() const noexcept { return _dbptr; }
};

/**
 * @brief A product type that represents either a value of `T` or an error_info.
 */
template <typename T>
class errable;

class errable_base {
    error_info _error;

public:
    constexpr errable_base(enum errc rc) noexcept
        : _error(rc, nullptr, nullptr) {}
    errable_base(enum errc rc, const connection_ref& db) noexcept;
    errable_base(enum errc rc, const char* context, const connection_ref& db) noexcept;
    constexpr errable_base(enum errc rc, const char* context) noexcept
        : _error(rc, context, nullptr) {}

    constexpr errable_base(error_info e) noexcept
        : _error(e) {}

    constexpr bool is_error() const noexcept {
        using e = neo::sqlite3::errc;
        auto rc = _error.errc();
        return rc != e::ok && rc != e::row && rc != e::done;
    }

    constexpr explicit operator bool() const noexcept { return !is_error(); }

    constexpr enum errc errc() const noexcept { return _error.errc(); }

    [[noreturn]] void throw_error() const;

    [[nodiscard]] constexpr error_info error() const noexcept { return _error; }

    constexpr void throw_if_error() const {
        if (is_error()) {
            throw_error();
        }
    }

    constexpr bool operator==(enum errc ec) const noexcept { return errc() == ec; }
};

template <>
class [[nodiscard]] errable<void> : public errable_base {
    using errable_base::errable_base;
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
    using errable_base::error;
    using errable_base::is_error;
    using errable_base::throw_error;
    using errable_base::operator==;

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
        return static_cast<T&&>(_value.get());                                                     \
    }                                                                                              \
    constexpr const T&& Name() const&& {                                                           \
        if (!has_value()) {                                                                        \
            throw_error();                                                                         \
        }                                                                                          \
        return static_cast<const T&&>(_value.get());                                               \
    }                                                                                              \
    static_assert(true)

    DECL_GETTER(value);
    DECL_GETTER(operator*);

#undef DECL_GETTER
};

/**
 * @brief Inline check the result of an errable-valued expression. If the errable object has an
 * error-indicating result code, return that error immediately.
 *
 */
#define NEO_SQLITE3_CHECK(Errable)                                                                 \
    do {                                                                                           \
        auto _neo_sqlite3_checking_errable = (Errable);                                            \
        if (_neo_sqlite3_checking_errable.is_error()) {                                            \
            return _neo_sqlite3_checking_errable.error();                                          \
        }                                                                                          \
    } while (0)

/**
 * @brief Inline check the result of an errable-valued expression. If the errable object does not
 * have the expected result code, return that error immediately.
 */
#define NEO_SQLITE3_CHECK_RC(Errable, Expect)                                                      \
    do {                                                                                           \
        auto _neo_sqlite3_checking_errable = (Errable);                                            \
        if (_neo_sqlite3_checking_errable.errc() != (Expect)) {                                    \
            return _neo_sqlite3_checking_errable.error();                                          \
        }                                                                                          \
    } while (0)

/**
 * @brief Inline declare a variable that pulls out the result of an errable-valued expression. If
 * the errable does not have a value, returns the error immediately. Declares a variable named
 * `VarName` in the enclosed scope. Note: This macro expands to more than a single statement.
 *
 */
#define NEO_SQLITE3_AUTO(VarName, Errable)                                                         \
    auto&& NEO_CONCAT_3(_, __LINE__, _errable) = (Errable);                                        \
    do {                                                                                           \
        if (!NEO_CONCAT_3(_, __LINE__, _errable).has_value()) {                                    \
            return NEO_CONCAT_3(_, __LINE__, _errable).error();                                    \
        }                                                                                          \
    } while (0);                                                                                   \
    auto&& VarName = NEO_FWD(NEO_CONCAT_3(_, __LINE__, _errable)).value()

}  // namespace neo::sqlite3
