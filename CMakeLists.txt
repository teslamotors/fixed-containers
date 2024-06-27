cmake_minimum_required(VERSION 3.16)
#if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
#  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
#endif()
#if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
#  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
#endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)  # This ensures that GNU extensions are not used (-std=c++20 vs -std=gnu++20)
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
    target_compile_options(project_options INTERFACE /wd4324)  # a test intentionally creates this type
else()
    target_compile_options(project_options INTERFACE -Werror -fno-exceptions -ftemplate-backtrace-limit=0)
endif()
if(${USING_CLANG})
    target_compile_options(project_options INTERFACE -ftrivial-auto-var-init=pattern)
    #target_compile_options(project_options INTERFACE --gcc-install-dir=/usr/bin/../lib/gcc/x86_64-linux-gnu/11)
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

        # Prevents iterators from returning non-references. See also:
        # https://quuxplusone.github.io/blog/2020/08/26/wrange-loop-analysis/
        -Wno-range-loop-bind-reference

        # due to gtest
        -Wno-covered-switch-default # v1.11.0 fails (needs new release). bazel points to a newer commit with the fix.
        -Wno-switch-default
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
        /wd4067
    )
else()
    SET(DIAGNOSTIC_FLAGS ${DIAGNOSTIC_FLAGS}
        -Wall
        -Wextra
        -Wpedantic
        # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=114225
        -Wno-dangling-reference
    )
endif()
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE ${DIAGNOSTIC_FLAGS})


if(NOT TARGET magic_enum)
    find_package(magic_enum CONFIG REQUIRED)
endif()

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

    find_package(GTest CONFIG REQUIRED)
    find_package(benchmark CONFIG REQUIRED)

    macro(add_test_dependencies TEST_TARGET)
        if(${USING_CLANG})
            target_compile_options(${TEST_TARGET} PRIVATE -fsanitize=address,undefined -fno-sanitize-recover=all)
            target_link_options(${TEST_TARGET} PRIVATE -fsanitize=address,undefined)
        endif()
        target_link_libraries(${TEST_TARGET} GTest::gtest GTest::gtest_main)
        target_link_libraries(${TEST_TARGET} benchmark::benchmark benchmark::benchmark_main)
        target_link_libraries(${TEST_TARGET} fixed_containers project_options project_warnings)
        add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
    endmacro()

    add_executable(circular_indexing_test test/circular_indexing_test.cpp)
    add_test_dependencies(circular_indexing_test)
    add_executable(circular_integer_range_iterator_test test/circular_integer_range_iterator_test.cpp)
    add_test_dependencies(circular_integer_range_iterator_test)
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
    add_executable(filtered_integer_range_iterator_test test/filtered_integer_range_iterator_test.cpp)
    add_test_dependencies(filtered_integer_range_iterator_test)
    add_executable(fixed_circular_deque_test test/fixed_circular_deque_test.cpp)
    add_test_dependencies(fixed_circular_deque_test)
    add_executable(fixed_circular_queue_test test/fixed_circular_queue_test.cpp)
    add_test_dependencies(fixed_circular_queue_test)
    add_executable(fixed_deque_test test/fixed_deque_test.cpp)
    add_test_dependencies(fixed_deque_test)
    add_executable(fixed_doubly_linked_list_test test/fixed_doubly_linked_list_test.cpp)
    add_test_dependencies(fixed_doubly_linked_list_test)
    add_executable(fixed_doubly_linked_list_raw_view_test test/fixed_doubly_linked_list_raw_view_test.cpp)
    add_test_dependencies(fixed_doubly_linked_list_raw_view_test)
    add_executable(fixed_list_test test/fixed_list_test.cpp)
    add_test_dependencies(fixed_list_test)
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
    add_executable(fixed_robinhood_hashtable_test test/fixed_robinhood_hashtable_test.cpp)
    add_test_dependencies(fixed_robinhood_hashtable_test)
    add_executable(fixed_unordered_map_test test/fixed_unordered_map_test.cpp)
    add_test_dependencies(fixed_unordered_map_test)
    add_executable(fixed_unordered_map_raw_view_test test/fixed_unordered_map_raw_view_test.cpp)
    add_test_dependencies(fixed_unordered_map_raw_view_test)
    add_executable(fixed_unordered_set_test test/fixed_unordered_set_test.cpp)
    add_test_dependencies(fixed_unordered_set_test)
    add_executable(fixed_unordered_set_raw_view_test test/fixed_unordered_set_raw_view_test.cpp)
    add_test_dependencies(fixed_unordered_set_raw_view_test)
    add_executable(fixed_stack_test test/fixed_stack_test.cpp)
    add_test_dependencies(fixed_stack_test)
    add_executable(fixed_queue_test test/fixed_queue_test.cpp)
    add_test_dependencies(fixed_queue_test)
    add_executable(fixed_string_test test/fixed_string_test.cpp)
    add_test_dependencies(fixed_string_test)
    add_executable(fixed_vector_test test/fixed_vector_test.cpp)
    add_test_dependencies(fixed_vector_test)
    add_executable(in_out_test test/in_out_test.cpp)
    add_test_dependencies(in_out_test)
    add_executable(instance_counter_test test/instance_counter_test.cpp)
    add_test_dependencies(instance_counter_test)
    add_executable(int_math_test test/int_math_test.cpp)
    add_test_dependencies(int_math_test)
    add_executable(integer_range_iterator_test test/integer_range_iterator_test.cpp)
    add_test_dependencies(integer_range_iterator_test)
    add_executable(integer_range_test test/integer_range_test.cpp)
    add_test_dependencies(integer_range_test)
    add_executable(macro_countermeasures_test test/macro_countermeasures_test.cpp)
    add_test_dependencies(macro_countermeasures_test)
    add_executable(memory_test test/memory_test.cpp)
    add_test_dependencies(memory_test)
    add_executable(optional_reference_test test/optional_reference_test.cpp)
    add_test_dependencies(optional_reference_test)
    add_executable(out_test test/out_test.cpp)
    add_test_dependencies(out_test)
    add_executable(pair_test test/pair_test.cpp)
    add_test_dependencies(pair_test)
    add_executable(pair_view_test test/pair_view_test.cpp)
    add_test_dependencies(pair_view_test)
    add_executable(queue_adapter_test test/queue_adapter_test.cpp)
    add_test_dependencies(queue_adapter_test)
    add_executable(reflection_test test/reflection_test.cpp)
    add_test_dependencies(reflection_test)
    add_executable(stack_adapter_test test/stack_adapter_test.cpp)
    add_test_dependencies(stack_adapter_test)
    add_executable(string_literal_test test/string_literal_test.cpp)
    add_test_dependencies(string_literal_test)
    add_executable(sub_struct_view_test test/sub_struct_view_test.cpp)
    add_test_dependencies(sub_struct_view_test)
    add_executable(tuples_test test/tuples_test.cpp)
    add_test_dependencies(tuples_test)
    add_executable(type_name_test test/type_name_test.cpp)
    add_test_dependencies(type_name_test)
endif()

option(FIXED_CONTAINERS_OPT_INSTALL "Enable install target" ${PROJECT_IS_TOP_LEVEL})
if (FIXED_CONTAINERS_OPT_INSTALL)
    target_include_directories(fixed_containers INTERFACE $<INSTALL_INTERFACE:include>)

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(${PROJECT_NAME}ConfigVersion.cmake
            VERSION "0.0.0"
            COMPATIBILITY AnyNewerVersion
            ARCH_INDEPENDENT)

    install(TARGETS ${PROJECT_NAME}
            EXPORT ${PROJECT_NAME}Config)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
            DESTINATION lib/cmake/${PROJECT_NAME})

    install(EXPORT ${PROJECT_NAME}Config
            NAMESPACE ${PROJECT_NAME}::
            DESTINATION lib/cmake/${PROJECT_NAME})

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
            DESTINATION .)

    export(EXPORT ${PROJECT_NAME}Config
            NAMESPACE ${PROJECT_NAME}::)
endif()
