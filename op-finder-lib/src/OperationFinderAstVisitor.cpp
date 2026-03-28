#include "OperationFinderAstVisitor.hpp"
#include "OperationFinder.hpp"
#include "ASTHelpers.hpp"
#include "WhileLoop.hpp"

#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <clang/AST/OperationKinds.h>
#include <clang/Lex/Lexer.h>
#include <llvm/Support/raw_ostream.h>
#include <set>
#include <vector>
#include <string>
#include <cassert>
#include <utility>
#include <iostream>

using namespace clang;
using namespace llvm;

namespace {
    std::string GetSourceTextForStmt(const clang::Stmt* S, const clang::SourceManager& SM, const clang::LangOptions& LO) {
        if (!S) return "";
        clang::SourceLocation StartLoc = S->getBeginLoc();
        clang::SourceLocation EndLoc = S->getEndLoc();

        if (StartLoc.isMacroID() || EndLoc.isMacroID()) {
            if (StartLoc.isMacroID()) StartLoc = SM.getSpellingLoc(StartLoc);
            if (EndLoc.isMacroID()) EndLoc = SM.getSpellingLoc(EndLoc);

            if (!StartLoc.isValid() || !EndLoc.isValid() || StartLoc.isMacroID() || EndLoc.isMacroID()) {
                return "<macro expansion>";
            }
        }

        clang::CharSourceRange Range = clang::CharSourceRange::getCharRange(StartLoc, clang::Lexer::getLocForEndOfToken(EndLoc, 0, SM, LO));

        if (Range.isInvalid()) {
            Range = clang::CharSourceRange::getCharRange(StartLoc, EndLoc);
        }
        return clang::Lexer::getSourceText(Range, SM, LO).str();
    }
}

OperationFinderAstVisitor::OperationFinderAstVisitor(OperationFinder* op_finder, OperatorCounter* op_counter)
    : _context(nullptr), _op_finder(op_finder), _op_counter(op_counter),
      _whileLoopAnalyzer(std::make_unique<WhileLoopIterationAnalyzer>()),
      _loop_header(), _branch_stack(), _loop_stack(),
      _current_function_decl(nullptr), _collected_gates(0)
{
    assert(op_finder && "OperationFinder pointer cannot be null.");
    assert(op_counter && "OperatorCounter pointer cannot be null.");
}

void OperationFinderAstVisitor::NewContext(clang::ASTContext* context)
{
    _context = context;
    _whileLoopAnalyzer->setASTContext(context);
}

bool OperationFinderAstVisitor::VisitFunctionDecl(clang::FunctionDecl* FDecl) {
    if (!FDecl->isImplicit() && _context && !_context->getSourceManager().isInSystemHeader(FDecl->getLocation())) {
        _current_function_decl = FDecl;
    } else {
        _current_function_decl = nullptr;
    }
    return true;
}

bool OperationFinderAstVisitor::TraverseFunctionDecl(clang::FunctionDecl* FDecl) {
    bool result = RecursiveASTVisitor::TraverseFunctionDecl(FDecl);
    return result;
}

long long OperationFinderAstVisitor::_getCurrentMultiplier() const {
    long long multiplier = 1;
    for (const auto& loop : _loop_stack) {
        multiplier *= loop.iteration_count;
    }
    return multiplier;
}

bool OperationFinderAstVisitor::VisitForStmt(clang::ForStmt* stmt) {
    if (!_context) {
        return true;
    }
    if (_context->getSourceManager().isInSystemHeader(stmt->getBeginLoc())) {
        return true;
    }
    
    WhileLoopAnalysisResult analysis_result = _whileLoopAnalyzer->analyzeWhileLoop(stmt, _context);
    analysis_result.loop_type = "for";

    _op_finder->processLoop(stmt, analysis_result.loop_type,
                            analysis_result.initialization_text,
                            analysis_result.stop_condition_text,
                            analysis_result.increment_decrement_text,
                            analysis_result.initial_value,
                            analysis_result.initial_value_resolved,
                            analysis_result.stop_value,
                            analysis_result.stop_value_resolved,
                            analysis_result.comparison_operator,
                            analysis_result.increment_value,
                            analysis_result.increment_value_resolved,
                            analysis_result.iteration_count,
                            analysis_result.iteration_count_resolved,
                            _context->getSourceManager());

    if (analysis_result.iteration_count_resolved && analysis_result.iteration_count > 0) {
        _loop_stack.push_back({stmt, analysis_result.iteration_count});
    }
    return true;
}

bool OperationFinderAstVisitor::VisitDoStmt(clang::DoStmt* stmt) {
    if (!_context) {
        return true;
    }
    if (_context->getSourceManager().isInSystemHeader(stmt->getBeginLoc())) {
        return true;
    }

    _loop_stack.push_back({stmt, 1});    
    WhileLoopAnalysisResult analysis_result = _whileLoopAnalyzer->analyzeWhileLoop(stmt, _context);
    analysis_result.loop_type = "do-while";

    _op_finder->processLoop(stmt, analysis_result.loop_type,
                            analysis_result.initialization_text,
                            analysis_result.stop_condition_text,
                            analysis_result.increment_decrement_text,
                            analysis_result.initial_value,
                            analysis_result.initial_value_resolved,
                            analysis_result.stop_value,
                            analysis_result.stop_value_resolved,
                            analysis_result.comparison_operator,
                            analysis_result.increment_value,
                            analysis_result.increment_value_resolved,
                            analysis_result.iteration_count,
                            analysis_result.iteration_count_resolved,
                            _context->getSourceManager());
    
    if (analysis_result.iteration_count_resolved && analysis_result.iteration_count > 0) {
        _loop_stack.back().iteration_count = analysis_result.iteration_count;
    } else {
        _loop_stack.back().iteration_count = 1;
    }
    return true;
}

bool OperationFinderAstVisitor::VisitIfStmt(clang::IfStmt* stmt) {
    if (!_context) {
        return true;
    }    
    if (_context->getSourceManager().isInSystemHeader(stmt->getBeginLoc())) {
        return true;
    }

    std::string condition_text = stmt->getCond() ? GetSourceTextForStmt(stmt->getCond(), _context->getSourceManager(), _context->getLangOpts()) : "";
    _op_finder->processIfStatement(stmt, condition_text, _context->getSourceManager());

    _collected_gates++;
    return true;
}

bool OperationFinderAstVisitor::VisitBinaryOperator(clang::BinaryOperator* op) {
    if (!_context) {
        return true;
    }
    
    std::string op_code_str = op->getOpcodeStr().str();
    unsigned int line = _context->getSourceManager().getSpellingLineNumber(op->getBeginLoc());

    if (_context->getSourceManager().isInSystemHeader(op->getBeginLoc())) {
        return true;
    }
    
    long long multiplier = _getCurrentMultiplier();
    std::string op_type_name = op->getType().getAsString();
    bool is_main = (_current_function_decl && _current_function_decl->getNameAsString() == "main");
    
    if (_current_function_decl && !is_main) {
        _op_finder->recordUserDefinedFunctionOperator(_current_function_decl->getNameAsString(), op_code_str);
    } else {
        _op_finder->processBinaryOperator(op, op_code_str, op_type_name, multiplier, _context->getSourceManager());
    }

    _collected_gates++;
    return true;
}

bool OperationFinderAstVisitor::VisitUnaryOperator(clang::UnaryOperator* op) {
    if (!_context) {
        return true;
    }
    
    std::string op_code_str = op->getOpcodeStr(op->getOpcode()).str();
    unsigned int line = _context->getSourceManager().getSpellingLineNumber(op->getBeginLoc());

    if (_context->getSourceManager().isInSystemHeader(op->getBeginLoc())) {
        return true;
    }
    
    long long multiplier = _getCurrentMultiplier();
    std::string op_type_name = op->getSubExpr()->getType().getAsString();

    bool is_main = (_current_function_decl && _current_function_decl->getNameAsString() == "main");

    if (_current_function_decl && !is_main) {
        _op_finder->recordUserDefinedFunctionOperator(_current_function_decl->getNameAsString(), op_code_str);
    } else {
        _op_finder->processUnaryOperator(op, op_code_str, op_type_name, multiplier, _context->getSourceManager());
    }
    _collected_gates++;
    return true;
}

bool OperationFinderAstVisitor::VisitCallExpr(clang::CallExpr* call) {
    if (!_context) {
        return true;
    }
    
    std::string function_name = "<unknown_call>";
    if (const clang::FunctionDecl* FDecl = call->getDirectCallee()) {
        function_name = FDecl->getNameAsString();
    }

    unsigned int line = _context->getSourceManager().getSpellingLineNumber(call->getBeginLoc());
    if (_context->getSourceManager().isInSystemHeader(call->getBeginLoc())) {
        return true;
    }
    
    _op_finder->processFunctionCall(call, function_name, _context->getSourceManager());
    bool is_main = (_current_function_decl && _current_function_decl->getNameAsString() == "main");
    
    if (_current_function_decl && !is_main) {
        _op_finder->recordUserDefinedFunctionOperator(_current_function_decl->getNameAsString(), "function_call");
    }
    _collected_gates++;
    return true;
}

bool OperationFinderAstVisitor::VisitArraySubscriptExpr(clang::ArraySubscriptExpr* subscript) {
    if (!_context) {
        return true;
    }
    unsigned int line = _context->getSourceManager().getSpellingLineNumber(subscript->getBeginLoc());

    if (_context->getSourceManager().isInSystemHeader(subscript->getBeginLoc())) {
        return true;
    }
    
    _op_finder->processArraySubscript(subscript, _context->getSourceManager());
    bool is_main = (_current_function_decl && _current_function_decl->getNameAsString() == "main");

    if (_current_function_decl && !is_main) {
        _op_finder->recordUserDefinedFunctionOperator(_current_function_decl->getNameAsString(), "array_subscript");
    }
    _collected_gates++;
    return true;
}

bool OperationFinderAstVisitor::dataTraverseStmtPre(clang::Stmt* stmt) {
    clang::Stmt* branch_entry_stmt = _isBranchEntry(stmt);
    if (branch_entry_stmt) {
        bool already_on_stack = false;
        for (clang::Stmt* s : _branch_stack) {
            if (s == branch_entry_stmt) {
                already_on_stack = true;
                break;
            }
        }
        if (!already_on_stack) {            
            _op_finder->branchEntered();
            _branch_stack.push_back(branch_entry_stmt);
        }
    }
    return true;
}

bool OperationFinderAstVisitor::dataTraverseStmtPost(clang::Stmt* stmt) {
    if (!_loop_stack.empty() && _loop_stack.back().stmt == stmt) {
        _loop_stack.pop_back();
    }

    if (!_branch_stack.empty() && _branch_stack.back() == stmt) {
        _op_finder->branchExited();
        _branch_stack.pop_back();
    }
    return true;
}

clang::Stmt* OperationFinderAstVisitor::_isBranchEntry(clang::Stmt* stmt) {
    if (clang::isa<clang::IfStmt>(stmt) ||
        clang::isa<clang::ForStmt>(stmt) ||
        clang::isa<clang::WhileStmt>(stmt) ||
        clang::isa<clang::DoStmt>(stmt)) {
        return stmt;
    }
    return nullptr;
}

bool OperationFinderAstVisitor::VisitWhileStmt(clang::WhileStmt* stmt) {
    if (!_context) {
        return true;
    }
    if (_context->getSourceManager().isInSystemHeader(stmt->getBeginLoc())) {
        return true;
    }

    _loop_stack.push_back({stmt, 1});

    WhileLoopAnalysisResult analysis_result = _whileLoopAnalyzer->analyzeWhileLoop(stmt, _context);
    analysis_result.loop_type = "while";

    _op_finder->processLoop(stmt, analysis_result.loop_type,
                            analysis_result.initialization_text,
                            analysis_result.stop_condition_text,
                            analysis_result.increment_decrement_text,
                            analysis_result.initial_value,
                            analysis_result.initial_value_resolved,
                            analysis_result.stop_value,
                            analysis_result.stop_value_resolved,
                            analysis_result.comparison_operator,
                            analysis_result.increment_value,
                            analysis_result.increment_value_resolved,
                            analysis_result.iteration_count,
                            analysis_result.iteration_count_resolved,
                            _context->getSourceManager());
    
    if (analysis_result.iteration_count_resolved && analysis_result.iteration_count > 0) {
        _loop_stack.back().iteration_count = analysis_result.iteration_count;
    } else {
        _loop_stack.back().iteration_count = 1;
    }
    return true;
}
