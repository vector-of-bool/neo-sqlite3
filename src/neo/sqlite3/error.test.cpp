#include <neo/sqlite3/error.hpp>

#include <catch2/catch.hpp>

using std::make_error_code;
using std::make_error_condition;

TEST_CASE("Map error codes to condition") {
    auto ec = make_error_code(neo::sqlite3::errc::ioerr_lock);
    CHECK_FALSE(ec == neo::sqlite3::errc::no_memory);
    CHECK(ec == neo::sqlite3::errc::ioerr_lock);
    CHECK_FALSE(ec == neo::sqlite3::errc::ioerr_nomem);
    CHECK_FALSE(ec == neo::sqlite3::errc::ioerr);
    CHECK(ec == neo::sqlite3::errcond::ioerr);
}

TEST_CASE("Throw errors and catch as conditions") {
    /// Check the bottom class:
    CHECK_THROWS_AS(neo::sqlite3::throw_error(make_error_code(neo::sqlite3::errc::busy_snapshot),
                                              "Oh no!",
                                              "lol"),
                    neo::sqlite3::errc_error<neo::sqlite3::errc::busy_snapshot>);

    try {
        neo::sqlite3::throw_error(make_error_code(neo::sqlite3::errc::busy_snapshot),
                                  "Oh no!",
                                  "lol");
    } catch (const neo::sqlite3::errc_error<neo::sqlite3::errc::busy>&) {
        FAIL_CHECK("Should not have caught with this type");
    } catch (const neo::sqlite3::errc_error<neo::sqlite3::errc::busy_snapshot>&) {
        // Okay!
    } catch (...) {
        FAIL_CHECK("Didn't catch expected type");
    }

    /// An errc_error inherits from the errcond_error of the code's default condition
    CHECK_THROWS_AS(neo::sqlite3::throw_error(make_error_code(neo::sqlite3::errc::busy_snapshot),
                                              "Oh no!",
                                              "lol"),
                    neo::sqlite3::errcond_error<neo::sqlite3::errcond::busy>);

    /// busy_error is an alias for the neo::sqlite3::errcond::busy exception base
    CHECK_THROWS_AS(neo::sqlite3::throw_error(make_error_code(neo::sqlite3::errc::busy_snapshot),
                                              "Oh no!",
                                              "lol"),
                    neo::sqlite3::busy_error);

    // Throwing the errc for an errcond will also inherit from the corresponding errcond_error
    CHECK_THROWS_AS(neo::sqlite3::throw_error(make_error_code(neo::sqlite3::errc::busy),
                                              "Oh no!",
                                              "lol"),
                    neo::sqlite3::errc_error<neo::sqlite3::errc::busy>);
    CHECK_THROWS_AS(neo::sqlite3::throw_error(make_error_code(neo::sqlite3::errc::busy),
                                              "Oh no!",
                                              "lol"),
                    neo::sqlite3::errcond_error<neo::sqlite3::errcond::busy>);
}
