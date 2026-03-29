#include <catch2/catch_test_macros.hpp>
#include "core/result.hpp"
#include <string>

using namespace mocap;

// success
TEST_CASE("Result: ok() constructs a successful result", "[result]")
{
    auto r = Result<int>::ok(42);
    REQUIRE(r.is_ok());
    REQUIRE_FALSE(r.is_err());
    REQUIRE(r.value() == 42);
}

TEST_CASE("Result: ok() with string value", "[result]")
{
    auto r = Result<std::string>::ok("hello");
    REQUIRE(r.is_ok());
    REQUIRE(r.value() == "hello");
}

// error
TEST_CASE("Result: err() constructs a failed result", "[result]")
{
    auto r = Result<int>::err("something went wrong");
    REQUIRE(r.is_err());
    REQUIRE_FALSE(r.is_ok());
    REQUIRE(r.error() == "something went wrong");
}

// map
TEST_CASE("Result: map() applies function on success", "[result]")
{
    auto r = Result<int>::ok(10).map([](int v) { return v * 2; });
    REQUIRE(r.is_ok());
    REQUIRE(r.value() == 20);
}

TEST_CASE("Result: map() propagates error unchanged", "[result]")
{
    auto r = Result<int>::err("initial error").map([](int v) { return v * 2; });
    REQUIRE(r.is_err());
    REQUIRE(r.error() == "initial error");
}

TEST_CASE("Result: map() can change the type", "[result]")
{
    auto r = Result<int>::ok(5).map([](int v) { return std::to_string(v); });
    REQUIRE(r.is_ok());
    REQUIRE(r.value() == "5");
}

// map_error
TEST_CASE("Result: map_error() transforms error message on failure", "[result]")
{
    auto r = Result<int>::err("raw error").map_error([](std::string e) { return "context: " + e; });
    REQUIRE(r.is_err());
    REQUIRE(r.error() == "context: raw error");
}

TEST_CASE("Result: map_error() leaves success unchanged", "[result]")
{
    auto r = Result<int>::ok(7).map_error([](std::string e) { return "context: " + e; });
    REQUIRE(r.is_ok());
    REQUIRE(r.value() == 7);
}

// chaining
TEST_CASE("Result: chained map calls all execute on success", "[result]")
{
    auto r = Result<int>::ok(3).map([](int v) { return v + 1; }).map([](int v) { return v * v; });
    REQUIRE(r.is_ok());
    REQUIRE(r.value() == 16);
}

TEST_CASE("Result: chain short-circuits on first error", "[result]")
{
    auto r = Result<int>::err("first failure").map([](int v) { return v + 1; }).map([](int v) { return v * v; });
    REQUIRE(r.is_err());
    REQUIRE(r.error() == "first failure");
}

// result<void> specialization
TEST_CASE("Result<void>: ok() constructs a successful void result", "[result]")
{
    auto r = Result<void>::ok();
    REQUIRE(r.is_ok());
    REQUIRE_FALSE(r.is_err());
}

TEST_CASE("Result<void>: err() constructs a failed void result", "[result]")
{
    auto r = Result<void>::err("void operation failed");
    REQUIRE(r.is_err());
    REQUIRE(r.error() == "void operation failed");
}

TEST_CASE("VoidResult alias works identically to Result<void>", "[result]")
{
    VoidResult r = VoidResult::ok();
    REQUIRE(r.is_ok());
}