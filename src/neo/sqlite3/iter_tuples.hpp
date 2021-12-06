#pragma once

#include <neo/sqlite3/iter_rows.hpp>

#include <neo/iterator_facade.hpp>

namespace neo::sqlite3 {

/**
 * @brief A range over the results of a SQLite statement as a range of tuples
 *
 * @tparam Ts The unpack-types of each column in the statement result rows
 */
template <typename... Ts>
class iter_tuples {
    statement* _st = nullptr;

public:
    iter_tuples() = default;

    /**
     * @brief Create a new range-of-tuples that pull the result rows from the given
     * statement.
     *
     * @param st The statement from which we will pull results
     */
    explicit iter_tuples(statement& st)
        : _st(&st) {}

    /**
     * @brief Iterator that accesses the rows and automatically unpacks them as tuples
     */
    class iterator : public neo::iterator_facade<iterator> {
    private:
        friend class iter_tuples;

        iter_rows::iterator _it;

        explicit iterator(iter_rows::iterator it)
            : _it(it) {}

    public:
        iterator() = default;

        using difference_type = std::ptrdiff_t;
        enum { single_pass_iterator = true };

        auto dereference() const noexcept { return _it->unpack<Ts...>(); }
        void increment() { ++_it; }

        struct sentinel_type {};
        bool operator==(sentinel_type) const noexcept { return at_end(); }
        bool at_end() const noexcept { return _it.at_end(); }
    };

    /**
     * @brief Begin iterating the tuple results.
     *
     * Calling this function will execute the statement *once* to ready the first
     * result. Beware calling this multiple times.
     */
    [[nodiscard]] iterator begin() const {
        neo_assert(expects, _st != nullptr, "Called begin() on default-constructed iter_tuples<>");
        return iterator(iter_rows(*_st).begin());
    }

    /**
     * @brief Obtain an end-sentinel for the tuple iterator
     */
    [[nodiscard]] constexpr typename iterator::sentinel_type end() const noexcept { return {}; }
};

}  // namespace neo::sqlite3

#ifdef __has_include
#if __has_include(<ranges/v3/range/concepts.hpp>)
#include <ranges/v3/range/concepts.hpp>
template <typename... Ts>
constexpr inline bool ranges::v3::enable_view<neo::sqlite3::iter_tuples<Ts...>> = true;
#endif
#endif

#include <ranges>
template <typename... Ts>
constexpr inline bool std::ranges::enable_view<neo::sqlite3::iter_tuples<Ts...>> = true;
