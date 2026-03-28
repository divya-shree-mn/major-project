#ifndef C_ANALYZER_OPERATIONFINDERASTCONSUMER_HPP
#define C_ANALYZER_OPERATIONFINDERASTCONSUMER_HPP

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>

#include "OperationFinderAstVisitor.hpp"
#include "OperatorCounter.hpp"

class OperationFinder;

class OperationFinderAstConsumer : public clang::ASTConsumer
{
public:

  explicit OperationFinderAstConsumer(OperationFinder* op_finder, OperatorCounter* op_counter);
  void Initialize(clang::ASTContext& context) override;
  void HandleTranslationUnit(clang::ASTContext& context) override;

private:

  OperationFinderAstVisitor _visitor;

};
#endif //C_ANALYZER_OPERATIONFINDERASTCONSUMER_HPP
