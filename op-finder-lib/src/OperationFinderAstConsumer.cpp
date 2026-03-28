#include "OperationFinderAstConsumer.hpp"

OperationFinderAstConsumer::OperationFinderAstConsumer(OperationFinder* op_finder, OperatorCounter* op_counter)
  : _visitor(op_finder, op_counter)
{ }

void OperationFinderAstConsumer::Initialize(clang::ASTContext& context)
{
  _visitor.NewContext(&context);
}

void OperationFinderAstConsumer::HandleTranslationUnit(clang::ASTContext& context)
{
  _visitor.TraverseAST(context);
}