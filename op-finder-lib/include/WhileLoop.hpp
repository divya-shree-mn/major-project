#ifndef C_ANALYZER_WHILELOOP_HPP
#define C_ANALYZER_WHILELOOP_HPP

#include <string>
#include <optional>
#include <utility>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Decl.h"

namespace clang {
    class ASTContext;
    class WhileStmt;
    class ForStmt;
    class DoStmt;
    class Expr;
    class Stmt;
    class VarDecl;
    class BinaryOperator;
}

struct WhileLoopAnalysisResult {
    bool analysis_resolved = false;
    std::string initialization_text;
    std::string stop_condition_text;
    std::string increment_decrement_text;
    long long initial_value = 0;
    bool initial_value_resolved = false;
    long long stop_value = 0;
    bool stop_value_resolved = false;
    std::string comparison_operator = "";
    long long increment_value = 0;
    bool increment_value_resolved = false;
    long long iteration_count = -1;
    bool iteration_count_resolved = false;
    std::string loop_type;
};

class WhileLoopIterationAnalyzer {
public:
    WhileLoopIterationAnalyzer(clang::ASTContext* context = nullptr) : _context(context) {}
    void setASTContext(clang::ASTContext* context) {
        _context = context;
    }

    bool isDeclRefExprToVar(const clang::Expr* E, const clang::VarDecl* V);
    std::optional<long long> evaluateAsConstant(const clang::Expr* expr);
    std::string getComparisonOperatorString(const clang::BinaryOperator* bin_op);

    const clang::Stmt*
    findPrecedingStmt(const clang::Stmt* target_stmt);

    WhileLoopAnalysisResult analyzeWhileLoop(const clang::Stmt* loop_stmt);    
    WhileLoopAnalysisResult analyzeWhileLoop( const clang::Stmt* loop_stmt, clang::ASTContext* context);

private:
    clang::ASTContext* _context;
    
    const clang::VarDecl*
    findLoopControlVariable(const clang::Expr* condition_expr, clang::ASTContext* context);

    std::pair<long long, bool>
    getInitialValue(const clang::Stmt* init_stmt, const clang::VarDecl* loop_var, clang::ASTContext* context);

    std::pair<long long, bool>
    getStepValueAndDirection(const clang::Stmt* loop_body, const clang::VarDecl* loop_var, clang::ASTContext* context);

    const clang::VarDecl*
    findLoopControlVariable(const clang::Expr* condition_expr);

    std::pair<long long, bool>
    getInitialValue(const clang::Stmt* init_stmt, const clang::VarDecl* loop_var);

    std::pair<long long, bool>
    getStepValueAndDirection(const clang::Stmt* loop_body, const clang::VarDecl* loop_var);

    std::string _getSourceText(const clang::Stmt* stmt);
};

#endif // C_ANALYZER_WHILELOOP_HPP
