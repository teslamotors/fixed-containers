load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

load("//:fixed_containers_deps.bzl", "fixed_containers_deps")
fixed_containers_deps()

http_archive(
    name = "com_google_googletest",
    urls = ["https://github.com/google/googletest/archive/eff443c6ef5eb6ab598bfaae27f9427fdb4f6af7.tar.gz"],
    strip_prefix = "googletest-eff443c6ef5eb6ab598bfaae27f9427fdb4f6af7",
    sha256 = "59243119d23c19baf18bb4064644e5894b156b15bd0191ea3046db9c4a7ee0ec",
)

http_archive(
    name = "com_google_benchmark",
    urls = ["https://github.com/google/benchmark/archive/447752540c71f34d5d71046e08192db181e9b02b.tar.gz"],
    strip_prefix = "benchmark-447752540c71f34d5d71046e08192db181e9b02b",
    sha256 = "df1235e0e37297e990bb833c723cd3dec61fc95c4951f64bdd5c72d1cea7f4f9",
)
