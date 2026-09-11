# libc++ regex test port

Ported from LLVM's [libcxx/test/std/re](https://github.com/llvm/llvm-project/tree/242ccbf6b6d9d090762a87138c854f2d93b6ab60/libcxx/test/std/re),
revision `242ccbf6b6d9d090762a87138c854f2d93b6ab60`.
The upstream copyright/SPDX headers and [license](LICENSE.TXT) are preserved.

Of the 177 upstream C++ test files, 147 are executable tests (including three
compile-time conformance tests), four are expected compilation failures, and 26
are inapplicable: 12 placeholders, nine wide-only aliases, two allocator-only
tests, two capacity-incompatible deduction tests, and one diagnostic-warning test.
[manifest.json](manifest.json) accounts for every upstream file and records its
adaptations. Wide-character branches in otherwise applicable tests are disabled.
This is a narrow-character fixed-capacity port, **not** a claim that the unchanged
STL suite passes or that full STL/locale compatibility has been achieved.

## Running

With the project's normal CMake test configuration:

```sh
cmake --build build --target fixed_regex_libcxx_tests -j4
ctest --test-dir build -L libcxx --output-on-failure -j4
```

The suite is enabled by default alongside the other tests. Disable it with
`-DFIXED_CONTAINERS_OPT_BUILD_LIBCXX_REGEX_TESTS=OFF`. CMake enables ASan/UBSan
for Clang, like the project's existing tests. GCC/Clang compile-failure tests
also run through CTest; this diagnostic harness is not enabled for MSVC.

Run the 147 executable tests through Bazel:

```sh
bazel test //test/libcxx_regex:conformance --config=gcc --test_output=errors
```

The standalone GCC/Clang runner needs only Python 3 and a C++20 compiler, and runs
both executable and negative tests without GTest or Bazel:

```sh
python3 test/libcxx_regex/run_tests.py --build-dir build/libcxx-regex-standalone --jobs 4
python3 test/libcxx_regex/run_tests.py --build-dir build/libcxx-regex-negative --kind compile_fail
```

Use `--cxx clang++`, `--filter re.alg`, or
`--flags='-fsanitize=address,undefined -fno-sanitize-recover=all'` as needed.
Assertions remain enabled even in Release builds. The negative harness requires
deleted-overload errors and checks **every** constructor annotated in the
temporary-object test, rather than treating any compilation error as success.

## Porting decisions

[adapter.h](adapter.h) contains aliases directly to `FixedRegex<256, 4096>`, fixed
results/submatches, and fixed iterators. Calls use the actual `fixed_containers`
algorithms; no standard regex engine is used. Regex/results `string_type` checks
expect `FixedString`, while externally supplied `std::string`, custom-allocator
strings, and `std::vector<int>` inputs remain in the tests. Replacement/format
output variables use `auto` to retain their fixed owning result type.

The test-only checking policy throws `std::regex_error` to preserve upstream
error-code assertions. These exception-enabled tests can allocate in their test
setup and error reporting; they do not establish the zero-allocation guarantee.
The separate `fixed_regex_no_allocation_test` guards the default library's
construction, matching, formatting, iteration, and destruction, including long
simple repetitions. Normal library tests still compile without exceptions.

Allocator-specific assertions are omitted from copy/move result tests, retaining
their state/noexcept assertions. Locale tests use explicit deterministic byte
tables, not installed OS locales or global facets. Character-class tests compare
membership for every byte instead of libc++'s implementation-specific mask
values. `LIBCPP_ASSERT` checks of libc++ internals are disabled independently of
the host STL, as they are upstream when testing other implementations.

Range-conformance assertions account for the host library's P2415R2 support:
rvalue results are viewable with `__cpp_lib_ranges >= 202110L`, but not with
older libraries such as libstdc++ 11. Lvalue results are checked on both.

The following behavioral differences are explicit rather than silently hidden:

- The fixed default primary-collation table folds ASCII case, so `[=M=]` includes
  `m`; the corresponding positive match and result-size assertions are adapted.
- Repeat-counter overflow tests exceed `size_t`, not libc++'s narrower internal
  integer. The original long-input and exponential-match workloads are unchanged.
- `\a` and `\e` are identity escapes under the current C++
  [modified grammar](https://eel.is/c++draft/re.grammar); invalid `\c` escapes
  still produce `error_escape`.
- The optional empty iteration of `(a*)*` is rejected per
  [ECMA-262 RepeatMatcher](https://262.ecma-international.org/5.1/#sec-15.10.2.5).
  Its capture is unmatched, and its position is the end of the input.
- For `match_prev_avail`, anchors use the preceding input context (the
  libstdc++/MSVC behavior), not libc++'s conflicting expectations. This remains an
  [underspecified standard issue](https://cplusplus.github.io/LWG/issue3605) and
  a [reported libc++ regression](https://github.com/llvm/llvm-project/issues/74838).
  The port changes those anchor expectations but preserves the word-boundary tests.

To reproduce the port, check out the pinned LLVM revision and run
`python3 test/libcxx_regex/import_tests.py /path/to/llvm-project`. This is an
explicit maintenance operation; builds never download tests or contact LLVM.
