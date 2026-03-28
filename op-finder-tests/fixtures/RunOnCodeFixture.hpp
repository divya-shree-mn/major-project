#ifndef C_ANALYZER_RUNONCODEFIXTURE_HPP
#define C_ANALYZER_RUNONCODEFIXTURE_HPP

#include <OperationStorage.hpp>
#include <OperationFinder.hpp>
#include <OperationFinderAstAction.hpp>
#include <OperatorCounter.hpp>

struct RunOnCodeFixture
{
    constexpr static char INPUT_FILE[] = "input.c";

    OperationStorage storage;
    OperatorCounter op_counter;
    OperationFinder finder;
    OperationFinderAstAction action;

    RunOnCodeFixture();

    bool runCode(const std::string& code);

    const std::vector<OperationLog>& operator()(const std::string& code);
};

#endif //C_ANALYZER_RUNONCODEFIXTURE_HPP
