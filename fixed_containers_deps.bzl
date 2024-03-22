"""Load dependencies needed to compile the fixed_containers library as a 3rd-party consumer."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def fixed_containers_deps():
    maybe(
        http_archive,
        name = "com_github_neargye_magic_enum",
        urls = ["https://github.com/Neargye/magic_enum/archive/d63a9d63717640705c6c9654d68fa6575d39bab8.tar.gz"],
        strip_prefix = "magic_enum-d63a9d63717640705c6c9654d68fa6575d39bab8",
        sha256 = "9b2b1788dd651feab88bcffb0b2ae3ded4c27142a295d6ede882c8fc8a327a35",
    )
