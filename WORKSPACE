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
    urls = ["https://github.com/google/benchmark/archive/bc946b919cac6f25a199a526da571638cfde109f.tar.gz"],
    strip_prefix = "benchmark-bc946b919cac6f25a199a526da571638cfde109f",
    sha256 = "997090899b61ff5a3f7f6714bc9147694d4f85266dbb93277ba9e6d60009a776",
)

http_archive(
    name = "com_github_ericniebler_range-v3",
    urls = ["https://github.com/ericniebler/range-v3/archive/53c40dd628450c977ee1558285ff43e0613fa7a9.tar.gz"],
    strip_prefix = "range-v3-53c40dd628450c977ee1558285ff43e0613fa7a9",
    sha256 = "2a3e956cc0174ea883d36c51aded8549af10078558177e8f6d76c5e33de950c1",
)
