#pragma once

#include <iterator>

#include <neo/sqlite3/statement.hpp>

namespace neo::sqlite3 {

class iter_rows {
    statement& _st;

public:
    class iterator {
        friend class iter_rows;

        statement* _st   = nullptr;
        bool       _done = false;

        struct end_iter {};
        explicit iterator(end_iter)
            : _done(true) {}

    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = row_access;
        using reference         = value_type&;
        using pointer           = value_type*;
        using iterator_category = std::input_iterator_tag;

        explicit iterator(statement& st)
            : _st(&st) {}

        reference operator*() const noexcept { return _st->row; }
        pointer   operator->() const noexcept { return &_st->row; }
        iterator& operator++() noexcept {
            if (_st->step() == statement::done) {
                _done = true;
            }
            return *this;
        }
        // The `void` is not a typo.
        void operator++(int) noexcept { ++(*this); }

        [[nodiscard]] bool is_end() const noexcept { return _done; }

        friend bool operator==(const iterator& lhs, const iterator& rhs) {
            return lhs._done == rhs._done;
        }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) { return !(lhs == rhs); }
    };

    explicit iter_rows(statement& st)
        : _st(st) {}

    iterator begin() const noexcept {
        if (!_st.is_busy()) {
            if (_st.step() == statement::done) {
                return iterator(iterator::end_iter());
            }
        }
        return iterator(_st);
    }
    iterator end() const noexcept { return iterator(iterator::end_iter()); }
};

}  // namespace neo::sqlite3