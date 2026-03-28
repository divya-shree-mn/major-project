cmake_minimum_required(VERSION 3.14)

find_package(nlohmann_json REQUIRED)
include("/home/divya/llvm-install/lib/cmake/llvm/LLVMConfig.cmake")
include("/home/divya/llvm-install/lib/cmake/clang/ClangConfig.cmake")
# find_package(Clang REQUIRED)
# find_package(LLVM REQUIRED COMPONENTS Support Option Core)

# LLVM is typically compiled without RTTI. To avoid linker errors related to 'typeinfo',
# ensure your code is also compiled without RTTI.
# The previous block modifying CMAKE_CXX_FLAGS is removed.
# Instead, we apply -fno-rtti directly to the target's compile options.

llvm_map_components_to_libnames(llvm_libs support option core)

add_library(op-finder-lib STATIC
    src/OperationFinder.cpp
    src/OperationStorage.cpp
    src/OperationFinderAstVisitor.cpp
    src/OperationFinderAstConsumer.cpp
    src/OperationFinderAstAction.cpp
    src/OperatorCounter.cpp
    src/WhileLoop.cpp
    src/OperationLog.cpp)

# Apply the -fno-rtti flag directly to this target
# For MSVC, this would be /GR-
target_compile_options(op-finder-lib PRIVATE -fno-rtti)

target_include_directories(op-finder-lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LLVM_INCLUDE_DIRS}
)

target_compile_definitions(op-finder-lib
    PUBLIC
        ${LLVM_DEFINITIONS}
)

target_link_libraries(op-finder-lib
    PUBLIC
        nlohmann_json::nlohmann_json
    PRIVATE
        ${llvm_libs}
        clangTooling
        clangBasic
        clangASTMatchers
)
