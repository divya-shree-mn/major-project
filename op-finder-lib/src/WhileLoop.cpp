#include "WhileLoop.hpp"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"

#include <set>
#include <string>
#include <algorithm>
#include <vector>
#include <cmath>
#include <utility>
#include <optional>

namespace {
    std::string GetSourceText(const clang::Stmt* S, const clang::SourceManager& SM, const clang::LangOptions& LO) {
        if (!S) return "";
        clang::SourceLocation StartLoc = S->getBeginLoc();
        clang::SourceLocation EndLoc = S->getEndLoc();

        if (StartLoc.isMacroID() || EndLoc.isMacroID()) {
            StartLoc = SM.getSpellingLoc(StartLoc);
            EndLoc = SM.getSpellingLoc(EndLoc);

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

class VarDeclRefFinder : public clang::RecursiveASTVisitor<VarDeclRefFinder> {
public:
    VarDeclRefFinder(const clang::VarDecl* targetVar, const clang::VarDecl*& foundVar)
        : TargetVar(targetVar), FoundVar(foundVar) {}

    bool VisitDeclRefExpr(clang::DeclRefExpr* DRE) {
        if (const clang::VarDecl* VD = clang::dyn_cast<clang::VarDecl>(DRE->getDecl())) {
            if (VD == TargetVar) {
                FoundVar = VD;
                return false;
            }
        }
        return true;
    }
private:
    const clang::VarDecl* TargetVar;
    const clang::VarDecl*& FoundVar;
};

class VarModifierFinder : public clang::RecursiveASTVisitor<VarModifierFinder> {
public:
    VarModifierFinder(const clang::VarDecl* targetVar, long long& stepVal, bool& resolved, clang::ASTContext* Ctx, WhileLoopIterationAnalyzer& analyzer)
        : TargetVar(targetVar), StepValue(stepVal), Resolved(resolved), Ctx(Ctx), Analyzer(analyzer) {}

    bool VisitUnaryOperator(clang::UnaryOperator* UO) {
        if ((UO->isIncrementOp() || UO->isDecrementOp()) && Analyzer.isDeclRefExprToVar(UO->getSubExpr(), TargetVar)) {
            StepValue = (UO->isIncrementOp() ? 1 : -1);
            Resolved = true;
            return false;
        }
        return true;
    }

    bool VisitBinaryOperator(clang::BinaryOperator* BO) {
        if (BO->isAssignmentOp() && Analyzer.isDeclRefExprToVar(BO->getLHS(), TargetVar)) {
            if (BO->getOpcode() == clang::BO_AddAssign || BO->getOpcode() == clang::BO_SubAssign) {
                clang::Expr::EvalResult StepExprResult;
                if (BO->getRHS()->EvaluateAsInt(StepExprResult, *Ctx)) {
                    StepValue = StepExprResult.Val.getInt().getZExtValue();
                    if (BO->getOpcode() == clang::BO_SubAssign) {
                        StepValue = -StepValue;
                    }
                    Resolved = true;
                    return false;
                }
            }
            if (const clang::BinaryOperator* RHS_BO = clang::dyn_cast<clang::BinaryOperator>(BO->getRHS()->IgnoreImplicitAsWritten())) {
                if ((RHS_BO->getOpcode() == clang::BO_Add || RHS_BO->getOpcode() == clang::BO_Sub)) {
                    const clang::Expr* other_operand = nullptr;
                    if (Analyzer.isDeclRefExprToVar(RHS_BO->getLHS(), TargetVar)) {
                        other_operand = RHS_BO->getRHS();
                    } else if (Analyzer.isDeclRefExprToVar(RHS_BO->getRHS(), TargetVar)) {
                        other_operand = RHS_BO->getLHS();
                    }

                    if (other_operand) {
                        clang::Expr::EvalResult StepExprResult;
                        if (other_operand->EvaluateAsInt(StepExprResult, *Ctx)) {
                            StepValue = StepExprResult.Val.getInt().getZExtValue();
                            if (RHS_BO->getOpcode() == clang::BO_Sub) {
                                StepValue = -StepValue;
                            }
                            Resolved = true;
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }

private:
    const clang::VarDecl* TargetVar;
    long long& StepValue;
    bool& Resolved;
    clang::ASTContext* Ctx;
    WhileLoopIterationAnalyzer& Analyzer;
};

class ParentCompoundStmtAndPredecessorFinder : public clang::RecursiveASTVisitor<ParentCompoundStmtAndPredecessorFinder> {
public:
    ParentCompoundStmtAndPredecessorFinder(const clang::Stmt* targetStmt, const clang::Stmt*& foundPredecessor, const clang::CompoundStmt*& foundParentCS)
        : TargetStmt(targetStmt), FoundPredecessor(foundPredecessor), FoundParentCS(foundParentCS) {}

    bool VisitCompoundStmt(clang::CompoundStmt* CS) {
        const clang::Stmt* currentPredecessor = nullptr;
        bool foundTarget = false;
        for (const clang::Stmt* S : CS->body()) {
            if (S == TargetStmt) {
                foundTarget = true;
                break;
            }
            currentPredecessor = S;
        }

        if (foundTarget) {
            FoundPredecessor = currentPredecessor;
            FoundParentCS = CS;
            return false;
        }
        return true;
    }

private:
    const clang::Stmt* TargetStmt;
    const clang::Stmt*& FoundPredecessor;
    const clang::CompoundStmt*& FoundParentCS;
};


bool WhileLoopIterationAnalyzer::isDeclRefExprToVar(const clang::Expr* E, const clang::VarDecl* V) {
    if (!E || !V) return false;
    if (const clang::DeclRefExpr* DRE = clang::dyn_cast<clang::DeclRefExpr>(E->IgnoreImplicitAsWritten())) {
        return DRE->getDecl() == V;
    }
    return false;
}

WhileLoopAnalysisResult WhileLoopIterationAnalyzer::analyzeWhileLoop(
    const clang::Stmt* loop_stmt,
    clang::ASTContext* context) {
    
    _context = context;

    WhileLoopAnalysisResult result;
    if (!_context || !loop_stmt) {
        return result;
    }

    const clang::Expr* condition = nullptr;
    const clang::Stmt* loop_body = nullptr;
    const clang::Stmt* init_stmt = nullptr;
    const clang::Expr* inc_expr = nullptr;

    if (const clang::WhileStmt* while_stmt = clang::dyn_cast<clang::WhileStmt>(loop_stmt)) {
        condition = while_stmt->getCond();
        loop_body = while_stmt->getBody();
        result.loop_type = "while";
        init_stmt = findPrecedingStmt(while_stmt);
    } else if (const clang::ForStmt* for_stmt = clang::dyn_cast<clang::ForStmt>(loop_stmt)) {
        condition = for_stmt->getCond();
        loop_body = for_stmt->getBody();
        init_stmt = for_stmt->getInit();
        inc_expr = for_stmt->getInc();
        result.loop_type = "for";
    } else if (const clang::DoStmt* do_stmt = clang::dyn_cast<clang::DoStmt>(loop_stmt)) {
        condition = do_stmt->getCond();
        loop_body = do_stmt->getBody();
        result.loop_type = "do-while";
        init_stmt = findPrecedingStmt(do_stmt);
    } else {
        return result;
    }

    if (!condition) {
        return result;
    }
    result.stop_condition_text = _getSourceText(const_cast<clang::Expr*>(condition));

    const clang::VarDecl* loop_var = findLoopControlVariable(condition);
    if (!loop_var) {
        return result;
    }

    if (init_stmt) {
        result.initialization_text = _getSourceText(init_stmt);
    } else {
        result.initialization_text = "N/A";
    }

    auto [initial_value, initial_value_resolved] = getInitialValue(init_stmt, loop_var);
    result.initial_value = initial_value;
    result.initial_value_resolved = initial_value_resolved;

    if (!initial_value_resolved && result.loop_type != "for") {
        return result;
    }

    if (const clang::BinaryOperator* bin_op = clang::dyn_cast<clang::BinaryOperator>(condition->IgnoreImplicitAsWritten())) {
        const clang::Expr* lhs = bin_op->getLHS();
        const clang::Expr* rhs = bin_op->getRHS();

        if (isDeclRefExprToVar(lhs, loop_var)) {
            if (auto val = evaluateAsConstant(rhs)) {
                result.stop_value = *val;
                result.stop_value_resolved = true;
            }
        } else if (isDeclRefExprToVar(rhs, loop_var)) {
            if (auto val = evaluateAsConstant(lhs)) {
                result.stop_value = *val;
                result.stop_value_resolved = true;
            }
        }
        result.comparison_operator = getComparisonOperatorString(bin_op);
    }

    if (!result.stop_value_resolved) {
        return result;
    }

    const clang::Stmt* stmt_for_step_value_analysis = loop_body;
    if (result.loop_type == "for" && inc_expr) {
        stmt_for_step_value_analysis = inc_expr;
        result.increment_decrement_text = _getSourceText(inc_expr);
    } else if (loop_body) {
        result.increment_decrement_text = _getSourceText(loop_body);
    }

    if (stmt_for_step_value_analysis) {
        auto [step_val, step_resolved] = getStepValueAndDirection(stmt_for_step_value_analysis, loop_var);
        result.increment_value = step_val;
        result.increment_value_resolved = step_resolved;

        if (result.increment_value_resolved) {
            if (result.increment_value > 0) {
                result.increment_decrement_text = "increment by " + std::to_string(result.increment_value);
            } else if (result.increment_value < 0) {
                result.increment_decrement_text = "decrement by " + std::to_string(std::abs(result.increment_value));
            } else {
                result.increment_decrement_text = "no change (possibly infinite loop)";
            }
        }
    }

    if (!result.increment_value_resolved || result.increment_value == 0) {
        result.analysis_resolved = false;
        result.iteration_count_resolved = false;
        return result;
    }

    if (result.initial_value_resolved && result.stop_value_resolved && result.increment_value_resolved && result.increment_value != 0) {
        long long current_iteration_count = 0;
        bool can_calculate_iterations = false;

        if (result.increment_value > 0) {
            if (result.initial_value < result.stop_value) {
                if (result.comparison_operator == "<" || result.comparison_operator == "!=") {
                    current_iteration_count = (result.stop_value - result.initial_value + result.increment_value - 1) / result.increment_value;
                    can_calculate_iterations = true;
                } else if (result.comparison_operator == "<=") {
                    current_iteration_count = (result.stop_value - result.initial_value) / result.increment_value + 1;
                    can_calculate_iterations = true;
                }
            } else {
                current_iteration_count = 0;
                can_calculate_iterations = true;
            }
        } else {
            if (result.initial_value > result.stop_value) {
                if (result.comparison_operator == ">" || result.comparison_operator == "!=") {
                    current_iteration_count = (result.initial_value - result.stop_value + std::abs(result.increment_value) - 1) / std::abs(result.increment_value);
                    can_calculate_iterations = true;
                } else if (result.comparison_operator == ">=") {
                    current_iteration_count = (result.initial_value - result.stop_value) / std::abs(result.increment_value) + 1;
                    can_calculate_iterations = true;
                }
            } else {
                current_iteration_count = 0;
                can_calculate_iterations = true;
            }
        }

        result.iteration_count = current_iteration_count;
        result.iteration_count_resolved = can_calculate_iterations;
    } else {
        result.iteration_count = -1;
        result.iteration_count_resolved = false;
    }

    if (loop_var && result.stop_value_resolved) {
        result.stop_condition_text = loop_var->getNameAsString() + " " + result.comparison_operator + " " + std::to_string(result.stop_value);
    }

    result.analysis_resolved = result.initial_value_resolved &&
                               result.stop_value_resolved &&
                               result.increment_value_resolved &&
                               result.increment_value != 0 &&
                               result.iteration_count_resolved;

    return result;
}

std::string WhileLoopIterationAnalyzer::_getSourceText(const clang::Stmt* stmt) {
    if (!stmt || !_context) return "";
    return GetSourceText(stmt, _context->getSourceManager(), _context->getLangOpts());
}

const clang::VarDecl* WhileLoopIterationAnalyzer::findLoopControlVariable(const clang::Expr* condition_expr) {
    if (!condition_expr || !_context) return nullptr;
    std::set<const clang::VarDecl*> candidate_vars;
    class CandidateVarFinder : public clang::RecursiveASTVisitor<CandidateVarFinder> {
    public:
        CandidateVarFinder(std::set<const clang::VarDecl*>& vars) : Candidates(vars) {}
        bool VisitDeclRefExpr(clang::DeclRefExpr* DRE) {
            if (const clang::VarDecl* VD = clang::dyn_cast<clang::VarDecl>(DRE->getDecl())) {
                Candidates.insert(VD);
            }
            return true;
        }
    private:
        std::set<const clang::VarDecl*>& Candidates;
    };
    CandidateVarFinder finder(candidate_vars);
    finder.TraverseStmt(const_cast<clang::Expr*>(condition_expr));

    if (candidate_vars.size() == 1) {
        return *candidate_vars.begin();
    }
    return nullptr;
}

const clang::Stmt* WhileLoopIterationAnalyzer::findPrecedingStmt(const clang::Stmt* target_stmt) {
    if (!_context) return nullptr;
    const clang::Stmt* found_predecessor = nullptr;
    const clang::CompoundStmt* found_parent_cs = nullptr;
    ParentCompoundStmtAndPredecessorFinder finder(target_stmt, found_predecessor, found_parent_cs);
    finder.TraverseDecl(_context->getTranslationUnitDecl());
    return found_predecessor;
}

std::pair<long long, bool> WhileLoopIterationAnalyzer::getInitialValue(const clang::Stmt* init_stmt, const clang::VarDecl* loop_var) {
    long long value = 0;
    bool resolved = false;
    if (!init_stmt || !loop_var || !_context) {
        return {value, resolved};
    }
    if (const clang::DeclStmt* DS = clang::dyn_cast<clang::DeclStmt>(init_stmt)) {
        if (DS->isSingleDecl()) {
            if (const clang::VarDecl* VD = clang::dyn_cast<clang::VarDecl>(DS->getSingleDecl())) {
                if (VD == loop_var && VD->hasInit()) {
                    if (auto val = evaluateAsConstant(VD->getInit())) {
                        value = *val;
                        resolved = true;
                    }
                }
            }
        }
    } else if (const clang::BinaryOperator* BO = clang::dyn_cast<clang::BinaryOperator>(init_stmt)) {
        if (BO->isAssignmentOp() && isDeclRefExprToVar(BO->getLHS(), loop_var)) {
            if (auto val = evaluateAsConstant(BO->getRHS())) {
                value = *val;
                resolved = true;
            }
        }
    }
    return {value, resolved};
}

std::pair<long long, bool> WhileLoopIterationAnalyzer::getStepValueAndDirection(const clang::Stmt* loop_body, const clang::VarDecl* loop_var) {
    long long step_value = 0;
    bool resolved = false;
    if (!loop_body || !loop_var || !_context) return {step_value, resolved};
    VarModifierFinder finder(loop_var, step_value, resolved, _context, *this);
    finder.TraverseStmt(const_cast<clang::Stmt*>(loop_body));
    return {step_value, resolved};
}

std::optional<long long> WhileLoopIterationAnalyzer::evaluateAsConstant(const clang::Expr* expr) {
    if (!expr || !_context) return std::nullopt;
    clang::Expr::EvalResult Result;
    if (expr->EvaluateAsInt(Result, *_context)) {
        return Result.Val.getInt().getZExtValue();
    }
    return std::nullopt;
}

std::string WhileLoopIterationAnalyzer::getComparisonOperatorString(const clang::BinaryOperator* bin_op) {
    if (!bin_op) return "";
    switch (bin_op->getOpcode()) {
        case clang::BO_LT: return "<";
        case clang::BO_LE: return "<=";
        case clang::BO_GT: return ">";
        case clang::BO_GE: return ">=";
        case clang::BO_EQ: return "==";
        case clang::BO_NE: return "!=";
        default: return "";
    }
}
