#include "OperationFinder.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <vector>

#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/raw_ostream.h"
#include "ASTHelpers.hpp"
#include "OperationFinderAstVisitor.hpp"
#include "OperationLog.hpp"
#include "nlohmann/json.hpp"

using namespace clang;
using namespace clang::ast_matchers;

namespace {
    std::string ResolveTypeName(const clang::QualType& t) {
        if (const clang::TypedefType* tdt = clang::dyn_cast<clang::TypedefType>(t.getTypePtr())) {
            return tdt->desugar().getAsString();
        } else if (t->isBuiltinType()) {
            return t.getAsString();
        } else {
            return t.getAsString();
        }
    }
}

OperationFinder::OperationFinder(IOperationOutput* storage, OperatorCounter* op_counter)
    : _storage(storage),
      _op_counter(op_counter),
      _context(nullptr),
      _function_gate_counts(),
      _currently_processing_functions(),
      _current_branch(0),
      _total_program_gate_count(0),
      _target_function_name(""),
      _operators_in_target_function(),
      _gateCounts() {
    assert(storage && "OperationStorage pointer cannot be null.");
    assert(op_counter && "OperatorCounter pointer cannot be null.");
}

void OperationFinder::processArithmetic(const clang::BinaryOperator* op, const clang::SourceManager& source_manager, long long multiplier) {
    const std::string op_code = op->getOpcodeStr().str();
    _storage->pushOperation(source_manager.getFilename(op->getBeginLoc()).str(),
                            OperationLog(source_manager.getSpellingLineNumber(op->getBeginLoc()),
                                         source_manager.getSpellingColumnNumber(op->getBeginLoc()),
                                         BasicOperation::TYPE_NAME,
                                         _createBasicOperationLogEntry(op, op_code, op->getLHS(), op->getRHS())));

    _op_counter->recordBinaryOperator(op_code, _current_branch, multiplier, op->getType().getAsString());

    clang::SourceLocation loc = op->getBeginLoc();
    llvm::outs() << source_manager.getFilename(loc) << ":"<< source_manager.getSpellingLineNumber(loc) << ":"<< source_manager.getSpellingColumnNumber(loc) << ":\n";
    llvm::outs() << "\tBinary arithmetic: Type: " << op_code << " in Branch: " << _current_branch << "\n";
}

void OperationFinder::processUnaryArithmetic(const clang::UnaryOperator* op, const clang::SourceManager& source_manager, long long multiplier) {
    const std::string op_code = clang::UnaryOperator::getOpcodeStr(op->getOpcode()).str();
    _storage->pushOperation(source_manager.getFilename(op->getBeginLoc()).str(),
                            OperationLog(source_manager.getSpellingLineNumber(op->getBeginLoc()),
                                         source_manager.getSpellingColumnNumber(op->getBeginLoc()),
                                         BasicOperation::TYPE_NAME,
                                         _createBasicOperationLogEntry(op, op_code, op->getSubExpr(), nullptr)));

    _op_counter->recordUnaryOperator(op_code, _current_branch, multiplier, op->getType().getAsString());

    clang::SourceLocation loc = op->getBeginLoc();
    llvm::outs() << source_manager.getFilename(loc) << ":"
                 << source_manager.getSpellingLineNumber(loc) << ":"
                 << source_manager.getSpellingColumnNumber(loc) << ":\n";
    llvm::outs() << "\tUnary arithmetic: Type: " << op_code << " in Branch: " << _current_branch << "\n";
}

void OperationFinder::processFunctionCall(const clang::CallExpr* call, const std::string& function_name, const clang::SourceManager& source_manager) {
    
    const clang::FunctionDecl* func = call->getDirectCallee();
    bool isSystemCall = func && source_manager.isInSystemHeader(func->getBeginLoc());

    _storage->addFunctionCall(call, function_name, call->getType().getAsString(), isSystemCall, source_manager);

    clang::SourceLocation loc = call->getBeginLoc();
    llvm::outs() << source_manager.getFilename(loc) << ":"<< source_manager.getSpellingLineNumber(loc) << ":"<< source_manager.getSpellingColumnNumber(loc) << ":\n";
    llvm::outs() << "\tFunction call: func name: " << function_name << " in Branch: " << _current_branch << "\n";
    llvm::outs() << "\tResult eval type: " << call->getType().getAsString() << "\n";
}

void OperationFinder::processArraySubscript(const clang::ArraySubscriptExpr* subscript, const clang::SourceManager& source_manager) {
    std::string opcode = "[]";
    _storage->pushOperation(source_manager.getFilename(subscript->getBeginLoc()).str(),
                            OperationLog(source_manager.getSpellingLineNumber(subscript->getBeginLoc()),
                                         source_manager.getSpellingColumnNumber(subscript->getBeginLoc()),
                                         BasicOperation::TYPE_NAME,
                                         _createBasicOperationLogEntry(subscript, opcode, subscript->getBase(), subscript->getRHS())));

    _op_counter->recordUnaryOperator("subscript", _current_branch, 1, subscript->getType().getAsString());

    clang::SourceLocation loc = subscript->getBeginLoc();
    llvm::outs() << source_manager.getFilename(loc) << ":"<< source_manager.getSpellingLineNumber(loc) << ":"<< source_manager.getSpellingColumnNumber(loc) << ":\n";
    llvm::outs() << "\tArray subscript in Branch: " << _current_branch << "\n";
    llvm::outs() << "\tExpression type: " << subscript->getType().getAsString() << "\n";
    
}

void OperationFinder::processLoop(const clang::Stmt* loop_stmt,
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
                                  const clang::SourceManager& source_manager) {
    _storage->addLoop(loop_stmt, loop_type,
                      initialization_text, stop_condition_text, increment_decrement_text,
                      initial_value, initial_value_resolved,
                      stop_value, stop_value_resolved,
                      comparison_operator,
                      increment_value, increment_value_resolved,
                      iteration_count, iteration_count_resolved,
                      source_manager);

    if (_op_counter) {
        _op_counter->recordLoopInfo(_current_branch, iteration_count);
    }

    clang::SourceLocation loc = loop_stmt->getBeginLoc();
    llvm::outs() << source_manager.getFilename(loc) << ":"<< source_manager.getSpellingLineNumber(loc) << ":"<< source_manager.getSpellingColumnNumber(loc) << ":\n";
    llvm::outs() << "\tLoop Found: Type: " << loop_type << " in Branch: " << _current_branch << "\n";
    llvm::outs() << "\t\tIteration Count: ";
    if (iteration_count_resolved) {
        llvm::outs() << iteration_count << "\n";
    } else {
        llvm::outs() << "Cannot Determine\n";
    }
}

void OperationFinder::processIfStatement(const clang::IfStmt* if_stmt, const std::string& condition_text, const clang::SourceManager& source_manager) {
    clang::SourceLocation loc = if_stmt->getBeginLoc();
    unsigned int line = source_manager.getSpellingLineNumber(loc);
    unsigned int column = source_manager.getSpellingColumnNumber(loc);

    auto if_info = std::make_unique<IfInfo>();
    if_info->condition = condition_text;
    if_info->has_else_branch = (if_stmt->getElse() != nullptr);

    std::string filename = source_manager.getFilename(loc).str();
    if (filename.empty()) {
        filename = "unknown_file";
    }

    _storage->pushOperation(filename, OperationLog(line, column, IfInfo::TYPE_NAME, std::move(if_info)));

    llvm::outs() << filename << ":" << line << ":" << column << ":\n";
    llvm::outs() << "\tIf Statement Found in Branch: " << _current_branch << "\n";
    llvm::outs() << "\t\tCondition: " << condition_text << "\n";
    llvm::outs() << "\t\tHas Else Branch: " << (if_stmt->getElse() != nullptr ? "Yes" : "No") << "\n";
}

void OperationFinder::processBinaryOperator(const clang::BinaryOperator* op, const std::string& op_text,
                                            const std::string& type_name, long long multiplier, const clang::SourceManager& source_manager) {
    processArithmetic(op, source_manager, multiplier);
}

void OperationFinder::processUnaryOperator(const clang::UnaryOperator* op, const std::string& op_text,
                                           const std::string& type_name, long long multiplier, const clang::SourceManager& source_manager) {
    processUnaryArithmetic(op, source_manager, multiplier);
}

void OperationFinder::branchEntered() {
    _current_branch++;
}

void OperationFinder::branchExited() {
    assert(_current_branch > -1);
    _current_branch--;
}

void OperationFinder::recordUserDefinedFunctionOperator(const std::string& function_name, const std::string& op_name) {
    if (_storage) {
        _storage->recordUserDefinedFunctionOperator(function_name, op_name);
    }
    if (function_name == _target_function_name) {
        _operators_in_target_function.push_back(op_name);
    }
}

void OperationFinder::setTargetFunction(const std::string& functionName) {
    _target_function_name = functionName;
    _operators_in_target_function.clear();
}

const std::vector<std::string>& OperationFinder::getOperatorsInTargetFunction() const {
    return _operators_in_target_function;
}

const std::map<std::string, std::vector<std::string>>& OperationFinder::getUserDefinedFunctionOperators() const {
    return _storage->getUserDefinedFunctionOperators();
}

void OperationFinder::setASTContext(clang::ASTContext& context) {
    _context = &context;
}

void OperationFinder::run(const clang::ast_matchers::MatchFinder::MatchResult& Result) {
    (void)Result;
}

unsigned long long OperationFinder::_getGateCountForFunction(const clang::FunctionDecl* func_decl) {
    std::string func_name = func_decl ? func_decl->getNameAsString() : "null_func";

    if (!func_decl || func_decl->isInvalidDecl() || !func_decl->hasBody()) {
        return 0;
    }

    if (_currently_processing_functions.count(func_decl)) {
        return 0;
    }

    auto it = _function_gate_counts.find(func_decl);
    if (it != _function_gate_counts.end()) {
        return it->second;
    }

    _currently_processing_functions.insert(func_decl);

    unsigned long long current_function_gates = 0;
    if (_context) {
        OperationFinderAstVisitor visitor(this, _op_counter);
        visitor.NewContext(_context);
        visitor.TraverseDecl(const_cast<clang::FunctionDecl*>(func_decl));
        current_function_gates = visitor.getCollectedGateCount();
    }

    _currently_processing_functions.erase(func_decl);
    _function_gate_counts[func_decl] = current_function_gates;
    return current_function_gates;
}

std::unique_ptr<BasicOperation>
OperationFinder::_createBasicOperationLogEntry(const clang::Expr* source, const std::string& opcode, const clang::Expr* op1, const clang::Expr* op2) {
    auto basic_op = std::make_unique<BasicOperation>();
    basic_op->opcode = opcode;
    basic_op->type_result = source->getType().getAsString();

    if (op1) {
        basic_op->operand_type = op1->getType().getAsString();
        if (_context && op1->isEvaluatable(*_context)) {
            basic_op->operand_values.push_back(std::to_string(op1->EvaluateKnownConstInt(*_context).getExtValue()));
        } else {
            basic_op->operand_values.push_back("");
        }
    }
    if (op2) {
        if (_context && op2->isEvaluatable(*_context)) {
            basic_op->operand_values.push_back(std::to_string(op2->EvaluateKnownConstInt(*_context).getExtValue()));
        } else {
            basic_op->operand_values.push_back("");
        }
    }
    
    return basic_op;
}

std::pair<std::string, OperationLog> OperationFinder::_createBaseOperationLog(const clang::Stmt* stmt, const clang::SourceManager& source_manager) {
    clang::SourceLocation loc = stmt->getBeginLoc();
    unsigned int line = source_manager.getSpellingLineNumber(loc);
    unsigned int column = source_manager.getSpellingColumnNumber(loc);

    std::string filename = source_manager.getFilename(loc).str();
    if (filename.empty()) {
        filename = "unknown_file";
    }

    OperationLog op_log;
    op_log.line = line;
    op_log.column = column;
    return {filename, std::move(op_log)};
}

void OperationFinder::loadGateCounts(const nlohmann::json& gate_counts_json) {
    _gateCounts.clear();
    for (nlohmann::json::const_iterator it = gate_counts_json.begin(); it != gate_counts_json.end(); ++it) {
        if (it.value().is_number_integer()) {
            _gateCounts[it.key()] = it.value().get<int>();
        }
    }
}

const std::map<std::string, int>& OperationFinder::getGateCounts() const {
    return _gateCounts;
}
