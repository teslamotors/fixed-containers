# C++ Fixed Containers

Header-only C++20 library that provides containers with the following properties:

- Fixed-capacity, declared at compile-time
- constexpr
- containers retain the properties of T (e.g. if T is trivially copyable, then so is FixedVector<T>)
- no pointers stored (shared memory friendly, can be accessed from different processes)
- no dynamic allocations

## Licensed under the [MIT License](LICENSE)
