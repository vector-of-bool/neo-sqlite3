#pragma once

#include "./fwd.hpp"

#include <neo/concepts.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

namespace neo::sqlite3 {

class statement;

/**
 * @brief Binding placeholder that constructs a zeroblob() of the given size
 */
struct zeroblob {
    std::size_t size = 0;
};

// clang-format off
template <typename T>
concept bindable =
    integral<std::remove_cvref_t<T>> ||
    std::floating_point<std::remove_cvref_t<T>> ||
    alike<T, null_t> ||
    alike<T, zeroblob> ||
    convertible_to<T, std::string_view>;
// clang-format on

/**
 * @brief Access an individual binding for a single statement parameter
 */
class binding {
    friend class binding_access;
    std::reference_wrapper<const statement> _owner;
    int                                     _index = 0;

    explicit binding(const statement& o, int idx)
        : _owner(o)
        , _index(idx) {}

    void _bind_double(double);
    void _bind_i64(std::int64_t);
    void _bind_str_nocopy(std::string_view s);
    void _bind_str_copy(std::string_view s);
    void _bind_null();
    void _bind_zeroblob(zeroblob z);

public:
    template <bindable T>
    void bind(const T& value) {
        if constexpr (std::floating_point<T>) {
            _bind_double(value);
        } else if constexpr (integral<T>) {
            _bind_i64(value);
        } else if constexpr (same_as<T, std::string_view>) {
            _bind_str_nocopy(value);
        } else if constexpr (convertible_to<T, std::string_view>) {
            _bind_str_copy(value);
        } else if constexpr (same_as<T, null_t>) {
            _bind_null();
        } else if constexpr (same_as<T, zeroblob>) {
            _bind_zeroblob(value);
        } else {
            static_assert(same_as<T, void>,
                          "This static_assertion should not fire. Please file a bug report with "
                          "neo-sqlite3!");
        }
    }

    template <typename T,
              typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, std::string_view>>>
    void bind(T v) {
        _bind_nocopy(v);
    }

    template <bindable T>
    decltype(auto) operator=(T&& t) {
        bind(t);
        return NEO_FWD(t);
    }
};

namespace detail {

template <typename T>
constexpr const T& view_if_string(const T& arg) noexcept {
    return arg;
}

inline std::string_view view_if_string(const std::string& str) noexcept { return str; }

/// Given a tuple, convert each std::string into a std::string_view
template <typename... Ts>
constexpr auto view_strings(const std::tuple<Ts...>& tup) noexcept {
    return std::apply([](const auto&... args)  //
                      { return std::forward_as_tuple(view_if_string(NEO_FWD(args))...); },
                      tup);
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
concept bindable_tuple
    = requires(Tuple&&                                                                 tup,
               std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>> Seq) {
    detail::bindable_tuple_check(tup, Seq);
};

/**
 * @brief Access to modify the bindings of a prepared statement.
 */
class binding_access {
    std::reference_wrapper<const statement> _owner;

public:
private:
    template <typename T>
    void _assign_one(int i, const T& what) {
        (*this)[i + 1] = what;
    }

    template <typename Tuple, std::size_t... Is>
    void _assign_tup(const Tuple& tup, std::index_sequence<Is...>) {
        (_assign_one(static_cast<int>(Is), std::get<Is>(tup)), ...);
    }

public:
    binding_access(const statement& o)
        : _owner(o) {}

    /**
     * @brief Access the binding at 1-based-index 'idx'
     *
     * @param idx The index of the binding (First binding is at '1', NOT zero)
     * @return binding
     */
    [[nodiscard]] binding operator[](int idx) const noexcept { return binding{_owner, idx}; }
    [[nodiscard]] binding operator[](const std::string& str) const noexcept {
        return operator[](named_parameter_index(str));
    }

    /**
     * @brief Get the index of the statement parameter with the given name
     *
     * @param name The name of the binding parameter in the prepared statement.
     * @return int The index of the parameter in the statement.
     */
    [[nodiscard]] int named_parameter_index(const std::string& name) const noexcept {
        return named_parameter_index(name.data());
    }
    [[nodiscard]] int named_parameter_index(const char* name) const noexcept;

    /**
     * @brief Reset the bound values for the prepared statement.
     */
    void clear() noexcept;

    template <bindable_tuple Tuple, std::size_t S = std::tuple_size_v<std::decay_t<Tuple>>>
    Tuple&& operator=(Tuple&& tup) {
        _assign_tup(tup, std::make_index_sequence<S>());
        return NEO_FWD(tup);
    }
};

}  // namespace neo::sqlite3