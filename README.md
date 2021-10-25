# C++ Fixed Containers

Header-only C++20 library that provides containers with the following properties:

- Fixed-capacity, declared at compile-time
- constexpr
- containers retain the properties of T (e.g. if T is trivially copyable, then so is FixedVector<T>)
- no pointers stored (data layout is purely self-referential and can be serialized directly)
- no dynamic allocations

# Integration

- Add the `include/` folder to your includes
- Get the dependencies with [vcpkg](https://github.com/Microsoft/vcpkg)
```
vcpkg install magic-enum ms-gsl range-v3 gtest
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

# Building the tests

### cmake

1) Get the dependencies (including test dependencies) with [vcpkg](https://github.com/Microsoft/vcpkg)
```
vcpkg install magic-enum ms-gsl range-v3 gtest
```

2) Build with the vcpkg toolchain file
```
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER=/bin/clang-13 -DCMAKE_CXX_COMPILER=/bin/clang++-13 -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

### bazel
```
CC=clang++-13 bazel build ...
```

## Licensed under the [MIT License](LICENSE)
