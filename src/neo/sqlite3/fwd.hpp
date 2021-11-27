#pragma once

namespace neo::sqlite3 {

class row_access;
template <typename... Ts>
class typed_row;

template <typename T>
class errable;

constexpr inline struct null_t {
} null;

enum class value_type;

class value_ref;
enum class fn_flags;

class blob_view;
class blob;
class statement;

class connection;
class connection_ref;

enum class errcond;
enum class errc;

class error;

template <errcond Cond>
struct errcond_error;

template <errc Code>
struct errc_error;

using database [[deprecated("neo::sqlite3::database has been renamed to neo::sqlite3::connection")]]
= connection;

using database_ref
    [[deprecated("neo::sqlite3::database_ref has been renamed to neo::sqlite3::connection_ref")]]
    = connection_ref;

}  // namespace neo::sqlite3
