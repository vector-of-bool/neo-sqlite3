#pragma once

#include <neo/sqlite3/config.hpp>
#include <neo/sqlite3/error.hpp>
#include <neo/sqlite3/value_ref.hpp>

#include <neo/assert.hpp>
#include <neo/fwd.hpp>

#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>

struct sqlite3_stmt;
struct sqlite3;

namespace neo::sqlite3 {

class database;
class statement;

/**
 * @brief Binding placeholder that constructs a zeroblob() of the given size
 */
struct zeroblob {
    std::size_t size = 0;
};

/**
 * @brief Access the metadata of a statement's result columns
 *
 */
class column {
    std::reference_wrapper<const statement> _owner;
    int                                     _index = 0;

public:
    /**
     * @brief Access the column metadata for 'st' at index 'idx' (zero-based)
     *
     * @param st The database statement to inspect
     * @param idx The index of the column to access (zero-based)
     */
    column(const statement& st, int idx)
        : _owner(st)
        , _index(idx) {}

    // The name of the column
    [[nodiscard]] std::string_view name() const noexcept;

    /// The original name of the column, regardless of the AS
    NEO_SQLITE3_COLUMN_METADATA_API [[nodiscard]] std::string_view origin_name() const noexcept;

    /// The name of the table that owns the column
    NEO_SQLITE3_COLUMN_METADATA_API [[nodiscard]] std::string_view table_name() const noexcept;

    /// The name of the database that owns the column
    NEO_SQLITE3_COLUMN_METADATA_API [[nodiscard]] std::string_view database_name() const noexcept;
};

/**
 * @brief Access to the result column metadata of a SQLite statement.
 */
class column_access {
    std::reference_wrapper<const statement> _owner;

public:
    /**
     * @brief Access the metadata of the columns of the given statement
     *
     * @param st The statement to access
     */
    explicit column_access(statement& st)
        : _owner(st) {}

    /**
     * @brief Access the column at the given index
     *
     * @param idx The index of the column. Left-most column is index zero
     */
    [[nodiscard]] column operator[](int idx) const noexcept {
        neo_assert(expects, idx < count(), "Column index is out-of-range", idx, count());
        return column{_owner, idx};
    }

    [[nodiscard]] int count() const noexcept;
};

/**
 * @brief Access the result row of an in-progress SQLite statement.
 */
class row_access {
    std::reference_wrapper<const statement> _owner;

    template <typename... Ts, std::size_t... Is>
    std::tuple<Ts...> _unpack(std::index_sequence<Is...>) const {
        return std::tuple<Ts...>((*this)[Is].as<Ts>()...);
    }

public:
    /// Get access to the results of the given statement.
    row_access(const statement& o) noexcept
        : _owner(o) {}

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
    [[nodiscard]] std::tuple<Ts...> unpack() const {
        return _unpack<Ts...>(std::index_sequence_for<Ts...>());
    }
};

/**
 * @brief Access to modify the bindings of a prepared statement.
 */
class binding_access {
    std::reference_wrapper<const statement> _owner;

public:
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

        void _bind_nocopy(std::string_view s);

    public:
        void bind(double);
        void bind(std::int64_t);
        void bind(const std::string&);
        void bind(null_t);
        void bind(zeroblob);
        template <typename T,
                  typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, std::string_view>>>
        void bind(T v) {
            _bind_nocopy(v);
        }

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
        template <typename T,
                  typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, std::string_view>>>
        std::string_view operator=(T v) && {
            bind(v);
            return v;
        }
        std::string operator=(const std::string& v) && {
            bind(std::move(v));
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

    [[nodiscard]] binding operator[](int idx) const noexcept { return binding{_owner, idx}; }
    [[nodiscard]] binding operator[](const std::string& str) const noexcept {
        return operator[](named_parameter_index(str));
    }

    [[nodiscard]] int named_parameter_index(const std::string& name) const noexcept {
        return named_parameter_index(name.data());
    }
    [[nodiscard]] int named_parameter_index(const char* name) const noexcept;

    void clear() noexcept;

    template <typename Tuple, std::size_t S = std::tuple_size<std::decay_t<Tuple>>::value>
    Tuple&& operator=(Tuple&& tup) {
        _assign_tup(tup, std::make_index_sequence<S>());
        return NEO_FWD(tup);
    }
};

/**
 * @brief A prepared statement returns by database{_ref}::prepare
 *
 * When the object is destroyed, the statement will be freed.
 */
class statement {
    sqlite3_stmt* _stmt_ptr = nullptr;
    void          _destroy() noexcept;

public:
    static constexpr auto done = errc::done;
    static constexpr auto more = errc::row;

    ~statement() {
        // This is defined inline so that compilers have greater visibility to DCE this branch
        if (_stmt_ptr) {
            _destroy();
        }
    }

    explicit statement(sqlite3_stmt*&& ptr) noexcept
        : _stmt_ptr(std::exchange(ptr, nullptr)) {}

    statement(statement&& o) noexcept { _stmt_ptr = o.release(); }

    statement& operator=(statement&& o) noexcept {
        if (_stmt_ptr) {
            _destroy();
        }
        _stmt_ptr = o.release();
        return *this;
    }

    void reset() noexcept;

    [[nodiscard]] sqlite3_stmt* c_ptr() const noexcept { return _stmt_ptr; }
    [[nodiscard]] sqlite3_stmt* release() noexcept { return std::exchange(_stmt_ptr, nullptr); }

    errc               step();
    [[nodiscard]] errc step(std::error_code& ec) noexcept;

    void run_to_completion() {
        while (step() == more) {
            /* Keep going */
        }
    }

    [[nodiscard]] bool is_busy() const noexcept;

    /**
     * @brief Access to the current row of this statement.
     */
    [[nodiscard]] auto row() const noexcept { return row_access{*this}; }

    /**
     * @brief Access/modify the parameter bindings of this statement
     */
    [[nodiscard]] auto bindings() noexcept { return binding_access{*this}; }

    /**
     * @brief Access the columne metadata of this statement.
     */
    [[nodiscard]] auto columns() noexcept { return column_access{*this}; }
};

}  // namespace neo::sqlite3
