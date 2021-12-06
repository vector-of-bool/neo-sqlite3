#include "./connection.hpp"

#include <neo/event.hpp>
#include <sqlite3/sqlite3.h>

using namespace neo;
using namespace neo::sqlite3;
using std::string_view;

errable<connection> connection::open(zstring_view db_name, openmode mode) noexcept {
    neo::emit(event::open_before{db_name, mode});
    ::sqlite3* new_db = nullptr;
    auto rc = errc{::sqlite3_open_v2(db_name.data(), &new_db, static_cast<int>(mode), nullptr)};
    if (rc != errc::ok) {
        if (new_db) {
            rc = errc{::sqlite3_extended_errcode(new_db)};
            // We created a database object, but failed to open the connection. Delete it now.
            std::ignore = connection{std::move(new_db)};
        }
        neo::emit(event::open_error{db_name, rc});
        return {rc, "Failed to open SQLite connection"};
    }
    // Enabled extended result codes on our new connection
    ::sqlite3_extended_result_codes(new_db, 1);

    auto db = connection(std::move(new_db));
    neo::emit(event::open_after{db_name, db});
    return db;
}
