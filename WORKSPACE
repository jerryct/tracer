load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_benchmark",
    strip_prefix = "benchmark-1.6.0",
    urls = ["https://github.com/google/benchmark/archive/refs/tags/v1.6.0.zip"],
    sha256 = "3da225763533aa179af8438e994842be5ca72e4a7fed4d7976dc66c8c4502f58",
)

http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-release-1.11.0",
    urls = ["https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip"],
    sha256 = "353571c2440176ded91c2de6d6cd88ddd41401d14692ec1f99e35d013feda55a",
)

http_archive(
    name = "fmtlib_fmt",
    strip_prefix = "fmt-8.0.1",
    urls = ["https://github.com/fmtlib/fmt/archive/refs/tags/8.0.1.zip"],
    sha256 = "6747442c189064b857336007dd7fa3aaf58512aa1a0b2ba76bf1182eefb01025",
    build_file = "@//:fmt.BUILD",
)

http_archive(
    name = "jerryct_string_view",
    strip_prefix = "string_view-1.0",
    urls = ["https://github.com/jerryct/string_view/archive/refs/tags/v1.0.zip"],
    sha256 = "d1aed4baa4ae5db57ac1faffe9028102abe623eaabe15beb4740d8c1cb06208f",
)
