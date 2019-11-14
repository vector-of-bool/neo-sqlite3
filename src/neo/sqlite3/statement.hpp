#pragma once

#include <neo/sqlite3/column_access.hpp>
#include <neo/sqlite3/error.hpp>
#include <neo/sqlite3/row_access.hpp>

#include <optional>
#include <tuple>

namespace neo::sqlite3 {

class database;
class statement;

struct zeroblob {
    std::size_t size = 0;

    zeroblob() = default;
    zeroblob(std::size_t s)
        : size(s) {}
};

class binding_access {
    friend class neo::sqlite3::statement;
    statement& _owner;
    binding_access(statement& o)
        : _owner(o) {}
    binding_access(const binding_access&) = delete;

    class binding {
        friend class binding_access;
        statement& _owner;
        int        _index = 0;

        explicit binding(statement& o, int idx)
            : _owner(o)
            , _index(idx) {}

    public:
        void bind(double);
        void bind(std::int64_t);
        void bind(std::string_view);
        void bind(null_t);
        void bind(zeroblob);

        double operator=(double v) && {
            bind(v);
            return v;
        }
        std::int16_t operator=(std::int16_t v) && {
            bind(std::int64_t(v));
            return v;
        }
        std::uint16_t operator=(std::uint16_t v) && {
            bind(std::int64_t(v));
            return v;
        }
        std::int32_t operator=(std::int32_t v) && {
            bind(std::int64_t(v));
            return v;
        }
        std::uint32_t operator=(std::uint32_t v) && {
            bind(std::int64_t(v));
            return v;
        }
        std::int64_t operator=(std::int64_t v) && {
            bind(v);
            return v;
        }
        std::string_view operator=(std::string_view v) && {
            bind(v);
            return v;
        }
        null_t operator=(null_t) && {
            bind(null);
            return null;
        }
        zeroblob operator=(zeroblob zb) && {
            bind(zb);
            return zb;
        }
    };

    template <typename T>
    void _assign_one(std::size_t i, const T& what) {
        (*this)[i + 1] = what;
    }

    template <typename Tuple, std::size_t... Is>
    void _assign_tup(const Tuple& tup, std::index_sequence<Is...>) {
        (_assign_one(Is, std::get<Is>(tup)), ...);
    }

public:
    binding operator[](int idx) const noexcept { return binding{_owner, idx}; }
    binding operator[](const std::string& str) const noexcept {
        return operator[](named_parameter_index(str));
    }

    int named_parameter_index(const std::string& name) const noexcept {
        return named_parameter_index(name.data());
    }
    int named_parameter_index(const char* name) const noexcept;

    void clear() noexcept;

    template <typename Tuple, std::size_t = std::tuple_size<Tuple>::value>
    Tuple operator=(const Tuple& tup) {
        _assign_tup(tup, std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }
};

class statement {
    friend class database;
    friend class row_access;
    friend class column_access;
    friend class binding_access;
    friend class binding_access::binding;

    void* _stmt_ptr = nullptr;
    void  _destroy() noexcept;

    statement() = default;

public:
    enum class state {
        done,
        more,
        error,
    };

    static constexpr auto done = state::done;
    static constexpr auto more = state::more;

    ~statement() {
        if (_stmt_ptr) {
            _destroy();
        }
    }

    statement(statement&& o) noexcept { _stmt_ptr = std::exchange(o._stmt_ptr, nullptr); }

    statement& operator=(statement&& o) noexcept {
        std::swap(o._stmt_ptr, _stmt_ptr);
        return *this;
    }

    void reset() noexcept;

    [[nodiscard]] state step();
    [[nodiscard]] state step(std::error_code& ec) noexcept;

    void run_to_completion() {
        while (step() == more) {
            /* Keep going */
        }
    }

    [[nodiscard]] bool is_busy() const noexcept;

    row_access     row{*this};
    binding_access bindings{*this};
    column_access  columns{*this};
};

}  // namespace neo::sqlite3