#include "OperationFinderAstAction.hpp"
#include "OperationFinderAstConsumer.hpp"

OperationFinderAstAction::OperationFinderAstAction(OperationFinder* op_finder, OperatorCounter* op_counter)
  : _op_finder(op_finder)
  , _op_counter(op_counter)
{ }

std::unique_ptr<clang::ASTConsumer> OperationFinderAstAction::newASTConsumer()
{
  return std::unique_ptr<clang::ASTConsumer>(
          new OperationFinderAstConsumer(_op_finder, _op_counter));
}