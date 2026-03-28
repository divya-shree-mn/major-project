#ifndef C_ANALYZER_ASTHELPERS_HPP
#define C_ANALYZER_ASTHELPERS_HPP

#include <clang/AST/ASTContext.h>

template<typename TOp>
clang::StringRef getOpcode(const TOp *op)
{
  return op->getOpcodeStr(op->getOpcode());
}

inline clang::SourceLocation resolveOperationSourceLocation(const clang::SourceManager& source_manager, const clang::SourceLocation& original)
{
  if (source_manager.isMacroBodyExpansion(original))
  {
    return source_manager.getExpansionLoc(original);
  }

  return original;
}

inline std::tuple<std::string, unsigned int, unsigned int> resolveLocationsWithLoc(const clang::SourceLocation& loc, const clang::SourceManager& source_manager)
{
  const auto& loc_resolved = resolveOperationSourceLocation(source_manager, loc);

  return {
          source_manager.getFilename(loc_resolved).str(),
          source_manager.getSpellingLineNumber(loc_resolved),
          source_manager.getSpellingColumnNumber(loc_resolved)
  };
}

template<typename TStmt>
std::tuple<std::string, unsigned int, unsigned int> resolveLocations(const TStmt* op, const clang::SourceManager& source_manager)
{
  const auto& loc = resolveOperationSourceLocation(source_manager, op->getBeginLoc());

  return {
          source_manager.getFilename(loc).str(),
          source_manager.getSpellingLineNumber(loc),
          source_manager.getSpellingColumnNumber(loc)
  };
}

#endif //C_ANALYZER_ASTHELPERS_HPP
