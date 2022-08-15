# C++ Fixed Containers

![License](https://img.shields.io/github/license/teslamotors/fixed-containers)
![Standard](https://img.shields.io/badge/c%2B%2B-20-blue)
[![Ubuntu](https://github.com/teslamotors/fixed-containers/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/teslamotors/fixed-containers/actions/workflows/ubuntu.yml)
[![Ubuntu (clang)](https://github.com/teslamotors/fixed-containers/actions/workflows/ubuntu-clang.yml/badge.svg)](https://github.com/teslamotors/fixed-containers/actions/workflows/ubuntu-clang.yml)
[![Windows](https://github.com/teslamotors/fixed-containers/actions/workflows/windows.yml/badge.svg)](https://github.com/teslamotors/fixed-containers/actions/workflows/windows.yml)
[![CodeQL](https://github.com/teslamotors/fixed-containers/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/teslamotors/fixed-containers/actions/workflows/codeql-analysis.yml)

Header-only C++20 library that provides containers with the following properties:

* Fixed-capacity, declared at compile-time
* constexpr
* containers retain the properties of T (e.g. if T is trivially copyable, then so is FixedVector<T>)
* no pointers stored (data layout is purely self-referential and can be serialized directly)
* no dynamic allocations

# Features

* `FixedVector` - Vector implementation with `std::vector` API and "fixed container" properties
* `FixedMap`/`FixedSet` - Red-Black Tree map/set implementation with `std::map`/`std::set` API and "fixed container" properties.
* `EnumMap`/`EnumSet` - For enum keys only, Map/Set implementation with `std::map`/`std::set` API and "fixed container" properties. O(1) lookups.
* `StringLiteral` - Compile-time null-terminated literal string.
* Rich enums - `enum` & `class` hybrid.

## Rich enum features
* Rich enums behave like an enum (compile-time known values, can be used in switch-statements and as template parameters as well as `EnumMap`/`EnumSet` etc).
* Can have member functions and fields.
* Readily available `count()`, `to_string()`.
* Conversion from string, ordinal.
* Implicit `std::optional`-like semantics.
* Avoid the need for error-prone sentinel values like `UNKNOWN`, `UNINITIALIZED`, `COUNT` etc.
* Avoid Undefined Behavior from uninitialized state. Default constructor can be disabled altogether.
* `EnumAdapter<T>` can adapt any enum-like class to the rich enum API.

```C++
static_assert(fixed_containers::rich_enums::is_rich_enum<Color>);  // Type-trait `concept`
inline constexpr const Color& COLOR = Color::RED();                // Note the parens
static_assert("RED" == COLOR.to_string());                         // auto-provided member
static_assert(COLOR.is_primary());                                 // Custom member
static_assert(COLOR == Color::value_of("RED").value());            // auto-provided
static_assert(4 == Color::count());                                // auto-provided
```
More examples can be found [here](test/enums_test_common.hpp).

# Integration

- Add the `include/` folder to your includes
- Get the dependencies. For example, with [vcpkg](https://github.com/Microsoft/vcpkg):
```
vcpkg install magic-enum range-v3
```

### bazel
Use the following in your `WORKSPACE` file:
```
http_archive(
    name = "fixed_containers",
    urls = ["https://github.com/teslamotors/fixed-containers/archive/<commit>.tar.gz"],
    strip_prefix = "fixed-containers-<commit>",
)

load("@fixed_containers//:fixed_containers_deps.bzl", "fixed_containers_deps")
fixed_containers_deps()
```

Then use in your targets like this:
```
cc_test(
    name = "test",
    srcs = ["test.cpp"],
    deps = [
        "@fixed_containers//:fixed_vector",
        "@fixed_containers//:enum_map",
        "@fixed_containers//:enum_set",
    ],
    copts = ["-std=c++20"],
)
```

# API deviations compared to the standard library

## Map iterator pair
Iterators from `FixedMap` and `EnumMap` do not return `std::pair<const K, V>` but instead `PairView<const K, V>`.  `PairView` acts exactly like a `std::pair`, except that the `.first` and `.second` members are functions instead: `.first()` and `.second()`.

### Explanation

1) Due to `operator->` of the iterator, the key-value pair must be stored somewhere so that we can return a non-dangling reference to it. Adhering to the spec would require storing key and value together, whereas it might be more performant to store them separately (e.g. for less padding) or, in the case of `EnumMap`, not storing the key at all.
2) Storing `std::pair<const K, V>` (with the `const`) is non-assignable and using `reinterprest_cast` for storage purposes clashes with `constexpr`-ness.
3) `std::pair` is not trivially copyable.
4) Using a layout compatible with `std::pair` and `reinterpret _cast`-ing for iterators clashes with `constexpr`-ness.

### Alternatives that have been considered

#### ArrowProxy
```cpp
struct ArrowProxy
{
    reference ref;
    constexpr pointer operator->() { return &ref; }
};

constexpr ArrowProxy operator->() const noexcept { /* impl */; }
```
The downside is that the return type of `operator->` is not an `l-value` reference.

`range-v3` fails to compile and refuses to chain forward the arrow operator. Furthermore, the built-in range-based for-loops are subtly different:

```
enum_map_test.cpp:416:26: error: loop variable 'key_and_value' is always a copy because the range of type 'EnumMap<TestEnum1, int>' does not return a reference [-Werror,-Wrange-loop-bind-reference]
        for (const auto& key_and_value : s)
                         ^
enum_map_test.cpp:416:14: note: use non-reference type 'std::pair<const TestEnum1 &, int &>'
        for (const auto& key_and_value : s)
             ^~~~~~~~~~~~~~~~~~~~~~~~~~~
```

Similar error occurs when using structured binding.

#### Pair of references

A `std::pair<const K&, V&>` would be able to use `.first` and `.second` (non-functions).
However, a reference is non-assignable, so storing this type would not work.


A workaround to the above is to use `std::pair<std::reference_wrapper<cost K>, std::reference_wrapper<V>>` (`constexpr` as of C++20).
Unfortunately, the amount of `.get()` calls makes it just as bad (possibly even worse) than the current solution of having `first()` and `second()` as functions.


### Additional Notes

- The compiler messages are very clear for the chosen solution:
```
test.cpp:128:70: error: reference to non-static member function must be called; did you mean to call it with no arguments?
            std::cout << it->second;
                         ~~~~~~~~~~^~~~~~
                                         ()
```

The compilation failures for the alternatives above are more verbose and harder to interpret.

- C++17+ 's [structured binding](https://en.cppreference.com/w/cpp/language/structured_binding) allows for more concise extraction of the key and value that using `.first` and `.second`:
```C++
for (const auto& [key, value] : my_map)
{

}
```
This syntax works as-is for `FixedMap` and `EnumMap`.

# Running the tests

### cmake

1) Build with the [vcpkg toolchain file](https://vcpkg.io/en/docs/users/integration.html#cmake-integration)
```
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER=/bin/clang-13 -DCMAKE_CXX_COMPILER=/bin/clang++-13 -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

2) Run tests
```
ctest -C Debug
```

### bazel
#### clang
1) Build separately (optional)
```
CC=clang++-13 bazel build --config=clang ...
```
2) Run tests
```
CC=clang++-13 bazel test --config=clang :all_tests
```
#### gcc
1) Build separately (optional)
```
CC=g++-11 bazel build ...
```

2) Run tests
```
CC=g++-11 bazel test :all_tests
```

## Tested Compilers

- Clang  13
- GCC 11
- MSVC++ 14.29 / Visual Studio 2019

## Licensed under the [MIT License](LICENSE)
