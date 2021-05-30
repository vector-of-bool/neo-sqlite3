#include "./function.hpp"

#include <neo/sqlite3/database.hpp>
#include <neo/sqlite3/error.hpp>
#include <neo/sqlite3/value_ref.hpp>

#include <neo/ufmt.hpp>
#include <sqlite3/sqlite3.h>

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
    fn_wrapper_ptr.invoke(reinterpret_cast<sqlite3_context*>(context),
                          nargs,
                          reinterpret_cast<::sqlite3_value**>(values));
}

}  // namespace

void detail::fn_wrapper_base::set_result(sqlite3_context* ctx, null_t) noexcept {
    ::sqlite3_result_null(ctx);
}

void detail::fn_wrapper_base::set_result(sqlite3_context* ctx, int n) noexcept {
    ::sqlite3_result_int(ctx, n);
}

void detail::fn_wrapper_base::set_result(sqlite3_context* ctx, std::int64_t i) noexcept {
    ::sqlite3_result_int64(ctx, i);
}

void detail::fn_wrapper_base::set_result(sqlite3_context* ctx, double d) noexcept {
    ::sqlite3_result_double(ctx, d);
}

void detail::fn_wrapper_base::set_result(sqlite3_context* ctx, std::string_view str) noexcept {
    ::sqlite3_result_text64(reinterpret_cast<::sqlite3_context*>(ctx),
                            str.data(),
                            str.size(),
                            SQLITE_TRANSIENT,
                            SQLITE_UTF8);
}

void detail::register_function(::sqlite3*                               db_,
                               const std::string&                       name,
                               std::unique_ptr<detail::fn_wrapper_base> wrapper,
                               std::size_t                              argc,
                               fn_flags                                 flags) {
    auto db = reinterpret_cast<::sqlite3*>(db_);

    int flags_i = SQLITE_UTF8;
#ifdef SQLITE_DIRECTONLY
    if (!test_flags(flags, fn_flags::allow_indirect)) {
        flags_i |= SQLITE_DIRECTONLY;
    }
#endif
    if (!test_flags(flags, fn_flags::nondeterministic)) {
        flags_i |= SQLITE_DETERMINISTIC;
    }

    auto rc = ::sqlite3_create_function_v2(db,
                                           name.data(),
                                           static_cast<int>(argc),
                                           flags_i,
                                           wrapper.get(),
                                           &invoke_fn_wrapper_shared_ptr,
                                           nullptr,
                                           nullptr,
                                           &destroy_fn_wrapper_shared_ptr);
    auto ec = to_error_code(rc);
    if (ec) {
        throw_error(ec,
                    ufmt("Error while creating a scalar function '{}'", name),
                    database_ref(db));
    }
    // Our shared_ptr is now in the care of SQLite. It will call our destroy
    // callback when needed.
    wrapper.release();
}

void detail::fn_wrapper_base::invoke(sqlite3_context*  ctx,
                                     int               argc,
                                     ::sqlite3_value** argv) noexcept {
    if (argc != this->arg_count()) {
        ::sqlite3_result_error(ctx,
                               ufmt("Incorrect number of arguments passed to custom SQLite "
                                    "function (Expected {}, but got {})",
                                    this->arg_count(),
                                    argc)
                                   .data(),
                               -1);
        return;
    }
    try {
        this->do_invoke(ctx, argc, argv);
    } catch (const std::exception& e) {
        ::sqlite3_result_error(ctx, e.what(), -1);
    } catch (...) {
        ::sqlite3_result_error(
            ctx, "[neo-sqlite3]: Non-std::exception type was thrown by custom function", -1);
    }
}
