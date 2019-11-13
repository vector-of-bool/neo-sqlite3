#include "./error.hpp"

#include <neo/sqlite3/c/sqlite3.h>

namespace {

class sqlite3_category : public std::error_category {
    const char* name() const noexcept override { return "neo::sqlite3"; }
    std::string message(int e) const override { return ::sqlite3_errstr(e); }
};

}  // namespace

const std::error_category& neo::sqlite3::error_category() noexcept {
    static sqlite3_category inst;
    return inst;
}