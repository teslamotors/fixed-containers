cmake_minimum_required(VERSION 3.12)
project(fixed_containers)

set(CMAKE_CXX_STANDARD 20)
enable_testing()

#set(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Werror")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fno-exceptions -ftrivial-auto-var-init=pattern")

SET(DIAGNOSTIC_FLAGS
        -Weverything
        # Disables C++98 to C++17 compatibility enforcement
        -Wno-c++98-compat-pedantic
        # Re-enable, as it is disabled by the previous one
        -Wc++98-compat-extra-semi
        # We specifically don't want std::move on return due to RVO
        -Wno-return-std-move-in-c++11
        # This is failing on the `consteval` keyword for some reason
        -Wno-c++20-compat
        # Need stdlib uprev, as even std::vector triggers this
        -Wno-ctad-maybe-unsupported
        # Has false positives
        # https://bugs.llvm.org/show_bug.cgi?id=18733
        # https://stackoverflow.com/questions/56041900/why-does-explicit-template-instantiation-result-in-weak-template-vtables-warning
        -Wno-weak-template-vtables
        #
        -Wno-global-constructors
        -Wno-padded
        -Wno-sign-conversion

        # due to gtest
        -Wno-covered-switch-default
        -Wno-exit-time-destructors
        -Wno-used-but-marked-unused
        )
string (REPLACE ";" " " DIAGNOSTIC_FLAGS "${DIAGNOSTIC_FLAGS}")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${DIAGNOSTIC_FLAGS}")


find_package(GTest CONFIG REQUIRED)

macro(add_test_dependencies TEST_TARGET)
    target_link_libraries(${TEST_TARGET} GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
    target_include_directories(${TEST_TARGET} PRIVATE .)
    add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
endmacro()

