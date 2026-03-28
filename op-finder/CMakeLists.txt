cmake_minimum_required(VERSION 3.14)

# LLVM is typically compiled without RTTI. To avoid linker errors related to 'typeinfo',
# ensure your code is also compiled without RTTI.
# The previous block modifying CMAKE_CXX_FLAGS is removed.
# Instead, we apply -fno-rtti directly to the target's compile options.

add_executable(op-finder
    main.cpp)

# Apply the -fno-rtti flag directly to this executable target
# For MSVC, this would be /GR-
target_compile_options(op-finder PRIVATE -fno-rtti)

target_link_libraries(op-finder
    PUBLIC
        op-finder-lib)
