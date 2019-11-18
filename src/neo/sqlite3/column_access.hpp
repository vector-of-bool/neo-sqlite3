#pragma once

#include <string_view>

namespace neo::sqlite3 {

class statement;

class column_access {
    friend class statement;

    statement& _owner;
    explicit column_access(statement& st)
        : _owner(st) {}
    column_access(const column_access&) = delete;

    class column {
        friend class column_access;
        statement& _owner;
        int        _index = 0;

        column(statement& st, int idx)
            : _owner(st)
            , _index(idx) {}

    public:
        std::string_view name() const noexcept;
        std::string_view origin_name() const noexcept;
        std::string_view table_name() const noexcept;
        std::string_view database_name() const noexcept;
    };

public:
    column operator[](int idx) const noexcept { return column{_owner, idx}; }

    int count() const noexcept;
};

}  // namespace neo::sqlite3