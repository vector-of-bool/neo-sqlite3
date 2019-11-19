#include <neo/sqlite3/function.hpp>
#include <neo/sqlite3/single.hpp>

#include "./tests.inl"

TEST_CASE("Wrap a callable") {}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Create a simple custom function") {
    db.register_function("just_five", [] { return 5; });
    auto st      = db.prepare("SELECT just_five()");
    auto [value] = neo::sqlite3::unpack_single<int>(st);
    CHECK(value == 5);
}

TEST_CASE_METHOD(sqlite3_memory_db_fixture, "Function with arguments") {
    db.register_function("three_more_than", [](int n) { return n + 3; });
    auto st      = db.prepare("SELECT three_more_than(4)");
    auto [value] = neo::sqlite3::unpack_single<int>(st);
    CHECK(value == 7);
}

TEST_CASE("Destroys function objects") {
    int n_destroys = 0;
    struct my_fn_object {
        int& n_destroys;
        ~my_fn_object() { n_destroys++; }

        void operator()() {}
    } fn{n_destroys};

    CHECK(n_destroys == 0);
    {
        auto db = neo::sqlite3::create_memory_db();
        db.register_function("dummy", fn);
        CHECK(n_destroys == 0);
    }
    // DB shutdown destroys the object
    CHECK(n_destroys == 1);
}
