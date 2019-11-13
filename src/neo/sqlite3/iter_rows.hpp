#pragma once

#include <iterator>

#include <neo/sqlite3/statement.hpp>

namespace neo::sqlite3 {

class iter_rows {
    statement& _st;

public:
    class iterator {
        friend class iter_rows;

        statement* _st = nullptr;
        enum state : char {
            okay,
            pending_adv,
            done,
        };
        mutable state _state = state::pending_adv;

        void _adv_if_pending() const {
            if (_state == state::pending_adv) {
                auto st_status = _st->step();
                _state         = st_status == statement::more ? state::okay : state::done;
            }
        }

        struct end_iter {};
        explicit iterator(end_iter)
            : _state(state::done) {}

    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = row_access;
        using reference         = value_type&;
        using pointer           = value_type*;
        using iterator_category = std::input_iterator_tag;

        explicit iterator(statement& st)
            : _st(&st) {}

        reference operator*() const noexcept {
            _adv_if_pending();
            return _st->row;
        }
        pointer operator->() const noexcept {
            _adv_if_pending();
            return &_st->row;
        }
        iterator& operator++() noexcept {
            _adv_if_pending();
            _state = state::pending_adv;
            return *this;
        }
        // The `void` is not a typo.
        void operator++(int) noexcept {
            _adv_if_pending();
            _state = state::pending_adv;
        }

        friend bool operator==(const iterator& lhs, const iterator& rhs) {
            lhs._adv_if_pending();
            rhs._adv_if_pending();
            return lhs._state == rhs._state;
        }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) { return !(lhs == rhs); }

        bool has_pending_advance() const noexcept { return _state == state::pending_adv; }
    };

    explicit iter_rows(statement& st)
        : _st(st) {}

    iterator begin() const noexcept { return iterator(_st); }
    iterator end() const noexcept { return iterator(iterator::end_iter()); }
};

}  // namespace neo::sqlite3