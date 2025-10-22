#pragma once

/**
 * @file compiler_compat.hpp
 * @brief Compiler compatibility utilities and attributes.
 */

/**
 * @brief FIXED_CONTAINERS_NONALLOCATING expands to [[clang::nonallocating]] when supported.
 *
 * This attribute helps clang's function effects analysis detect potentially allocating
 * functions. It's only applied when:
 * 1. Using clang compiler
 * 2. Clang version supports the nonallocating attribute (clang 20+)
 *    - https://releases.llvm.org/20.1.0/tools/clang/docs/ReleaseNotes.html#static-analyzer
 * 3. _GLIBCXX_ASSERTIONS is not enabled (to avoid false positives from debug assertions)
 *
 * @note _GLIBCXX_ASSERTIONS enables debug assertions in the standard library which contain calls
 * to __builtin_printf that clang's function effects analysis considers potentially allocating,
 * even though the code under test may not allocate in normal operation.
 */
#if defined(__clang__) && __clang_major__ >= 20 && !defined(_GLIBCXX_ASSERTIONS)
#define FIXED_CONTAINERS_NONALLOCATING [[clang::nonallocating]]
#else
#define FIXED_CONTAINERS_NONALLOCATING
#endif

/**
 * @brief FIXED_CONTAINERS_SUPPRESS_FUNCTION_EFFECTS disables nonallocating and nonblocking checks.
 *
 * This macro applies diagnostic pragmas to suppress function effects warnings for code
 * that is expected to be non-allocating but may trigger false positives in the compiler's
 * conservative analysis.
 *
 * Usage:
 * ```cpp
 * FIXED_CONTAINERS_SUPPRESS_FUNCTION_EFFECTS(
 *     some_function_that_might_trigger_false_positive();
 * )
 * ```
 */
#if defined(__clang__)
#define FIXED_CONTAINERS_SUPPRESS_FUNCTION_EFFECTS(...)                  \
    _Pragma("clang diagnostic push")                                     \
        _Pragma("clang diagnostic ignored \"-Wunknown-warning-option\"") \
            _Pragma("clang diagnostic ignored \"-Wfunction-effects\"")   \
                __VA_ARGS__ _Pragma("clang diagnostic pop")
#else
#define FIXED_CONTAINERS_SUPPRESS_FUNCTION_EFFECTS(...) __VA_ARGS__
#endif
