#include "./function.hpp"

#include <neo/sqlite3/c/sqlite3.h>
#include <neo/sqlite3/database.hpp>
#include <neo/sqlite3/error.hpp>
#include <neo/sqlite3/value_ref.hpp>

#include <array>

using namespace neo::sqlite3;
using namespace std::literals;

namespace {

auto get_fn_wrapper_ptr(void* ptr_ptr) {
    return reinterpret_cast<detail::fn_wrapper_base*>(ptr_ptr);
}

void destroy_fn_wrapper_shared_ptr(void* ptr_ptr) noexcept {
    std::unique_ptr<detail::fn_wrapper_base> ptr(get_fn_wrapper_ptr(ptr_ptr));
    ptr.reset();
}

void invoke_fn_wrapper_shared_ptr(::sqlite3_context* context,
                                  int                nargs,
                                  ::sqlite3_value**  values) noexcept {
    auto  udata_ptr      = ::sqlite3_user_data(context);
    auto& fn_wrapper_ptr = *get_fn_wrapper_ptr(udata_ptr);
    fn_wrapper_ptr.invoke(reinterpret_cast<raw::sqlite3_context*>(context),
                          nargs,
                          reinterpret_cast<raw::sqlite3_value**>(values));
}

}  // namespace

void detail::fn_wrapper_base::set_result(raw::sqlite3_context* ctx_, null_t) noexcept {
    auto context = reinterpret_cast<::sqlite3_context*>(ctx_);
    ::sqlite3_result_null(context);
}

void detail::fn_wrapper_base::set_result(raw::sqlite3_context* ctx_, int n) noexcept {
    auto context = reinterpret_cast<::sqlite3_context*>(ctx_);
    ::sqlite3_result_int(context, n);
}

void detail::fn_wrapper_base::set_result(raw::sqlite3_context* ctx, std::int64_t i) noexcept {
    ::sqlite3_result_int64(reinterpret_cast<::sqlite3_context*>(ctx), i);
}

void detail::fn_wrapper_base::set_result(raw::sqlite3_context* ctx_, double d) noexcept {
    ::sqlite3_result_double(reinterpret_cast<::sqlite3_context*>(ctx_), d);
}

void detail::fn_wrapper_base::set_result(raw::sqlite3_context* ctx, std::string_view str) noexcept {
    ::sqlite3_result_text64(reinterpret_cast<::sqlite3_context*>(ctx),
                            str.data(),
                            str.size(),
                            SQLITE_TRANSIENT,
                            SQLITE_UTF8);
}

void detail::register_function(raw::sqlite3*                            db_,
                               const std::string&                       name,
                               std::unique_ptr<detail::fn_wrapper_base> wrapper,
                               std::size_t                              argc,
                               fn_flags                                 flags) {
    auto db = reinterpret_cast<::sqlite3*>(db_);

    auto rc
        = ::sqlite3_create_function_v2(db,
                                       name.data(),
                                       argc,
                                       SQLITE_UTF8
                                           | ((flags & fn_flags::allow_indirect) != fn_flags::none
                                                  ? 0
                                                  : SQLITE_DIRECTONLY)
                                           | ((flags & fn_flags::nondeterministic) != fn_flags::none
                                                  ? 0
                                                  : SQLITE_DETERMINISTIC),
                                       wrapper.get(),
                                       &invoke_fn_wrapper_shared_ptr,
                                       nullptr,
                                       nullptr,
                                       &destroy_fn_wrapper_shared_ptr);
    auto ec = to_error_code(rc);
    throw_if_error(ec,
                   "Failure while creating a scalar function '"s + name + "'"s,
                   ::sqlite3_errmsg(db));
    // Our shared_ptr is now in the care of SQLite. It will call our destroy
    // callback when needed.
    wrapper.release();
}