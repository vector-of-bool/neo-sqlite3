#pragma once

namespace neo::sqlite3 {

class row_access;

constexpr inline struct null_t {
} null;

enum class value_type;

class value_ref;
enum class fn_flags;

class blob;
class statement;

class database;
class database_ref;

enum class errcond;
enum class errc;

class error;

template <errcond Cond>
struct errcond_error;

template <errc Code>
struct errc_error;

}  // namespace neo::sqlite3
