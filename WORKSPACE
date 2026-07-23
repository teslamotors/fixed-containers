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
    urls = ["https://github.com/google/benchmark/archive/refs/tags/v1.9.5.tar.gz"],
    strip_prefix = "benchmark-1.9.5",
    sha256 = "9631341c82bac4a288bef951f8b26b41f69021794184ece969f8473977eaa340",
)
