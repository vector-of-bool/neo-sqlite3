#pragma once

#include <neo/sqlite3/iter_rows.hpp>
#include <neo/sqlite3/statement.hpp>

#include <cassert>
#include <optional>

namespace neo::sqlite3 {

template <typename... Ts>
class iter_tuples {
    statement* _st = nullptr;

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

        auto _unpack_current() noexcept { return _it->unpack<Ts...>(); }

    public:
        iterator() = default;
        explicit iterator(iter_rows::iterator it)
            : _it(it) {
            if (!it.is_end()) {
                _tup.emplace(_unpack_current());
            }
        }

        reference operator*() const noexcept { return *_tup; }

        pointer operator->() const noexcept { return &*_tup; }

        iterator& operator++() noexcept {
            ++_it;
            if (_it.is_end()) {
                _tup.reset();
            } else {
                _tup.emplace(_unpack_current());
            }
            return *this;
        }
        void operator++(int) noexcept { ++*this; }

        friend bool operator==(const iterator& lhs, const iterator& rhs) noexcept {
            return lhs._it == rhs._it;
        }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) noexcept {
            return lhs._it != rhs._it;
        }
    };

    iter_tuples() = default;
    explicit iter_tuples(statement& st)
        : _st(&st) {}

    iterator begin() const noexcept {
        assert(_st != nullptr);
        return iterator(iter_rows(*_st).begin());
    }
    iterator end() const noexcept {
        assert(_st != nullptr);
        return iterator(iter_rows(*_st).end());
    }
};

}  // namespace neo::sqlite3