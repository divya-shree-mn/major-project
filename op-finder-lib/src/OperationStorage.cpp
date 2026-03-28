#include "OperationStorage.hpp"

#include <fstream>
#include <filesystem>
#include <iomanip>
#include <llvm/Support/CommandLine.h>
#include "clang/Basic/SourceManager.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"

OperationStorage::OperationStorage(OperatorCounter* op_counter)
    : _op_counter(op_counter)
{
}

void OperationStorage::enablePrettyPrint()
{
    _pretty_print = true;
}

void OperationStorage::toStream(std::ostream& stream)
{
    nlohmann::json json_output;
    json_output["operations"] = _operations;
    json_output["user_defined_functions"] = _user_defined_function_ops;

    if (_pretty_print)
        stream << std::setw(4) << json_output;
    else
        stream << json_output;
}

void OperationStorage::toFile(const std::string& output_filename)
{
    std::ofstream file(output_filename);
    toStream(file);
}

void OperationStorage::pushOperation(const std::string& original_filename, OperationLog&& op)
{
    const std::string filename = _convertFilepath(original_filename);
    auto it = _operations.find(filename);

    if (it == _operations.end())
        it = _operations.emplace(filename, std::vector<OperationLog>()).first;

    it->second.emplace_back(std::move(op));
}

const std::unordered_map<std::string, std::vector<OperationLog>>& OperationStorage::getOperations() const
{
    return _operations;
}

const std::map<std::string, std::vector<std::string>>& OperationStorage::getUserDefinedFunctionOperators() const
{
    return _user_defined_function_ops;
}

std::string OperationStorage::_convertFilepath(const std::string& original)
{
    const std::filesystem::path path = original;
    return path.filename().string();
}

void OperationStorage::addLoop(const clang::Stmt* loop_stmt,
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
                               const clang::SourceManager& source_manager)
{
    clang::SourceLocation loc = loop_stmt->getBeginLoc();
    unsigned int line = source_manager.getSpellingLineNumber(loc);
    unsigned int column = source_manager.getSpellingColumnNumber(loc);

    auto loop_info = std::make_unique<LoopInfo>();
    loop_info->loop_type = loop_type;
    loop_info->initialization_text = initialization_text;
    loop_info->stop_condition_text = stop_condition_text;
    loop_info->increment_decrement_text = increment_decrement_text;
    loop_info->initial_value = initial_value;
    loop_info->initial_value_resolved = initial_value_resolved;
    loop_info->stop_value = stop_value;
    loop_info->stop_value_resolved = stop_value_resolved;
    loop_info->comparison_operator = comparison_operator;
    loop_info->increment_value = increment_value;
    loop_info->increment_value_resolved = increment_value_resolved;
    loop_info->iteration_count = iteration_count;
    loop_info->iteration_count_resolved = iteration_count_resolved;

    OperationLog op_log;
    op_log.line = line;
    op_log.column = column;
    op_log.entry_type = LoopInfo::TYPE_NAME;
    op_log.entry = std::move(loop_info);

    std::string filename = source_manager.getFilename(loc).str();
    if (filename.empty()) {
        filename = "unknown_file";
    }

    pushOperation(filename, std::move(op_log));
}

void OperationStorage::addFunctionCall(const clang::Expr* call_expr,
                                       const std::string& function_name,
                                       const std::string& call_result_type,
                                       bool is_system_call,
                                       const clang::SourceManager& source_manager)
{
    clang::SourceLocation loc = call_expr->getBeginLoc();
    unsigned int line = source_manager.getSpellingLineNumber(loc);
    unsigned int column = source_manager.getSpellingColumnNumber(loc);

    auto func_call = std::make_unique<FunctionCall>();
    func_call->function_name = function_name;
    func_call->call_result_type = call_result_type;
    func_call->is_system_call = is_system_call;

    OperationLog op_log;
    op_log.line = line;
    op_log.column = column;
    op_log.entry_type = FunctionCall::TYPE_NAME;
    op_log.entry = std::move(func_call);

    std::string filename = source_manager.getFilename(loc).str();
    if (filename.empty()) {
        filename = "unknown_file";
    }

    pushOperation(filename, std::move(op_log));

    if (_op_counter) {
        _op_counter->recordFunctionCall(function_name);
    }
}

void OperationStorage::recordUserDefinedFunctionOperator(const std::string& function_name, const std::string& op_name)
{
    _user_defined_function_ops[function_name].push_back(op_name);
}

void OperationStorage::addBinaryOperator(const clang::BinaryOperator* bin_op, const std::string& op_code, const std::string& op_type_name, long long multiplier, const clang::SourceManager& source_manager) {
    clang::SourceLocation loc = bin_op->getBeginLoc();
    unsigned int line = source_manager.getSpellingLineNumber(loc);
    unsigned int column = source_manager.getSpellingColumnNumber(loc);

    auto basic_op = std::make_unique<BasicOperation>();
    basic_op->opcode = op_code;
    basic_op->op_type_name = op_type_name;
    basic_op->operand_type = bin_op->getType().getAsString();

    OperationLog op_log;
    op_log.line = line;
    op_log.column = column;
    op_log.entry_type = BasicOperation::TYPE_NAME;
    op_log.entry = std::move(basic_op);

    std::string filename = source_manager.getFilename(loc).str();
    if (filename.empty()) {
        filename = "unknown_file";
    }

    pushOperation(filename, std::move(op_log));
    if (_op_counter) {
        _op_counter->recordBinaryOperator(op_code, 0, multiplier, bin_op->getType().getAsString());
    }
}

void OperationStorage::addUnaryOperator(const clang::UnaryOperator* un_op, const std::string& op_code, const std::string& op_type_name, long long multiplier, const clang::SourceManager& source_manager) {
    clang::SourceLocation loc = un_op->getBeginLoc();
    unsigned int line = source_manager.getSpellingLineNumber(loc);
    unsigned int column = source_manager.getSpellingColumnNumber(loc);

    auto basic_op = std::make_unique<BasicOperation>();
    basic_op->opcode = op_code;
    basic_op->op_type_name = op_type_name;
    basic_op->operand_type = un_op->getSubExpr()->getType().getAsString();

    OperationLog op_log;
    op_log.line = line;
    op_log.column = column;
    op_log.entry_type = BasicOperation::TYPE_NAME;
    op_log.entry = std::move(basic_op);

    std::string filename = source_manager.getFilename(loc).str();
    if (filename.empty()) {
        filename = "unknown_file";
    }

    pushOperation(filename, std::move(op_log));
    if (_op_counter) {
        _op_counter->recordUnaryOperator(op_code, 0, multiplier, un_op->getSubExpr()->getType().getAsString());
    }
}

void OperationStorage::addIfStatement(const clang::IfStmt* if_stmt, const std::string& condition_text, const clang::SourceManager& source_manager) {
    clang::SourceLocation loc = if_stmt->getBeginLoc();
    unsigned int line = source_manager.getSpellingLineNumber(loc);
    unsigned int column = source_manager.getSpellingColumnNumber(loc);
    
    auto if_info = std::make_unique<IfInfo>();
    if_info->condition = condition_text;
    if_info->has_else_branch = (if_stmt->getElse() != nullptr);

    OperationLog op_log;
    op_log.line = line;
    op_log.column = column;
    op_log.entry_type = IfInfo::TYPE_NAME;
    op_log.entry = std::move(if_info);
    
    std::string filename = source_manager.getFilename(loc).str();
    if (filename.empty()) {
        filename = "unknown_file";
    }

    pushOperation(filename, std::move(op_log));
}

void OperationStorage::addArraySubscript(const clang::ArraySubscriptExpr* subscript, const std::string& op_type_name, const clang::SourceManager& source_manager) {
    clang::SourceLocation loc = subscript->getBeginLoc();
    unsigned int line = source_manager.getSpellingLineNumber(loc);
    unsigned int column = source_manager.getSpellingColumnNumber(loc);
    
    auto basic_op = std::make_unique<BasicOperation>();
    basic_op->opcode = "[]";
    basic_op->op_type_name = op_type_name;
    basic_op->operand_type = subscript->getType().getAsString();
    
    OperationLog op_log;
    op_log.line = line;
    op_log.column = column;
    op_log.entry_type = BasicOperation::TYPE_NAME;
    op_log.entry = std::move(basic_op);

    std::string filename = source_manager.getFilename(loc).str();
    if (filename.empty()) {
        filename = "unknown_file";
    }

    pushOperation(filename, std::move(op_log));
}