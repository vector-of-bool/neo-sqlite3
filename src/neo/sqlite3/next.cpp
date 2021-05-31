#include "./next.hpp"

#include "./statement.hpp"

using namespace neo::sqlite3;

errable<void> detail::run_step(statement& st) noexcept { return st.step(std::nothrow); }