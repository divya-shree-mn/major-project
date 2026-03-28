#include "OperatorCounter.hpp"
#include <iomanip>
#include <map>
#include <cmath>
#include <iostream>
#include <vector>
#include "nlohmann/json.hpp"
#include <llvm/Support/raw_ostream.h>

OperatorCounter::OperatorCounter() = default;

int OperatorCounter::getGateCostForFunction(const std::string& function_name) const {
    auto it = _gateCounts.find(function_name);
    if (it != _gateCounts.end()) {
        return it->second;
    }
    return 0;
}

void OperatorCounter::recordSystemFunctionGate(const std::string& function_name, int gate_cost) {
    _systemCallGateCounts[function_name] += gate_cost;
}

void OperatorCounter::loadGateCounts(const nlohmann::json& gate_counts_json) {
    _gateCounts.clear();
    for (nlohmann::json::const_iterator it = gate_counts_json.begin(); it != gate_counts_json.end(); ++it) {
        if (it.value().is_number_integer()) {
            _gateCounts[it.key()] = it.value().get<int>();
        } else {
            std::cerr << "Warning: gates.json: Non-integer gate count for operator '" << it.key() << "'. Skipping.\n";
        }
    }
}

long long OperatorCounter::getEffectiveMultiplier(int branch_number) const {
    long long effective_multiplier = 1;
    for (int i = 1; i < branch_number; ++i) {
        auto it = _loopIterationCounts.find(i);
        if (it != _loopIterationCounts.end() && it->second > 0) {
            effective_multiplier *= it->second;
        }
    }
    return effective_multiplier;
}

const llvm::StringMap<int>& OperatorCounter::getFunctionCallCounts() const {
    return _functionCallCounts;
}

void OperatorCounter::recordBinaryOperator(const std::string& op_code, int branch_number, long long multiplier, const std::string& type_name) {
    std::string full_op_name = op_code + ":" + type_name;
    _binaryOperatorCountsByBranch[branch_number][full_op_name] += multiplier;
}

void OperatorCounter::recordUnaryOperator(const std::string& op_code, int branch_number, long long multiplier, const std::string& type_name) {
    std::string full_op_name = op_code + ":" + type_name;
    _unaryOperatorCountsByBranch[branch_number][full_op_name] += multiplier;
}

void OperatorCounter::recordLoopInfo(int branch_number, long long iteration_count) {
    _loopIterationCounts[branch_number] = iteration_count;
}

void OperatorCounter::recordFunctionCall(const std::string& function_name) {
    _functionCallCounts[function_name]++;
}

void OperatorCounter::printCounts(std::ostream& os) const {
    os << "\n--- Operator Counts ---\n";
    os << "Binary Operators:\n";
    if (_binaryOperatorCountsByBranch.empty()) {
        os << "    No binary operators found.\n";
    } else {
        for (const auto& branch_pair : _binaryOperatorCountsByBranch) {
            int branch_number = branch_pair.first;
            const auto& op_counts = branch_pair.second;
            os << "    Branch " << branch_number << ":\n";
            for (const auto& pair : op_counts) {
                os << "        " << std::left << std::setw(15) << pair.first << ": " << pair.second << "\n";
            }
        }
    }

    os << "\nUnary Operators:\n";
    if (_unaryOperatorCountsByBranch.empty()) {
        os << "    No unary operators found.\n";
    } else {
        for (const auto& branch_pair : _unaryOperatorCountsByBranch) {
            int branch_number = branch_pair.first;
            const auto& op_counts = branch_pair.second;
            os << "    Branch " << branch_number << ":\n";
            for (const auto& pair : op_counts) {
                os << "        " << std::left << std::setw(15) << pair.first << ": " << pair.second << "\n";
            }
        }
    }
    os << "-----------------------\n";

    printFunctionCallCounts(os);
}

void OperatorCounter::printFunctionCallCounts(std::ostream& os) const {
    os << "\n--- Function Call Counts ---\n";
    if (_functionCallCounts.empty()) {
        os << "    No function calls found.\n";
    } else {
        for (const auto& pair : _functionCallCounts) {
            os << "    " << std::left << std::setw(15) << pair.first().str() << ": " << pair.second << " calls\n";
        }
    }
    os << "----------------------------\n";
}

void OperatorCounter::printBasicOpGateBreakdown(std::ostream& os) const {
    long long basic_op_gates = 0;
    os << "\n--- Gate Count Analysis (Basic Operations) ---\n";

    std::map<std::string, long long> aggregated_op_counts;
    for (const auto& branch_pair : _binaryOperatorCountsByBranch) {
        for (const auto& op_pair : branch_pair.second) {
            aggregated_op_counts[op_pair.first] += op_pair.second;
        }
    }

    for (const auto& branch_pair : _unaryOperatorCountsByBranch) {
        for (const auto& op_pair : branch_pair.second) {
            aggregated_op_counts[op_pair.first] += op_pair.second;
        }
    }

    for (const auto& aggregated_pair : aggregated_op_counts) {
        const std::string& full_op_code = aggregated_pair.first;
        long long total_count = aggregated_pair.second;
        
        size_t type_separator = full_op_code.find(':');
        std::string op_code = full_op_code;
        std::string type_name = "unknown";
        if (type_separator != std::string::npos) {
            op_code = full_op_code.substr(0, type_separator);
            type_name = full_op_code.substr(type_separator + 1);
        }

        auto it = _gateCounts.find(op_code);
        if (it != _gateCounts.end()) {
            int gate_per_op = it->second;
            if (type_name == "double") {
                gate_per_op *= 2;
            }
            long long op_gates = total_count * gate_per_op;
            
            os << "    " << std::left << std::setw(15) << full_op_code << ": "
               << std::setw(5) << total_count << " instances * "
               << std::setw(5) << gate_per_op << " gates/op = "
               << op_gates << " gates\n";
            basic_op_gates += op_gates;
        } else {
            os << "    Warning: No gate count defined in gates.json for operator '" << op_code << "'. Skipping.\n";
        }
    }
    
    os << "\nTotal estimated gates (Basic Operations): " << basic_op_gates << "\n";
    os << "------------------------------------------\n";
}

void OperatorCounter::scaleUserDefinedFunctionOperators(
    const std::map<std::string, std::vector<std::string>>& user_defined_function_ops)
{
    if (_binaryOperatorCountsByBranch.count(0)) {
        for (const auto& func_pair : user_defined_function_ops) {
            const auto& operators = func_pair.second;
            for (const auto& op_name : operators) {
                if (_binaryOperatorCountsByBranch[0].count(op_name + ":int")) {
                    _binaryOperatorCountsByBranch[0].erase(op_name + ":int");
                }
                if (_binaryOperatorCountsByBranch[0].count(op_name + ":double")) {
                    _binaryOperatorCountsByBranch[0].erase(op_name + ":double");
                }
            }
        }
    }

    for (const auto& func_pair : user_defined_function_ops) {
        const std::string& function_name = func_pair.first;
        const auto& operators = func_pair.second;

        auto it = _functionCallCounts.find(function_name);
        if (it == _functionCallCounts.end()) {
            continue;
        }
        long long call_count = it->second;
        
        for (const auto& op_name : operators) {
            _binaryOperatorCountsByBranch[0][op_name + ":int"] += call_count;
        }
    }
}

long long OperatorCounter::getTotalGateCount() const {
    long long basic_op_gates = 0;

    std::map<std::string, long long> aggregated_op_counts;
    
    for (const auto& branch_pair : _binaryOperatorCountsByBranch) {
        for (const auto& op_pair : branch_pair.second) {
            aggregated_op_counts[op_pair.first] += op_pair.second;
        }
    }
    for (const auto& branch_pair : _unaryOperatorCountsByBranch) {
        for (const auto& op_pair : branch_pair.second) {
            aggregated_op_counts[op_pair.first] += op_pair.second;
        }
    }
    
    for (const auto& aggregated_pair : aggregated_op_counts) {
        const std::string& full_op_code = aggregated_pair.first;
        long long total_count = aggregated_pair.second;
        
        size_t type_separator = full_op_code.find(':');
        std::string op_code = full_op_code;
        std::string type_name = "unknown";
        if (type_separator != std::string::npos) {
            op_code = full_op_code.substr(0, type_separator);
            type_name = full_op_code.substr(type_separator + 1);
        }

        auto it = _gateCounts.find(op_code);
        if (it != _gateCounts.end()) {
            int gate_per_op = it->second;
            if (type_name == "double") {
                gate_per_op *= 2;
            }
            long long op_gates = total_count * gate_per_op;
            basic_op_gates += op_gates;
        }
    }
    return basic_op_gates;
}
