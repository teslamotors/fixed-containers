load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "gsl",
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
    name = "gtest",
    urls = ["https://github.com/google/googletest/archive/eb6e9273dcf9c6535abb45306afe558aa961e3c3.tar.gz"],
    strip_prefix = "googletest-eb6e9273dcf9c6535abb45306afe558aa961e3c3",
    sha256 = "b3e3fe31ac3f201b5b8ca0fa6c730b16a730d55c20741b0c3ec23a433756fd20",
    patches = [
            "//:patches/gtest_wcovered_switch_default.patch",
            "//:patches/gtest_use_maybe_unused_instead_of_attribute_unused.patch",
    ],
)

http_archive(
    name = "range-v3",
    urls = ["https://github.com/ericniebler/range-v3/archive/0.11.0.tar.gz"],
    strip_prefix = "range-v3-0.11.0",
    sha256 = "376376615dbba43d3bef75aa590931431ecb49eb36d07bb726a19f680c75e20c",
)
