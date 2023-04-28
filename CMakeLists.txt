cmake_minimum_required(VERSION 3.16)
#if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
#  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
#endif()
#if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
#  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
#endif()

set(CMAKE_CXX_STANDARD 20)
# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)


project(fixed_containers LANGUAGES CXX)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(USING_CLANG ON)
else()
    set(USING_CLANG OFF)
endif()


add_library(project_options INTERFACE)
if(MSVC)
    target_compile_options(project_options INTERFACE /WX /EHs-)

    target_compile_options(project_options INTERFACE /wd4804 /wd4018)  # due to magic-enum, mixed arithmetic
    target_compile_options(project_options INTERFACE /wd4530)  # due to ostream, exception handler
else()
    target_compile_options(project_options INTERFACE -Werror -fno-exceptions -ftemplate-backtrace-limit=0)
endif()
if(${USING_CLANG})
    target_compile_options(project_options INTERFACE -ftrivial-auto-var-init=pattern)
endif()
target_compile_features(project_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})


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
        -Wno-covered-switch-default # v1.11.0 fails (needs new release). bazel points to a newer commit with the fix.
        -Wno-exit-time-destructors
        -Wno-used-but-marked-unused

        # This is failing on the `consteval` keyword for some reason
        -Wno-c++20-compat
        # Need stdlib uprev, as even std::vector triggers this
        -Wno-ctad-maybe-unsupported

        -Wno-padded
        -Wno-poison-system-directories
    )
elseif(MSVC)
    SET(DIAGNOSTIC_FLAGS ${DIAGNOSTIC_FLAGS}
        /W4
    )
else()
    SET(DIAGNOSTIC_FLAGS ${DIAGNOSTIC_FLAGS}
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE ${DIAGNOSTIC_FLAGS})


find_package(magic_enum CONFIG REQUIRED)

add_library(fixed_containers INTERFACE)
add_library(fixed_containers::fixed_containers ALIAS fixed_containers)
target_include_directories(fixed_containers INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(fixed_containers INTERFACE magic_enum::magic_enum)


# TODO: split library in components like in bazel
#add_library(fixed_containers_bidirectional_iterator INTERFACE include/fixed_containers/bidirectional_iterator.hpp)
#add_library(fixed_containers::bidirectional_iterator ALIAS fixed_containers_bidirectional_iterator)
#target_include_directories(fixed_containers_bidirectional_iterator INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)


option(BUILD_TESTS "Enable Tests" ${PROJECT_IS_TOP_LEVEL})
if(BUILD_TESTS)
    # This variable is set by project() in CMake 3.21+
    string(COMPARE EQUAL
            "${CMAKE_SOURCE_DIR}"
            "${PROJECT_SOURCE_DIR}"
            PROJECT_IS_TOP_LEVEL)
    if(PROJECT_IS_TOP_LEVEL)
        # Consider the CTest module, which creates targets and options!
        # Only needed if you want to enable submissions to a CDash server.
        include(CTest)
    endif()
    enable_testing()

    find_package(range-v3 CONFIG REQUIRED)
    find_package(GTest CONFIG REQUIRED)

    macro(add_test_dependencies TEST_TARGET)
        target_link_libraries(${TEST_TARGET} range-v3)
        target_link_libraries(${TEST_TARGET} GTest::gtest GTest::gtest_main)
        target_link_libraries(${TEST_TARGET} fixed_containers project_options project_warnings)
        add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
    endmacro()

    add_executable(comparison_chain_test test/comparison_chain_test.cpp)
    add_test_dependencies(comparison_chain_test)
    add_executable(concepts_test test/concepts_test.cpp)
    add_test_dependencies(concepts_test)
    add_executable(enum_array_test test/enum_array_test.cpp)
    add_test_dependencies(enum_array_test)
    add_executable(enum_map_test test/enum_map_test.cpp)
    add_test_dependencies(enum_map_test)
    add_executable(enum_set_test test/enum_set_test.cpp)
    add_test_dependencies(enum_set_test)
    add_executable(enum_utils_test test/enum_utils_test.cpp)
    add_test_dependencies(enum_utils_test)
    add_executable(fixed_map_test test/fixed_map_test.cpp)
    add_test_dependencies(fixed_map_test)
    add_executable(fixed_map_perf_test test/fixed_map_perf_test.cpp)
    add_test_dependencies(fixed_map_perf_test)
    add_executable(fixed_red_black_tree_test test/fixed_red_black_tree_test.cpp)
    add_test_dependencies(fixed_red_black_tree_test)
    add_executable(fixed_red_black_tree_view_test test/fixed_red_black_tree_view_test.cpp)
    add_test_dependencies(fixed_red_black_tree_view_test)
    add_executable(fixed_set_test test/fixed_set_test.cpp)
    add_test_dependencies(fixed_set_test)
    add_executable(fixed_vector_test test/fixed_vector_test.cpp)
    add_test_dependencies(fixed_vector_test)
    add_executable(index_range_predicate_iterator_test test/index_range_predicate_iterator_test.cpp)
    add_test_dependencies(index_range_predicate_iterator_test)
    add_executable(instance_counter_test test/instance_counter_test.cpp)
    add_test_dependencies(instance_counter_test)
    add_executable(macro_countermeasures_test test/macro_countermeasures_test.cpp)
    add_test_dependencies(macro_countermeasures_test)
    add_executable(pair_view_test test/pair_view_test.cpp)
    add_test_dependencies(pair_view_test)
    add_executable(string_literal_test test/string_literal_test.cpp)
    add_test_dependencies(string_literal_test)
    add_executable(type_name_test test/type_name_test.cpp)
    add_test_dependencies(type_name_test)
endif()
