#include <catch2/catch.hpp>

#include "fixtures/RunOnCodeFixture.hpp"

TEST_CASE("Find function call.", "[functions]")
{
    RunOnCodeFixture fixture;

    const std::string code = "int b(); int main() { b(); }";
    const auto& operations = fixture(code);

    REQUIRE(operations.size() == 1);

    const OperationLog& log = operations.front();

    REQUIRE(log.entry_type == FunctionCall::TYPE_NAME);

    const FunctionCall* call = static_cast<const FunctionCall*>(log.entry.get());

    REQUIRE(call->function_name == "b");
    REQUIRE(call->call_result_type == "int");
}

TEST_CASE("Find stacked function calls.", "[functions]")
{
    RunOnCodeFixture fixture;

    const std::string code = "int a(int); int b(); int main() { a(b()); }";
    const auto& operations = fixture(code);

    REQUIRE(operations.size() == 2);

    const OperationLog& log = operations.front();

    REQUIRE(log.entry_type == FunctionCall::TYPE_NAME);

    const FunctionCall* call = static_cast<const FunctionCall*>(log.entry.get());

    REQUIRE(call->function_name == "a");
    REQUIRE(call->call_result_type == "int");

    const OperationLog& log_two = operations.back();

    REQUIRE(log_two.entry_type == FunctionCall::TYPE_NAME);

    const FunctionCall* call_two = static_cast<const FunctionCall*>(log_two.entry.get());

    REQUIRE(call_two->function_name == "b");
    REQUIRE(call_two->call_result_type == "int");
}
