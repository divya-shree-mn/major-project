#ifndef OPERATION_FINDER_AST_VISITOR_HPP
#define OPERATION_FINDER_AST_VISITOR_HPP

#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "OperationFinder.hpp"
#include "OperatorCounter.hpp"
#include "WhileLoop.hpp"

#include <vector>
#include <string>
#include <utility>
#include <memory>
#include <map>

namespace clang {
    class Stmt;
    class SourceManager;
    class LangOptions;
    class FunctionDecl;
    class VarDecl;
    class Expr;
    class BinaryOperator;
    class UnaryOperator;
    class CallExpr;
    class ArraySubscriptExpr;
    class IfStmt;
    class ForStmt;
    class WhileStmt;
    class DoStmt;
}

struct AstVisitorLoopContext {
    clang::Stmt* stmt;
    long long iteration_count;
};

struct LoopHeaderInfo {
    bool in_loop_header = false;
    clang::Stmt* init = nullptr;
    clang::Stmt* header_start = nullptr;
    clang::Stmt* header_end = nullptr;
};

class OperationFinderAstVisitor : public clang::RecursiveASTVisitor<OperationFinderAstVisitor> {
public:
    OperationFinderAstVisitor(OperationFinder* op_finder, OperatorCounter* op_counter);

    void NewContext(clang::ASTContext* context);
    bool VisitFunctionDecl(clang::FunctionDecl* FDecl);
    bool TraverseFunctionDecl(clang::FunctionDecl* FDecl);

    bool VisitForStmt(clang::ForStmt* stmt);
    bool VisitWhileStmt(clang::WhileStmt* stmt);
    bool VisitDoStmt(clang::DoStmt* stmt);
    bool VisitIfStmt(clang::IfStmt* stmt);
    bool VisitBinaryOperator(clang::BinaryOperator* op);
    bool VisitUnaryOperator(clang::UnaryOperator* op);
    bool VisitCallExpr(clang::CallExpr* call);
    bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr* subscript);

    bool dataTraverseStmtPre(clang::Stmt* S);
    bool dataTraverseStmtPost(clang::Stmt* S);

    unsigned long long getCollectedGateCount() const { return _collected_gates; }
    void resetCollectedGateCount() { _collected_gates = 0; }

    OperatorCounter* getOperatorCounter() const { return _op_counter; }

    const std::map<std::string, std::vector<std::string>>& getUserDefinedFunctionOperators() const {
        return _op_finder->getUserDefinedFunctionOperators();
    }

    IOperationOutput* getStorage() const { return _op_finder->getStorage(); }

private:
    bool _isStmtInCurrentFunction(const clang::Stmt* stmt);
    std::string _getSourceText(const clang::Stmt* stmt);
    clang::Stmt* _isBranchEntry(clang::Stmt* stmt);    
    long long _getCurrentMultiplier() const;

    clang::ASTContext* _context;
    OperationFinder* _op_finder;
    OperatorCounter* _op_counter;
    std::unique_ptr<WhileLoopIterationAnalyzer> _whileLoopAnalyzer;
    LoopHeaderInfo _loop_header;
    std::vector<clang::Stmt*> _branch_stack;

    const clang::FunctionDecl* _current_function_decl;
    unsigned long long _collected_gates;
    const clang::VarDecl* _findLoopVarInCondition(const clang::Expr* cond_expr);
    
    void _getLoopInitialValue(const clang::Stmt* init_stmt, const clang::VarDecl* loop_var, long long& value, bool& resolved);
    void _getLoopStopValueAndComparison(const clang::Expr* cond_expr, const clang::VarDecl* loop_var, long long& value, bool& resolved, std::string& op_str);
    void _getLoopIncrementDecrementValue(const clang::Stmt* stmt, const clang::VarDecl* loop_var, long long& value, bool& resolved);
    
    std::vector<AstVisitorLoopContext> _loop_stack;
};

#endif // OPERATION_FINDER_AST_VISITOR_HPP
