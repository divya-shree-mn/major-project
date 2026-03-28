#ifndef LLVM_PROTO_OPERATIONFINDER_HPP
#define LLVM_PROTO_OPERATIONFINDER_HPP

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <memory>
#include "nlohmann/json.hpp"
#include "OperationLog.hpp"
#include "OperatorCounter.hpp"
#include "OperationStorage.hpp"

namespace clang {
    class Stmt;
    class Expr;
    class BinaryOperator;
    class UnaryOperator;
    class CallExpr;
    class ArraySubscriptExpr;
    class IfStmt;
    class SourceManager;
    class ASTContext;
    class FunctionDecl;
    namespace ast_matchers {
        class MatchResult;
        class MatchFinder;
    }
}

class FunctionOperationCounterVisitor;

class OperationFinder
{
public:
    explicit OperationFinder(IOperationOutput* storage, OperatorCounter* op_counter);

    void processArithmetic(const clang::BinaryOperator* op, const clang::SourceManager& source_manager, long long multiplier);
    void processUnaryArithmetic(const clang::UnaryOperator* op, const clang::SourceManager& source_manager, long long multiplier);
    
    void processFunctionCall(const clang::CallExpr* call, const std::string& function_name, const clang::SourceManager& source_manager);        
    void processArraySubscript(const clang::ArraySubscriptExpr* subscript, const clang::SourceManager& source_manager);

    void processBinaryOperator(const clang::BinaryOperator* op, const std::string& op_text, const std::string& type_name, long long multiplier, const clang::SourceManager& source_manager);
    void processUnaryOperator(const clang::UnaryOperator* op, const std::string& op_text, const std::string& type_name, long long multiplier, const clang::SourceManager& source_manager);
    
    void processLoop(const clang::Stmt* loop_stmt,
                     const std::string& loop_type,
                     const std::string& initialization_text,
                     const std::string& stop_condition_text,
                     const std::string& increment_decrement_text,
                     long long initial_value,
                     bool initial_value_resolved,
                     long long stop_value,
                     bool stop_value_resolved,
                     const std::string& comparison_operator,
                     long long increment_value,
                     bool increment_value_resolved,
                     long long iteration_count,
                     bool iteration_count_resolved,
                     const clang::SourceManager& source_manager);

    void processIfStatement(const clang::IfStmt* if_stmt,
                            const std::string& condition_text,
                            const clang::SourceManager& source_manager);

    int getCurrentBranch() const { return _current_branch; }

    void branchEntered();
    void branchExited();

    unsigned long long getTotalProgramGateCount() const { return _total_program_gate_count; }
    void recordUserDefinedFunctionOperator(const std::string& function_name, const std::string& op_name);

    void setTargetFunction(const std::string& functionName);
    const std::vector<std::string>& getOperatorsInTargetFunction() const;

    const std::map<std::string, std::vector<std::string>>& getUserDefinedFunctionOperators() const;
    void run(const clang::ast_matchers::MatchFinder::MatchResult& Result);

    void setASTContext(clang::ASTContext& context);
    IOperationOutput* getStorage() const { return _storage; }

    void loadGateCounts(const nlohmann::json& gate_counts_json);
    const std::map<std::string, int>& getGateCounts() const;

private:
    clang::ASTContext* _context;
    
    IOperationOutput* _storage;
    OperatorCounter* _op_counter;

    std::map<const clang::FunctionDecl*, unsigned long long> _function_gate_counts;
    std::set<const clang::FunctionDecl*> _currently_processing_functions; 

    unsigned long long _getGateCountForFunction(const clang::FunctionDecl* func_decl);

    std::unique_ptr<BasicOperation>
        _createBasicOperationLogEntry(const clang::Expr* source, const std::string& opcode, const clang::Expr* op1, const clang::Expr* op2);

    std::pair<std::string, OperationLog> _createBaseOperationLog(const clang::Stmt* stmt, const clang::SourceManager& source_manager);

    int _current_branch = 0;
    unsigned long long _total_program_gate_count = 0;

    std::string _target_function_name;
    std::vector<std::string> _operators_in_target_function;
    std::map<std::string, int> _gateCounts;
};

#endif // LLVM_PROTO_OPERATIONFINDER_HPP
