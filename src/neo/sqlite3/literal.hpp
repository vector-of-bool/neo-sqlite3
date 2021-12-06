#pragma once

#include <compare>
#include <cstdint>

namespace neo::sqlite3 {

class sql_string_literal;

inline namespace literals {
constexpr sql_string_literal operator""_sql(const char*, std::size_t) noexcept;
}

/**
 * @brief Class type that represents a string literal that has been annotated
 * by the `_sql` user-defined literal.
 *
 * Because these are guaranteed to correspond to a string literal, the address
 * of the string is guaranteed to always be valid and stable for the lifetime
 * of the program.
 *
 * This is primarily used with the statement_cache class, since it uses the
 * address of the statement strings to perform cache lookups.
 */
class sql_string_literal {
    friend constexpr sql_string_literal literals::operator""_sql(const char*, std::size_t) noexcept;

    const char* _str = "[invalid]";

    sql_string_literal() = default;

public:
    /// Obtain the string of this literal
    constexpr const char* string() const noexcept { return _str; }

    friend constexpr auto operator<=>(sql_string_literal left, sql_string_literal right) noexcept {
        return left._str <=> right._str;
    }

    friend constexpr bool operator==(sql_string_literal lhs, sql_string_literal rhs) noexcept {
        /// Only equivalent if they are the same literal
        return lhs._str == rhs._str;
    }
};

inline namespace literals {

/**
 * @brief Create a new sql_string_literal from a string literal. This is the
 * only way to create sql_string_literal objects.
 */
[[nodiscard]] constexpr sql_string_literal operator""_sql(const char* ptr, std::size_t) noexcept {
    sql_string_literal ret;
    ret._str = ptr;
    return ret;
}
}  // namespace literals

}  // namespace neo::sqlite3