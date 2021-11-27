#include "./iter_rows.hpp"

#include "./statement.hpp"

using namespace neo::sqlite3;

iter_rows::iterator::iterator(statement& st)
    : _st(&st) {
    st.step().throw_if_error();
}

void iter_rows::iterator::increment() {
    neo_assert(expects, _st != nullptr, "Advanced of row-iterator with no associated statement");
    neo_assert(expects, !at_end(), "Advance of a finished row-iterator");
    _st->step().throw_if_error();
}
bool iter_rows::iterator::at_end() const noexcept { return !_st->is_busy(); }
