#include "RunOnCodeFixture.hpp"

#include <catch2/catch.hpp>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>

RunOnCodeFixture::RunOnCodeFixture()
    : op_counter()
    , storage(&op_counter)
    , finder(&storage, &op_counter)
    , action(&finder, &op_counter)
{
}

bool RunOnCodeFixture::runCode(const std::string& code)
{
    return clang::tooling::runToolOnCode(
        clang::tooling::newFrontendActionFactory(&action)->create(),
        code,
        RunOnCodeFixture::INPUT_FILE
    );
}

const std::vector<OperationLog>& RunOnCodeFixture::operator()(const std::string& code)
{
    const bool success = runCode(code);

    REQUIRE(success);

    const auto& operations = storage.getOperations();

    REQUIRE(operations.count(RunOnCodeFixture::INPUT_FILE) == 1);

    return operations.at(RunOnCodeFixture::INPUT_FILE);
}
