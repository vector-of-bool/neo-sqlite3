#include "./value_ref.hpp"

#include <neo/assert.hpp>
#include <neo/ufmt.hpp>
#include <sqlite3/sqlite3.h>

using namespace neo::sqlite3;

std::string value_ref::value_repr_string() const noexcept {
    using vt = value_type;
    switch (type()) {
    case vt::integer:
        return neo::ufmt("{}", as_integer());
    case vt::real:
        return std::to_string(as_real());
    case vt::blob:
        return "blob[...]";
    case vt::null:
        return "null";
    case vt::text:
        return neo::ufmt("\"{}\"s", as_text());
    }
    neo::unreachable();
}
