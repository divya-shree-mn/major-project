#include <catch2/catch.hpp>

#include "fixtures/RunOnCodeFixture.hpp"

TEST_CASE("For loop without header.", "[branches][for_loops]")
{
  RunOnCodeFixture fixture;

  const std::string code = "int main() { int a; for (;;) {} }";
  REQUIRE(fixture.runCode(code));

  REQUIRE(fixture.storage.getOperations().empty());
}

TEST_CASE("For loop with init only.", "[branches][for_loops]")
{
  RunOnCodeFixture fixture;

  const std::string code = "int main() { int a; for (a = 4;;) {} }";
  const auto& operations = fixture(code);

  REQUIRE(operations.size() == 1);

  const OperationLog& log = operations.front();

  REQUIRE(log.branch_number == 0);
}


TEST_CASE("For loop with init & cond.", "[branches][for_loops]")
{
  RunOnCodeFixture fixture;

  const std::string code = "int main() { int a; for (a = 4; a < 4;) {} }";
  const auto& operations = fixture(code);

  REQUIRE(operations.size() == 2);

  const OperationLog& log_init = operations.at(0);
  REQUIRE(log_init.branch_number == 1);

  const OperationLog& log_cond = operations.at(1);
  REQUIRE(log_cond.branch_number == 2);
}

TEST_CASE("For loop with init & inc.", "[branches][for_loops]")
{
  RunOnCodeFixture fixture;

  const std::string code = "int main() { int a; for (a = 4;;a++) {} }";
  const auto& operations = fixture(code);

  REQUIRE(operations.size() == 2);

  const OperationLog& log_init = operations.at(0);
  REQUIRE(log_init.branch_number == 1);

  const OperationLog& log_inc = operations.at(1);
  REQUIRE(log_inc.branch_number == 2);
}

TEST_CASE("For loop with full header.", "[branches][for_loops]")
{
  RunOnCodeFixture fixture;

  const std::string code = "int main() { int a; for (a = 4; a < 4 ;a++) {} }";
  const auto& operations = fixture(code);

  REQUIRE(operations.size() == 3);

  const OperationLog& log_init = operations.at(0);
  REQUIRE(log_init.branch_number == 1);

  const OperationLog& log_cond = operations.at(1);
  REQUIRE(log_cond.branch_number == 2);

  const OperationLog& log_inc = operations.at(2);
  REQUIRE(log_inc.branch_number == 2);
}

TEST_CASE("For loop without init.", "[branches][for_loops]")
{
  RunOnCodeFixture fixture;

  const std::string code = "int main() { int a; for (;a < 4 ;a++) {} }";
  const auto& operations = fixture(code);

  REQUIRE(operations.size() == 2);

  const OperationLog& log_cond = operations.at(0);
  REQUIRE(log_cond.branch_number == 0);

  const OperationLog& log_inc = operations.at(1);
  REQUIRE(log_inc.branch_number == 0);
}

TEST_CASE("For loop closes branches inside the loop.", "[branches][for_loops]")
{
  RunOnCodeFixture fixture;

  const std::string code = "int main() { int a; for (a = 4; a < 4 ; a++) { a = 5; } }";
  const auto& operations = fixture(code);

  REQUIRE(operations.size() == 4);

  const OperationLog& log_inner = operations.back();
  REQUIRE(log_inner.branch_number == 0);
}

TEST_CASE("For loop closes branches outside the loop.", "[branches][for_loops]")
{
  RunOnCodeFixture fixture;

  const std::string code = "int main() { int a; for (a = 4; a < 4 ; a++) {} a = 5; }";
  const auto& operations = fixture(code);

  REQUIRE(operations.size() == 4);

  const OperationLog& log_outer = operations.back();
  REQUIRE(log_outer.branch_number == 0);
}

