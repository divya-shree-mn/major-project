#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include <iostream>
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <iomanip>
#include "OperationFinder.hpp"
#include "OperationStorage.hpp"
#include "OperationFinderAstAction.hpp"
#include "OperatorCounter.hpp"

using namespace clang::tooling;
using namespace clang::ast_matchers;
using namespace llvm;

cl::opt<std::string> GatesFile(
    "gates-file",
    cl::desc("Specify path to gates.json lookup table for gate count analysis"),
    cl::value_desc("filename"),
    cl::init("gates.json")
);

static cl::OptionCategory MyToolCategory("op-finder options");
static cl::opt<std::string> OutputFile("o", cl::desc("File to output the JSON to."), cl::cat(MyToolCategory));
static cl::opt<std::string> RootDirectory("r", cl::desc("The root directory of the source files."), cl::cat(MyToolCategory));
static cl::opt<bool> PrettyPrint("pretty", cl::desc("Pretty-print the output JSON."));
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::extrahelp MoreHelp("\nThe program takes the input <source0> ... files, parses their\n"
                              "AST and outputs a singular file containing a list of all noteworthy operations\n"
                              "for later analysis.\n");

int main(int argc, const char** argv)
{
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory, llvm::cl::ZeroOrMore, "Usage: op-finder <options> <files...>");

    if (!ExpectedParser)
    {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }

    CommonOptionsParser& OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    OperatorCounter op_counter;
    OperationStorage storage(&op_counter);

    if (PrettyPrint.getValue())
        storage.enablePrettyPrint();

    OperationFinder op_finder(&storage, &op_counter);

    if (!GatesFile.empty()) {
        std::ifstream gates_ifs(GatesFile.getValue());
        if (!gates_ifs.is_open()) {
            std::cerr << "Error: Could not open gates file: " << GatesFile.getValue() << "\n";
            return 1;
        }
        try {
            nlohmann::json gates_json = nlohmann::json::parse(gates_ifs);
            op_finder.loadGateCounts(gates_json);
            op_counter.loadGateCounts(gates_json);
        }
        catch (const nlohmann::json::parse_error& e) {
            std::cerr << "Error parsing gates file '" << GatesFile.getValue() << "': " << e.what() << "\n";
            return 1;
        }
    }

    OperationFinderAstAction action(&op_finder, &op_counter);
    Tool.run(newFrontendActionFactory(&action).get());

    const auto& user_defined_function_ops = op_finder.getUserDefinedFunctionOperators();    
    op_counter.scaleUserDefinedFunctionOperators(user_defined_function_ops);

    op_counter.printBasicOpGateBreakdown(std::cout);
    long long total_estimated_gates_from_basic_ops = op_counter.getTotalGateCount();    

    long long total_gates_from_system_calls = 0;
    const auto& function_call_counts = op_counter.getFunctionCallCounts();
    const auto& gate_counts = op_finder.getGateCounts();
    
    std::cout << "\n--- System Call Gate Analysis ---\n";
    for (const auto& pair : function_call_counts) {
        const std::string function_name = pair.first().str();
        long long call_count = pair.second;

        auto it = gate_counts.find(function_name);
        if (it != gate_counts.end()) {
            long long cost = it->second;
            long long function_gates = cost * call_count;
            total_gates_from_system_calls += function_gates;
            
            std::cout << "    " << std::left << std::setw(15) << function_name << ": "<< std::setw(5) << call_count << " calls * "
                      << std::setw(5) << cost << " gates/call = "<< function_gates << " gates\n";
        }
    }

    std::cout << "Total gates from system calls: " << total_gates_from_system_calls << "\n";
    std::cout << "--------------------------------\n";

    long long total_combined_gates = total_estimated_gates_from_basic_ops + total_gates_from_system_calls;
    std::cout << "\n--- Combined Gate Count ---\n";
    std::cout << "Total estimated gates (Basic Ops + System Calls): " << total_combined_gates << "\n";
    std::cout << "\n";
    
    if (!OutputFile.getValue().empty())
        storage.toFile(OutputFile.getValue());
    else
        storage.toStream(std::cout);

    op_counter.printCounts(std::cout);
    return 0;
}
