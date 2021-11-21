cmake_minimum_required(VERSION 3.12)
project(fixed_containers)

set(CMAKE_CXX_STANDARD 20)
enable_testing()

#set(CMAKE_VERBOSE_MAKEFILE ON)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(USING_CLANG ON)
else()
    set(USING_CLANG OFF)
endif()

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Werror")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fno-exceptions")

if(${USING_CLANG})
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -ftrivial-auto-var-init=pattern")
endif()

SET(DIAGNOSTIC_FLAGS "")
if(${USING_CLANG})
    SET(DIAGNOSTIC_FLAGS ${DIAGNOSTIC_FLAGS}
        # Only available in clang
        -Weverything
        # Disables C++98 to C++17 compatibility enforcement
        -Wno-c++98-compat-pedantic
        # Re-enable, as it is disabled by the previous one
        -Wc++98-compat-extra-semi

        # Has false positives
        # https://bugs.llvm.org/show_bug.cgi?id=18733
        # https://stackoverflow.com/questions/56041900/why-does-explicit-template-instantiation-result-in-weak-template-vtables-warning
        -Wno-weak-template-vtables
        #
        -Wno-global-constructors

        # due to gtest
        -Wno-exit-time-destructors
        -Wno-used-but-marked-unused
    )
endif()


SET(DIAGNOSTIC_FLAGS ${DIAGNOSTIC_FLAGS}
        -Wall
        -Wextra
        -Wpedantic
        # This is failing on the `consteval` keyword for some reason
        -Wno-c++20-compat
        # Need stdlib uprev, as even std::vector triggers this
        -Wno-ctad-maybe-unsupported

        -Wno-padded
        )
string (REPLACE ";" " " DIAGNOSTIC_FLAGS "${DIAGNOSTIC_FLAGS}")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${DIAGNOSTIC_FLAGS}")

find_package(GTest CONFIG REQUIRED)

macro(add_test_dependencies TEST_TARGET)
    target_link_libraries(${TEST_TARGET} GTest::gtest GTest::gtest_main)
    target_include_directories(${TEST_TARGET} PRIVATE include)
    add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
endmacro()

add_executable(concepts_test test/concepts_test.cpp)
add_test_dependencies(concepts_test)
add_executable(enum_map_test test/enum_map_test.cpp)
add_test_dependencies(enum_map_test)
add_executable(enum_set_test test/enum_set_test.cpp)
add_test_dependencies(enum_set_test)
add_executable(enum_utils_test test/enum_utils_test.cpp)
add_test_dependencies(enum_utils_test)
add_executable(fixed_map_test test/fixed_map_test.cpp)
add_test_dependencies(fixed_map_test)
add_executable(fixed_red_black_tree_test test/fixed_red_black_tree_test.cpp)
add_test_dependencies(fixed_red_black_tree_test)
add_executable(fixed_set_test test/fixed_set_test.cpp)
add_test_dependencies(fixed_set_test)
add_executable(fixed_vector_test test/fixed_vector_test.cpp)
add_test_dependencies(fixed_vector_test)
add_executable(index_range_predicate_iterator_test test/index_range_predicate_iterator_test.cpp)
add_test_dependencies(index_range_predicate_iterator_test)
add_executable(pair_view_test test/pair_view_test.cpp)
add_test_dependencies(pair_view_test)
add_executable(string_literal_test test/string_literal_test.cpp)
add_test_dependencies(string_literal_test)
add_executable(type_name_test test/type_name_test.cpp)
add_test_dependencies(type_name_test)
