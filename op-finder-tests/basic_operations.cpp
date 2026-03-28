#include <catch2/catch.hpp>

#include "fixtures/RunOnCodeFixture.hpp"
#include "OperationLog.hpp"

TEST_CASE("Finds binary operations", "[basic_operation]") {
    auto binary_operand = GENERATE(std::string("="), std::string("+"), std::string("-"), std::string("/"),
                                   std::string("*"), std::string("<<"), std::string(">>"),
                                   std::string("^"), std::string("=="), std::string("|"), std::string("&"));
    RunOnCodeFixture fixture;

    const std::string code = "int main() { int a; (void)(a " + binary_operand + " 4); }";
    const auto& operations = fixture(code);

    REQUIRE(operations.size() == 1);

    const OperationLog& log = operations.front();

    REQUIRE(log.entry_type == BasicOperation::TYPE_NAME);

    const BasicOperation* op = static_cast<const BasicOperation*>(log.entry.get());

    REQUIRE(op->opcode == binary_operand);
    REQUIRE(op->operand_values.size() == 2);
    REQUIRE(op->operand_type == "int");
    REQUIRE(op->type_result == "int");
}

TEST_CASE("Find unary operations", "[basic_operation]") {
    auto unary_operand = GENERATE(std::string("++"), std::string("--"), std::string("~"), std::string("!"));
    RunOnCodeFixture fixture;

    const std::string code = "int main() { int a; (void)(" + unary_operand + "a); }";
    const auto& operations = fixture(code);

    REQUIRE(operations.size() == 1);

    const OperationLog& log = operations.front();

    REQUIRE(log.entry_type == BasicOperation::TYPE_NAME);

    const BasicOperation* op = static_cast<const BasicOperation*>(log.entry.get());

    REQUIRE(op->opcode == unary_operand);
    REQUIRE(op->operand_values.size() == 1);
    REQUIRE(op->operand_type == "int");
    REQUIRE(op->type_result == "int");
}

TEST_CASE("Find subscript operation", "[basic_operation]") {
    RunOnCodeFixture fixture;

    const std::string code = "int main() { int a[4]; (void)a[4]; }";
    const auto& operations = fixture(code);

    REQUIRE(operations.size() == 1);

    const OperationLog& log = operations.front();

    REQUIRE(log.entry_type == ArraySubscriptInfo::TYPE_NAME);

    const ArraySubscriptInfo* op = static_cast<const ArraySubscriptInfo*>(log.entry.get());

    REQUIRE(op->array_name == "a");
    REQUIRE(op->index_text == "4");
}

TEST_CASE("Find subscript operation reversed", "[basic_operation]") {
    RunOnCodeFixture fixture;

    const std::string code = "int main() { int a[4]; (void)4[a]; }";
    const auto& operations = fixture(code);

    REQUIRE(operations.size() == 1);

    const OperationLog& log = operations.front();

    REQUIRE(log.entry_type == ArraySubscriptInfo::TYPE_NAME);

    const ArraySubscriptInfo* op = static_cast<const ArraySubscriptInfo*>(log.entry.get());

    REQUIRE(op->array_name == "a");
    REQUIRE(op->index_text == "4");
    
}
