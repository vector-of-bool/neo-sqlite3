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

/**
 * @brief A result row that has associated types at compile-time.
 *
 * @tparam Ts The types of the fields of the row
 */
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

    [[nodiscard]] int column_count() const noexcept;

    friend constexpr void do_repr(auto out, row_access const* self) noexcept {
        out.type("neo::sqlite3::row_access");
        if (self) {
            out.append("{");
            auto n_cols = self->column_count();
            for (auto i = 0; i < n_cols; ++i) {
                out.append("[{}]={}", i, out.repr_value((*self)[i]));
                if (i + 1 < n_cols) {
                    out.append(", ");
                }
            }
            out.append("}");
        }
    }
};

template <typename... Ts>
class typed_row {
    row_access _row;

public:
    explicit typed_row(const statement& st) noexcept
        : _row(st) {}

    template <std::size_t Idx>
    using nth_type = detail::type_at<Idx, Ts...>::type;

    template <std::size_t Idx>
    decltype(auto) get() const {
        return static_cast<nth_type<Idx>>(_row[Idx].template as<nth_type<Idx>>());
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

template <typename... Ts>
struct std::tuple_size<neo::sqlite3::typed_row<Ts...>> {
    constexpr static std::size_t value = sizeof...(Ts);
};

template <std::size_t I, typename... Ts>
struct std::tuple_element<I, neo::sqlite3::typed_row<Ts...>> {
    using type = neo::sqlite3::typed_row<Ts...>::template nth_type<I>;
};
