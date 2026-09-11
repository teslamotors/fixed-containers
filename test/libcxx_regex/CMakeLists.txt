# Vendored conformance tests intentionally use assert and a throwing checking
# policy. Do not link project_options (-fno-exceptions) or apply project-specific
# style warnings to LLVM's test/support sources.
file(GLOB_RECURSE LIBCXX_REGEX_TEST_SOURCES CONFIGURE_DEPENDS tests/*.pass.cpp)
add_custom_target(fixed_regex_libcxx_tests)
foreach(source IN LISTS LIBCXX_REGEX_TEST_SOURCES)
    file(RELATIVE_PATH relative "${CMAKE_CURRENT_SOURCE_DIR}/tests" "${source}")
    string(REPLACE "/" "_" test_name "fixed_regex_libcxx_${relative}")
    string(REPLACE "." "_" test_name "${test_name}")
    add_executable(${test_name} "${source}")
    target_link_libraries(${test_name} PRIVATE fixed_containers)
    target_include_directories(${test_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/support")
    if(MSVC)
        target_compile_options(${test_name} PRIVATE /UNDEBUG /EHsc)
    else()
        target_compile_options(${test_name} PRIVATE -UNDEBUG -fexceptions)
    endif()
    if(USING_CLANG)
        target_compile_options(${test_name} PRIVATE -fsanitize=address,undefined -fno-sanitize-recover=all)
        target_link_options(${test_name} PRIVATE -fsanitize=address,undefined)
    endif()
    add_dependencies(fixed_regex_libcxx_tests ${test_name})
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES LABELS "fixed_regex;libcxx" TIMEOUT 60)
endforeach()

# GCC/Clang negative tests validate deleted-overload diagnostics, including every
# constructor annotated in upstream's temporary-objects.verify test.
if(NOT MSVC)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME fixed_regex_libcxx_compile_fail
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py"
            --cxx "${CMAKE_CXX_COMPILER}" --kind compile_fail
            --build-dir "${CMAKE_CURRENT_BINARY_DIR}/compile_fail"
            "--flags=${CMAKE_CXX_FLAGS}")
    set_tests_properties(fixed_regex_libcxx_compile_fail PROPERTIES LABELS "fixed_regex;libcxx" TIMEOUT 120)
endif()
