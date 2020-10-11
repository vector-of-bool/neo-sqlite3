#pragma once

#include <neo/sqlite3/database.hpp>
#include <neo/sqlite3/value_ref.hpp>

#include <neo/assert.hpp>
#include <neo/enum.hpp>
#include <neo/fwd.hpp>

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

struct sqlite3_context;

namespace neo::sqlite3 {

namespace detail {

template <typename... Args>
struct argtypes_tag {
    constexpr static std::size_t count = sizeof...(Args);
};

template <typename... Args>
struct is_argtypes {
    using type = argtypes_tag<std::decay_t<Args>...>;
};

template <typename Func>
struct infer_argtypes {
    using type = void;
};

template <typename Owner, typename Ret, typename... Args>
struct infer_argtypes<Ret (Owner::*)(Args...)> : is_argtypes<Args...> {};

template <typename Owner, typename Ret, typename... Args>
struct infer_argtypes<Ret (Owner::*)(Args...) const> : is_argtypes<Args...> {};

template <typename Owner, typename Ret, typename... Args>
struct infer_argtypes<Ret (Owner::*)(Args...) noexcept> : is_argtypes<Args...> {};

template <typename Owner, typename Ret, typename... Args>
struct infer_argtypes<Ret (Owner::*)(Args...) const noexcept> : is_argtypes<Args...> {};

template <typename Ret, typename... Args>
struct infer_argtypes<Ret (*)(Args...)> : is_argtypes<Args...> {};

template <typename Func>
requires requires {
    &std::decay_t<Func>::operator();
}
struct infer_argtypes<Func> {
    using type = typename infer_argtypes<decltype(&std::decay_t<Func>::operator())>::type;
};

class fn_wrapper_base {
public:
    void invoke(sqlite3_context* ctx, int argc, sqlite3_value** argv) noexcept;

    virtual ~fn_wrapper_base() = default;

protected:
    virtual void do_invoke(sqlite3_context* ctx, int argc, sqlite3_value** argv) = 0;
    virtual int  arg_count() const noexcept                                      = 0;

    void set_result(sqlite3_context* ctx, null_t) noexcept;
    void set_result(sqlite3_context* ctx, int) noexcept;
    void set_result(sqlite3_context* ctx, std::int64_t) noexcept;
    void set_result(sqlite3_context* ctx, double) noexcept;
    void set_result(sqlite3_context* ctx, std::string_view) noexcept;
};

template <typename Func, typename ArgTypesTag>
class fn_wrapper;

template <typename Func, typename... ArgTypes>
class fn_wrapper<Func, argtypes_tag<ArgTypes...>> : public fn_wrapper_base {
public:
    template <typename FuncArg>
    fn_wrapper(FuncArg&& fn)
        : _fn(NEO_FWD(fn)) {}

private:
    Func _fn;

    template <typename T>
    T _get_arg(sqlite3_value* ptr) {
        return value_ref(ptr).as<T>();
    }

    template <std::size_t... Is>
    std::tuple<ArgTypes...> _get_args(sqlite3_value** argv, std::index_sequence<Is...>) {
        return std::tuple<ArgTypes...>(_get_arg<ArgTypes>(argv[Is])...);
    }

    void do_invoke(sqlite3_context* ctx, int argc, sqlite3_value** argv) override {
        neo_assert(invariant,
                   argc == arg_count(),
                   "Incorrect number of arguments passed through to custom function",
                   argc,
                   this->arg_count());
        // Unpack the SQLite arguments into a tuple
        auto args_tup = _get_args(argv, std::index_sequence_for<ArgTypes...>());
        // Do the call
        using result_type = std::invoke_result_t<Func, ArgTypes...>;
        if constexpr (std::is_void_v<result_type>) {
            std::apply(_fn, args_tup);
            set_result(ctx, null);
        } else {
            auto result = std::apply(_fn, args_tup);
            set_result(ctx, result);
        }
    }

    int arg_count() const noexcept override { return sizeof...(ArgTypes); }
};

void register_function(::sqlite3*                       db,
                       const std::string&               name,
                       std::unique_ptr<fn_wrapper_base> ptr,
                       std::size_t                      argc,
                       fn_flags                         flags);

}  // namespace detail

enum class fn_flags {
    none             = 0,
    nondeterministic = 0b0000'0001,
    allow_indirect   = 0b0000'0010,
};

NEO_DECL_ENUM_BITOPS(fn_flags);

template <typename Func>
void database_ref::register_function(const std::string& name, Func&& fn) {
    register_function(name, fn_flags::none, NEO_FWD(fn));
}

template <typename Func>
void database_ref::register_function(const std::string& name, fn_flags flags, Func&& fn) {
    using argtypes = typename detail::infer_argtypes<Func>::type;
    static_assert(!std::is_void_v<argtypes>,
                  "Unable to infer the argument types of the function object. Did you pass a "
                  "callable object? Argument type detection can fail if you passed a callable "
                  "object with a generic/templated call operator (including as a closure from a "
                  "generic lambda expression).");
    // Generate the wrapper, and register
    auto wrapper = std::make_unique<detail::fn_wrapper<std::decay_t<Func>, argtypes>>(NEO_FWD(fn));
    detail::register_function(_ptr, name, std::move(wrapper), argtypes::count, flags);
}

}  // namespace neo::sqlite3
