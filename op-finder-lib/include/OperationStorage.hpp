#ifndef C_ANALYZER_OPERATIONSTORAGE_HPP
#define C_ANALYZER_OPERATIONSTORAGE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <memory>

#include "OperationLog.hpp"
#include "OperatorCounter.hpp"
#include "clang/Basic/SourceManager.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/Decl.h"

class OperationStorage : public IOperationOutput
{
public:
    OperationStorage() = default;
    
    explicit OperationStorage(OperatorCounter* op_counter);
    OperationStorage(const OperationStorage&) = delete;

    ~OperationStorage() override = default;

    void enablePrettyPrint();
    void toStream(std::ostream& stream);
    void toFile(const std::string& output_filename);

    void pushOperation(const std::string& original_filename, OperationLog&& op) override;
    [[nodiscard]] const std::unordered_map<std::string, std::vector<OperationLog>>& getOperations() const;

    void addLoop(const clang::Stmt* loop_stmt,
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
                 const clang::SourceManager& source_manager) override;

    void addFunctionCall(const clang::Expr* call_expr,
                         const std::string& function_name,
                         const std::string& call_result_type,
                         bool is_system_call,
                         const clang::SourceManager& source_manager) override;

    void recordUserDefinedFunctionOperator(const std::string& function_name, const std::string& op_name) override;
    
    [[nodiscard]] const std::map<std::string, std::vector<std::string>>& getUserDefinedFunctionOperators() const override;

    void addBinaryOperator(const clang::BinaryOperator* bin_op, const std::string& op_code, const std::string& op_type_name, long long multiplier, const clang::SourceManager& source_manager) override;
    void addUnaryOperator(const clang::UnaryOperator* un_op, const std::string& op_code, const std::string& op_type_name, long long multiplier, const clang::SourceManager& source_manager) override;
    void addIfStatement(const clang::IfStmt* if_stmt, const std::string& condition_text, const clang::SourceManager& source_manager) override;
    void addArraySubscript(const clang::ArraySubscriptExpr* subscript_expr, const std::string& array_name, const clang::SourceManager& source_manager) override;

private:
    std::unordered_map<std::string, std::vector<OperationLog>> _operations;
    
    OperatorCounter* _op_counter;
    bool _pretty_print = false;

    std::string _convertFilepath(const std::string& original);
    std::map<std::string, std::vector<std::string>> _user_defined_function_ops;
};

#endif // C_ANALYZER_OPERATIONSTORAGE_HPP
