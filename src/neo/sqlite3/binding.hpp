#pragma once

#include "./fwd.hpp"

#include "./blob_view.hpp"
#include "./errable.hpp"

#include <neo/concepts.hpp>
#include <neo/fwd.hpp>
#include <neo/opt_ref_fwd.hpp>
#include <neo/text_range.hpp>
#include <neo/zstring_view.hpp>

#include <cstdint>
#include <string_view>
#include <tuple>

struct sqlite3_stmt;

namespace neo::sqlite3 {

extern "C" namespace c_api {
    int sqlite3_bind_blob(::sqlite3_stmt*, int col, const void* data, int n, void (*dtor)(void*));
    int sqlite3_bind_double(::sqlite3_stmt*, int col, double v);
    int sqlite3_bind_int64(::sqlite3_stmt*, int col, std::int64_t v);
    int sqlite3_bind_null(::sqlite3_stmt*, int col);
    int sqlite3_bind_text64(::sqlite3_stmt*,
                            int           col,
                            const char*   cstr,
                            std::uint64_t length,
                            void (*dtor)(void*),
                            unsigned char encoding);
    int sqlite3_bind_zeroblob(::sqlite3_stmt*, int col, int size);
    int sqlite3_clear_bindings(::sqlite3_stmt*);
    int sqlite3_bind_parameter_index(::sqlite3_stmt*, const char*);
}

class statement;

/**
 * @brief Binding placeholder that constructs a zeroblob() of the given size
 */
struct zeroblob {
    std::size_t size = 0;
};

namespace detail {

// clang-format off
template <typename T>
concept base_bindable =
    integral<std::remove_cvref_t<T>> ||
    std::floating_point<std::remove_cvref_t<T>> ||
    alike<T, null_t> ||
    alike<T, zeroblob> ||
    alike<T, blob_view> ||
    alike<T, const char*> ||
    neo::text_range<T>;

template <typename T>
concept detect_optional =
    std::default_initializable<T>
    && requires(T opt) {
        typename T::value_type;
        requires std::convertible_to<typename T::value_type, T>;
        { opt.has_value() } -> neo::simple_boolean;
        { *opt };
    };

template <typename T>
constexpr bool bindable_opt_ref_v = false;

template <typename T>
constexpr bool bindable_opt_ref_v<neo::opt_ref<T>> = base_bindable<T>;
// clang-format on

}  // namespace detail

// clang-format off
template <typename T>
concept bindable =
    detail::base_bindable<T>
    || (
        detail::detect_optional<std::remove_cvref_t<T>>
        && detail::base_bindable<typename std::remove_cvref_t<T>::value_type>
    )
    || detail::bindable_opt_ref_v<T>;
// clang-format on

/**
 * @brief Access an individual binding for a single statement parameter
 */
class binding {
    friend class binding_access;
    ::sqlite3_stmt* _owner;
    int             _index = 0;

    explicit binding(::sqlite3_stmt* o, int idx)
        : _owner(o)
        , _index(idx) {}

    errable<void> _make_error(errc rc, const char* message) const noexcept;
    errable<void> _maybe_make_error(errc rc, const char* message) const noexcept {
        if (is_error_rc(rc)) {
            return _make_error(rc, message);
        }
        return rc;
    }

public:
    errable<void> bind_double(double d) noexcept {
        auto rc = errc{c_api::sqlite3_bind_double(_owner, _index, d)};
        return _maybe_make_error(rc, "sqlite3_bind_double() failed");
    }

    errable<void> bind_i64(std::int64_t i) noexcept {
        auto rc = errc{c_api::sqlite3_bind_int64(_owner, _index, i)};
        return _maybe_make_error(rc, "sqlite3_bind_int64() failed");
    }

    errable<void> bind_str_nocopy(neo::text_view auto s) noexcept {
        auto rc
            = errc{c_api::sqlite3_bind_text64(_owner,
                                              _index,
                                              std::ranges::data(s),
                                              static_cast<std::uint64_t>(neo::text_range_size(s)),
                                              nullptr /* SQLITE_STATIC */,
                                              1 /* SQLITE_UFT8 */)};
        return _maybe_make_error(rc, "sqlite_bind_text() failed");
    }

    errable<void> bind_str_copy(neo::text_range auto&& s) noexcept {
        auto transient = (void (*)(void*))(-1);

        auto rc
            = errc{c_api::sqlite3_bind_text64(_owner,
                                              _index,
                                              std::ranges::data(s),
                                              static_cast<std::uint64_t>(neo::text_range_size(s)),
                                              transient /* SQLITE_TRANSIENT */,
                                              1 /* SQLITE_UFT8 */)};
        return _maybe_make_error(rc, "sqlite_bind_text() failed");
    }

    errable<void> bind_null() noexcept {
        auto rc = errc{c_api::sqlite3_bind_null(_owner, _index)};
        return _maybe_make_error(rc, "sqlite_bind_null() failed");
    }

    errable<void> bind_zeroblob(zeroblob z) noexcept {
        auto rc = errc{c_api::sqlite3_bind_zeroblob(_owner, _index, static_cast<int>(z.size))};
        return _maybe_make_error(rc, "sqlite_bind_zeroblob() failed");
    }

    errable<void> bind_blob_view(blob_view v) noexcept {
        auto transient = static_cast<void (*)(void*)>(0);
        auto rc        = errc{c_api::sqlite3_bind_blob(_owner,
                                                _index,
                                                v.data(),
                                                static_cast<int>(v.size()),
                                                transient)};
        if (is_error_rc(rc)) {
            return _make_error(rc, "sqlite3_bind_blob_failed");
        }
        return rc;
    }

    template <bindable T>
    errable<void> bind(const T& value) noexcept {
        if constexpr (std::floating_point<T>) {
            return bind_double(value);
        } else if constexpr (integral<T>) {
            return bind_i64(value);
        } else if constexpr (neo::text_view<T> || alike<T, const char*>) {
            return bind_str_nocopy(std::string_view(value));
        } else if constexpr (neo::text_range<T>) {
            return bind_str_copy(value);
        } else if constexpr (same_as<T, null_t>) {
            return bind_null();
        } else if constexpr (same_as<T, zeroblob>) {
            return bind_zeroblob(value);
        } else if constexpr (same_as<T, blob_view>) {
            return bind_blob_view(value);
        } else if constexpr (detail::detect_optional<T>) {
            if (!value.has_value()) {
                return bind_null();
            } else {
                return bind(*value);
            }
        } else if constexpr (detail::bindable_opt_ref_v<T>) {
            if (value.pointer()) {
                return bind(*value);
            } else {
                return bind_null();
            }
        } else {
            static_assert(same_as<T, void>,
                          "This static_assertion should not fire. Please file a bug report with "
                          "neo-sqlite3!");
            return errc::ok;
        }
    }

    template <bindable T>
    decltype(auto) operator=(T&& t) {
        bind(t).throw_if_error();
        return NEO_FWD(t);
    }
};

namespace detail {

template <typename T>
constexpr const T& view_if_string(const T& arg) noexcept {
    return arg;
}

inline auto view_if_string(neo::text_range auto&& str) noexcept { return neo::view_text(str); }

template <typename Tup, std::size_t... Idx>
constexpr auto view_strings_1(const Tup& tup, std::index_sequence<Idx...>) noexcept {
    return std::tuple(view_if_string(std::get<Idx>(tup))...);
}

/// Given a tuple, convert each std::string into a std::string_view
template <typename... Ts>
constexpr auto view_strings(const std::tuple<Ts...>& tup) noexcept {
    return view_strings_1(tup, std::index_sequence_for<Ts...>{});
}

template <typename Tuple, std::size_t... Is>
void bindable_tuple_check(const Tuple&, std::index_sequence<Is...>) requires(
    bindable<std::tuple_element_t<Is, Tuple>>&&...);

}  // namespace detail

/**
 * @brief CHeck that the given tuple type can be assigned to a binding_access value
 *
 * @tparam Tuple A cvr-qualified tuple type.
 */
template <typename Tuple>
concept bindable_tuple = requires {
    typename std::tuple_size<std::remove_cvref_t<Tuple>>::type;
}
&&requires(Tuple&&                                                                 tup,
           std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>> Seq) {
    detail::bindable_tuple_check(tup, Seq);
};

/**
 * @brief Access to modify the bindings of a prepared statement.
 */
class binding_access {
    ::sqlite3_stmt* _owner;

public:
private:
    template <typename T>
    void _assign_one(int i, const T& what) {
        (*this)[i + 1] = what;
    }

    template <typename Tuple, std::size_t... Is>
    errable<void> _assign_tup(const Tuple& tup, std::index_sequence<Is...>) {
        return bind_all(std::get<Is>(tup)...);
    }

    template <typename H, typename... Tail>
    errable<void> bind_next(int idx, const H& h, const Tail&... tail) noexcept {
        NEO_SQLITE3_CHECK(operator[](idx).bind(h));
        if constexpr (sizeof...(tail)) {
            return bind_next(idx + 1, tail...);
        } else {
            return errc::ok;
        }
    }

public:
    explicit binding_access(::sqlite3_stmt* o) noexcept
        : _owner(o) {}

    /**
     * @brief Access the binding at 1-based-index 'idx'
     *
     * @param idx The index of the binding (First binding is at '1', NOT zero)
     * @return binding
     */
    [[nodiscard]] binding operator[](int idx) noexcept { return binding{_owner, idx}; }
    [[nodiscard]] binding operator[](neo::zstring_view str) noexcept {
        return operator[](named_parameter_index(str));
    }

    /**
     * @brief Get the index of the statement parameter with the given name
     *
     * @param name The name of the binding parameter in the prepared statement.
     * @return int The index of the parameter in the statement.
     */
    [[nodiscard]] int named_parameter_index(neo::zstring_view name) const noexcept {
        return c_api::sqlite3_bind_parameter_index(_owner, name.data());
    }

    /**
     * @brief Reset the bound values for the prepared statement.
     */
    void clear() noexcept { c_api::sqlite3_clear_bindings(_owner); }

    template <bindable_tuple Tuple, std::size_t S = std::tuple_size_v<std::decay_t<Tuple>>>
    Tuple&& operator=(Tuple&& tup) {
        _assign_tup(tup, std::make_index_sequence<S>()).throw_if_error();
        return NEO_FWD(tup);
    }

    template <bindable... Ts>
    errable<void> bind_all(const Ts&... ts) noexcept {
        if constexpr (sizeof...(ts)) {
            return bind_next(1, ts...);
        } else {
            return errc::ok;
        }
    }

    template <bindable_tuple Tuple, std::size_t S = std::tuple_size_v<std::decay_t<Tuple>>>
    errable<void> bind_tuple(const Tuple& tup) noexcept {
        return _assign_tup(tup, std::make_index_sequence<S>());
    }
};

}  // namespace neo::sqlite3
