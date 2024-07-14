# C++ Fixed Containers

![License](https://img.shields.io/github/license/teslamotors/fixed-containers)
![Standard](https://img.shields.io/badge/c%2B%2B-20-blue)
[![Ubuntu](https://github.com/teslamotors/fixed-containers/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/teslamotors/fixed-containers/actions/workflows/ubuntu.yml)
[![Ubuntu (clang)](https://github.com/teslamotors/fixed-containers/actions/workflows/ubuntu-clang.yml/badge.svg)](https://github.com/teslamotors/fixed-containers/actions/workflows/ubuntu-clang.yml)
[![Windows](https://github.com/teslamotors/fixed-containers/actions/workflows/windows.yml/badge.svg)](https://github.com/teslamotors/fixed-containers/actions/workflows/windows.yml)
[![CodeQL](https://github.com/teslamotors/fixed-containers/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/teslamotors/fixed-containers/actions/workflows/codeql-analysis.yml)

Header-only C++20 library that provides containers with the following properties:

* Fixed-capacity, declared at compile-time, no dynamic allocations
* constexpr - can be used at both compile-time and runtime (including mutation)
* containers retain the properties of T (e.g. if T is trivially copyable, then so is FixedVector<T>)
* no pointers stored (data layout is purely self-referential and can be serialized directly)
* instances can be used as non-type template parameters

# Features

* The following table shows the available fixed containers and their equivalent std-containers.
The fixed-container types have identical APIs to their std:: equivalents, so you can refer to the traditional C++ docs for how to use them.

   | fixed-container      | std-container equivalent                        |
   |:---------------------|:------------------------------------------------|
   | `FixedVector`        | `std::vector`                                   |
   | `FixedDeque`         | `std::deque`                                    |
   | `FixedList `         | `std::list`                                     |
   | `FixedQueue`         | `std::queue`                                    |
   | `FixedStack`         | `std::stack`                                    |
   | `FixedCircularDeque` | `std::deque` API with Circular Buffer semantics |
   | `FixedCircularQueue` | `std::queue` API with Circular Buffer semantics |
   | `FixedString`        | `std::string`                                   |
   | `FixedMap`           | `std::map`                                      |
   | `FixedSet`           | `std::set`                                      |
   | `FixedUnorderedMap`  | `std::unordered_map`                            |
   | `FixedUnorderedSet`  | `std::unordered_set`                            |
   | `EnumMap`            | `std::map` for enum keys only                   |
   | `EnumSet`            | `std::set` for enum keys only                   |
   | `EnumArray`          | `std::array` but with typed accessors           |

* `StringLiteral` - Compile-time null-terminated literal string.
* Rich enums - `enum` & `class` hybrid.

## Rich enum features
* Rich enums behave like an enum (compile-time known values, can be used in switch-statements, template parameters as well as `EnumMap`/`EnumSet` etc).
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

# Examples
- FixedVector
    ```C++
    constexpr auto v1 = []()
    {
        FixedVector<int, 11> v{};
        v.push_back(0);
        v.emplace_back(1);
        v.push_back(2);
        return v;
    }();
    static_assert(v1[0] == 0);
    static_assert(v1[1] == 1);
    static_assert(v1[2] == 2);
    static_assert(v1.size() == 3);
    static_assert(v1.capacity() == 11);
    ```

- FixedList
    ```C++
    constexpr auto v1 = []()
    {
        FixedList<int, 11> v{};
        v.push_back(0);
        v.emplace_back(1);
        v.push_front(2);
        return v;
    }();
    static_assert(*v1.begin() == 2);
    static_assert(v1.size() == 3);
    static_assert(v1.max_size() == 11);
    ```

- FixedDeque
    ```C++
    constexpr auto v1 = []()
    {
        FixedDeque<int, 11> v{};
        v.push_back(0);
        v.emplace_back(1);
        v.push_front(2);
        return v;
    }();
    static_assert(v1[0] == 2);
    static_assert(v1[1] == 0);
    static_assert(v1[2] == 1);
    static_assert(v1.size() == 3);
    static_assert(v1.max_size() == 11);
    ```

- FixedQueue
    ```C++
    constexpr auto s1 = []()
    {
        FixedQueue<int, 3> v1{};
        v1.push(77);
        v1.push(88);
        v1.push(99);
        return v1;
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.back() == 99);
    static_assert(s1.size() == 3);
    ```

- FixedStack
    ```C++
    constexpr auto s1 = []()
    {
        FixedStack<int, 3> v1{};
        int my_int = 77;
        v1.push(my_int);
        v1.push(99);
        return v1;
    }();

    static_assert(s1.top() == 99);
    static_assert(s1.size() == 2);
    ```

- FixedCircularDeque
    ```C++
    constexpr auto v1 = []()
    {
        FixedCircularDeque<int, 3> v{};
        v.push_back(2);
        v.emplace_back(3);
        v.push_front(1);
        v.emplace_front(0);
        v.push_back(4);
        return v;
    }();

    static_assert(v1[0] == 1);
    static_assert(v1[1] == 2);
    static_assert(v1[2] == 4);
    static_assert(v1.size() == 3);
    ```

- FixedCircularQueue
    ```C++
    constexpr auto s1 = []()
    {
        FixedCircularQueue<int, 3> v1{};
        v1.push(55);
        v1.push(66);
        v1.push(77);
        v1.push(88);
        v1.push(99);
        return v1;
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.back() == 99);
    static_assert(s1.size() == 3);
    ```

- FixedString
    ```C++
    constexpr auto v1 = []()
    {
        FixedString<11> v{"012"};
        v.at(1) = 'b';

        return v;
    }();

    static_assert(v1.at(0) == '0');
    static_assert(v1.at(1) == 'b');
    static_assert(v1.at(2) == '2');
    static_assert(v1.size() == 3);
    ```

- FixedMap
    ```C++
    constexpr auto m1 = []()
    {
        FixedMap<int, int, 11> m{};
        m.insert({2, 20});
        m[4] = 40;
        return m;
    }();
    static_assert(!m1.contains(1));
    static_assert(m1.contains(2));
    static_assert(m1.at(4) == 40);
    static_assert(m1.size() == 2);
    static_assert(m1.max_size() == 11);
    ```

- FixedSet
    ```C++
    constexpr auto s1 = []()
    {
        FixedSet<int, 11> s{};
        s.insert(2);
        s.insert(4);
        return s;
    }();
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(s1.size() == 2);
    static_assert(s1.max_size() == 11);
    ```

- FixedUnorderedMap
    ```C++
    constexpr auto m1 = []()
    {
        FixedUnorderedMap<int, int, 11> m{};
        m.insert({2, 20});
        m[4] = 40;
        return m;
    }();
    static_assert(!m1.contains(1));
    static_assert(m1.contains(2));
    static_assert(m1.at(4) == 40);
    static_assert(m1.size() == 2);
    static_assert(m1.max_size() == 11);
    ```

- FixedUnorderedSet
    ```C++
    constexpr auto s1 = []()
    {
        FixedUnorderedSet<int, 11> s{};
        s.insert(2);
        s.insert(4);
        return s;
    }();
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(s1.size() == 2);
    static_assert(s1.max_size() == 11);
    ```

- EnumMap
    ```C++
    enum class Color { RED, YELLOW, BLUE};

    constexpr auto m1 = []()
    {
        EnumMap<Color, int> m{};
        m.insert({Color::RED, 20});
        m[Color::YELLOW] = 40;
        return m;
    }();
    static_assert(!m1.contains(Color::BLUE));
    static_assert(m1.contains(Color::RED));
    static_assert(m1.at(Color::YELLOW) == 40);
    static_assert(m1.size() == 2);

    // Ensures all keys are specified, at compile-time
    constexpr auto m2 = EnumMap<Color, int>::create_with_all_entries({
        {Color::RED, 42},
        {Color::YELLOW, 7},
        {Color::BLUE, 6},
    });
    ```

- EnumSet
    ```C++
    enum class Color { RED, YELLOW, BLUE};

    constexpr auto s1 = []()
    {
        EnumSet<Color> s{};
        s.insert(Color::RED);
        s.insert(Color::YELLOW);
        return s;
    }();
    static_assert(!s1.contains(Color::BLUE));
    static_assert(s1.contains(Color::RED));
    static_assert(s1.size() == 2);

    constexpr auto s2 = EnumSet<Color>::all(); // full set
    constexpr auto s3 = EnumSet<Color>::none(); // empty set
    constexpr auto s4 = EnumSet<Color>::complement_of(s2); // empty set
    ```

- EnumArray
    ```C++
    constexpr EnumArray<TestEnum1, int> s1{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
    static_assert(4 == s1.max_size());
    static_assert(s1.at(TestEnum1::ONE) == 10);
    static_assert(s1.at(TestEnum1::TWO) == 0);
    static_assert(s1.at(TestEnum1::THREE) == 0);
    static_assert(s1.at(TestEnum1::FOUR) == 40);
    ```

- StringLiteral
    ```C++
    static constexpr const char* s = "blah"; // strlen==4, sizeof==8
    static constexpr const char s[5] = "blah";  // strlen==4, sizeof==5 (null terminator)
    static constexpr StringLiteral s = "blah";  // constexpr .size()==4
    ```

- Using instances as non-type template parameters
    ```C++
    // Similarly to simple types like ints/enums and std::array,
    // fixed_container instances can be used as template parameters
    template <FixedVector<int, 5> /*MY_VEC*/>
    constexpr void fixed_vector_instance_can_be_used_as_a_template_parameter()
    {
    }

    void test()
    {
        static constexpr FixedVector<int, 5> VEC1{};
        fixed_vector_instance_can_be_used_as_a_template_parameter<VEC1>();
    }
    ```

# Integration

### cmake
```
find_package(fixed_containers CONFIG REQUIRED)
target_link_libraries(<your_binary> fixed_containers::fixed_containers)
```

### bazel
If you are managing dependencies with the newer bzlmod system, use the following in your `MODULE.bazel` file:
```
bazel_dep(name = "fixed_containers")
archive_override(
    module_name = "fixed_containers",
    strip_prefix = "fixed-containers-<commit>",
    urls = ["https://github.com/teslamotors/fixed-containers/archive/<commit>.tar.gz"],
)
```

If you are managing dependencies with the older `WORKSPACE` system, use the following in your `WORKSPACE` file:
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
### Alternative
Since this is a header-only library, you can also:
- Add the `include/` folder to your includes
- Get the dependencies. For example, with [vcpkg](https://github.com/Microsoft/vcpkg):
```
vcpkg install magic-enum
```

# Running the tests

### cmake

1) Build with the [vcpkg toolchain file](https://vcpkg.io/en/docs/users/buildsystems/cmake-integration.html)
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
CC=g++-11 bazel build --config=gcc ...
```

2) Run tests
```
CC=g++-11 bazel test --config=gcc :all_tests
```

## Tested Compilers

- Clang  13
- GCC 11
- MSVC++ 14.29 / Visual Studio 2019

## Licensed under the [MIT License](LICENSE)
