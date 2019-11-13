#pragma once

#include <neo/sqlite3/iter_rows.hpp>
#include <neo/sqlite3/statement.hpp>

#include <optional>

namespace neo::sqlite3 {

template <typename... Ts>
class iter_tuples {
    statement& _st;

public:
    class iterator {
    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::tuple<Ts...>;
        using reference         = const value_type&;
        using pointer           = const value_type*;
        using iterator_category = std::input_iterator_tag;

    private:
        friend class iter_tuples;

        iter_rows::iterator               _it;
        mutable std::optional<value_type> _tup;

    public:
        explicit iterator(iter_rows::iterator it)
            : _it(it) {}

        reference operator*() const noexcept {
            _tup.emplace(_it->unpack<Ts...>());
            return *_tup;
        }

        pointer operator->() const noexcept {
            _tup.emplace(_it->unpack<Ts...>());
            return *_tup;
        }

        iterator& operator++() noexcept {
            ++_it;
            return *this;
        }
        iterator operator++(int) noexcept {
            auto copy = *this;
            ++*this;
            return copy;
        }

        friend bool operator==(const iterator& lhs, const iterator& rhs) noexcept {
            return lhs._it == rhs._it;
        }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) noexcept {
            return lhs._it != rhs._it;
        }
    };

    explicit iter_tuples(statement& st)
        : _st(st) {}

    iterator begin() const noexcept { return iterator(iter_rows(_st).begin()); }
    iterator end() const noexcept { return iterator(iter_rows(_st).end()); }
};

}  // namespace neo::sqlite3