"""Load dependencies needed to compile the fixed_containers library as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def fixed_containers_deps():
    maybe(
        http_archive,
        name = "com_github_neargye_magic_enum",
        urls = ["https://github.com/Neargye/magic_enum/archive/41c916432bfc1a51560f9f165d52ed5c89d4932e.tar.gz"],
        strip_prefix = "magic_enum-41c916432bfc1a51560f9f165d52ed5c89d4932e",
        sha256 = "cac49b49ec8a9f61fe161874ad30bbd1ca3b104b156246d533f0b1ea480e481e",
    )
