#ifndef C_ANALYZER_OPERATIONFINDERASTACTION_HPP
#define C_ANALYZER_OPERATIONFINDERASTACTION_HPP

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include "OperatorCounter.hpp"

class OperationFinder;
class OperatorCounter;

class OperationFinderAstAction
{
public:

  explicit OperationFinderAstAction(OperationFinder* op_finder, OperatorCounter* op_counter);
  std::unique_ptr<clang::ASTConsumer> newASTConsumer();
  
private:
  OperationFinder* _op_finder;
  OperatorCounter* _op_counter;
};

#endif //C_ANALYZER_OPERATIONFINDERASTACTION_HPP
