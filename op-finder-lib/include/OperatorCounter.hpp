#ifndef C_ANALYZER_OPERATORCOUNTER_HPP
#define C_ANALYZER_OPERATORCOUNTER_HPP

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include "nlohmann/json.hpp"
#include "llvm/ADT/StringMap.h"

class OperatorCounter {
public:
    OperatorCounter();

    void recordBinaryOperator(const std::string& op_code, int branch_number, long long multiplier, const std::string& type_name);
    void recordUnaryOperator(const std::string& op_code, int branch_number, long long multiplier, const std::string& type_name);

    void recordLoopInfo(int branch_number, long long iteration_count);
    void recordFunctionCall(const std::string& function_name);
    
    int getGateCostForFunction(const std::string& function_name) const;
    void recordSystemFunctionGate(const std::string& function_name, int gate_cost);

    void printCounts(std::ostream& os) const;
    void printFunctionCallCounts(std::ostream& os) const;

    void loadGateCounts(const nlohmann::json& gate_counts_json);
    void printBasicOpGateBreakdown(std::ostream& os) const;
    
    void scaleUserDefinedFunctionOperators(const std::map<std::string, std::vector<std::string>>& user_defined_function_ops);    
    const llvm::StringMap<int>& getFunctionCallCounts() const;
    long long getTotalGateCount() const;

private:
    std::map<int, std::map<std::string, long long>> _binaryOperatorCountsByBranch;
    std::map<int, std::map<std::string, long long>> _unaryOperatorCountsByBranch;
    
    std::map<int, long long> _loopIterationCounts;
    std::map<std::string, int> _gateCounts;
    
    std::map<std::string, int> _systemCallGateCounts;
    llvm::StringMap<int> _functionCallCounts;

    long long getEffectiveMultiplier(int branch_number) const;
};

#endif // C_ANALYZER_OPERATORCOUNTER_HPP
