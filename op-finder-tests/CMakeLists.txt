cmake_minimum_required(VERSION 3.14)

find_package(Catch2 REQUIRED)

# LLVM is typically compiled without RTTI. To avoid linker errors related to 'typeinfo',
# ensure your code is also compiled without RTTI.
# The previous block modifying CMAKE_CXX_FLAGS is removed.
# Instead, we apply -fno-rtti directly to the target's compile options.

add_executable(op-finder-tests
    main.cpp
    fixtures/RunOnCodeFixture.cpp
    basic_operations.cpp
    branches.cpp
    function_calls.cpp)

# Apply the -fno-rtti flag directly to this executable target
# For MSVC, this would be /GR-
target_compile_options(op-finder-tests PRIVATE -fno-rtti)

target_link_libraries(op-finder-tests
    PUBLIC
        op-finder-lib
        Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(op-finder-tests)
