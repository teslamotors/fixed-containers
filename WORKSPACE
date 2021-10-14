load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

load("//:fixed_containers_deps.bzl", "fixed_containers_deps")
fixed_containers_deps()

http_archive(
    name = "com_microsoft_gsl",
    urls = ["https://github.com/microsoft/GSL/archive/v3.1.0.tar.gz"],
    strip_prefix = "GSL-3.1.0",
    sha256 = "d3234d7f94cea4389e3ca70619b82e8fb4c2f33bb3a070799f1e18eef500a083",
    build_file_content = """
cc_library(
    name = "gsl",
    hdrs = glob(["include/gsl/*"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
""",
)

http_archive(
    name = "com_google_googletest",
    urls = ["https://github.com/google/googletest/archive/16f637fbf4ffc3f7a01fa4eceb7906634565242f.tar.gz"],
    strip_prefix = "googletest-16f637fbf4ffc3f7a01fa4eceb7906634565242f",
    sha256 = "879a4064738cb3c76a11d449145b442654d7282f3cd0f5f03ec8c3dc862646c6",
    patches = [
            "//:patches/gtest_use_maybe_unused_instead_of_attribute_unused.patch",
    ],
)

http_archive(
    name = "com_github_ericniebler_range-v3",
    urls = ["https://github.com/ericniebler/range-v3/archive/0.11.0.tar.gz"],
    strip_prefix = "range-v3-0.11.0",
    sha256 = "376376615dbba43d3bef75aa590931431ecb49eb36d07bb726a19f680c75e20c",
)
