#pragma once

#include "./value_ref.hpp"

#include <tuple>

namespace neo::sqlite3 {

namespace detail {

template <std::size_t I, typename...>
struct type_at;

template <typename Found, typename... Tail>
struct type_at<0, Found, Tail...> {
    using type = Found;
};

template <std::size_t I, typename Head, typename... Tail>
struct type_at<I, Head, Tail...> : type_at<I - 1, Tail...> {};

}  // namespace detail

class statement;
class value_ref;

template <typename... Ts>
class typed_row;

class row_access {
    const statement* _owner;

public:
    /// Get access to the results of the given statement.
    row_access(const statement& o) noexcept
        : _owner(&o) {}

    /**
     * @brief Obtain the value at the given index (zero-based)
     *
     * @param idx The column index. Left-most is index zero.
     */
    [[nodiscard]] value_ref operator[](int idx) const noexcept;

    /**
     * @brief Unpack the entire row into a typed tuple.
     *
     * @tparam Ts The types of the columns of the result
     */
    template <typename... Ts>
    [[nodiscard]] typed_row<Ts...> unpack() const {
        return typed_row<Ts...>{*_owner};
    }
};

template <typename... Ts>
class typed_row {
    const statement* _st;

public:
    explicit typed_row(const statement& st) noexcept
        : _st(&st) {}

    template <std::size_t Idx>
    using nth_type = detail::type_at<Idx, Ts...>::type;

    template <std::size_t Idx>
    decltype(auto) get() const {
        return static_cast<nth_type<Idx>>(row_access(*_st)[Idx].template as<nth_type<Idx>>());
    }

private:
    template <std::size_t... Is>
    auto _as_tuple(std::index_sequence<Is...>) const {
        return std::tuple(this->get<Is>()...);
    }

public:
    auto as_tuple() const { return _as_tuple(std::index_sequence_for<Ts...>{}); }
};

}  // namespace neo::sqlite3