#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

namespace neo::sqlite3 {

class sql_string_literal;

inline namespace literals {
constexpr sql_string_literal operator""_sql(const char*, std::size_t) noexcept;
}

class sql_string_literal {
    friend constexpr sql_string_literal literals::operator""_sql(const char*, std::size_t) noexcept;

    std::string_view _str = "[invalid]";

public:
    constexpr std::string_view string() const noexcept { return _str; }

    friend constexpr bool operator<(sql_string_literal lhs, sql_string_literal rhs) noexcept {
        return std::less<>()(lhs._str.data(), rhs._str.data());
    }

    friend constexpr bool operator==(sql_string_literal lhs, sql_string_literal rhs) noexcept {
        return std::equal_to<>()(lhs._str.data(), rhs._str.data());
    }

    friend constexpr bool operator!=(sql_string_literal       lhs,
                                     const sql_string_literal rhs) noexcept {
        return !(lhs == rhs);
    }
};

inline namespace literals {

constexpr sql_string_literal operator""_sql(const char* ptr, std::size_t len) noexcept {
    sql_string_literal ret;
    ret._str = std::string_view(ptr, len);
    return ret;
}
}  // namespace literals

}  // namespace neo::sqlite3