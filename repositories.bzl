"""Import third_party libraries."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def dbscan_dependencies():
    maybe(
        http_archive,
        name = "rules_python",
        sha256 = "a644da969b6824cc87f8fe7b18101a8a6c57da5db39caa6566ec6109f37d2141",
        strip_prefix = "rules_python-0.20.0",
        url = "https://github.com/bazelbuild/rules_python/releases/download/0.20.0/rules_python-0.20.0.tar.gz",
    )

    maybe(
        http_archive,
        name = "nanoflann",
        build_file = "@dbscan//third_party:nanoflann.BUILD.bazel",
        sha256 = "cbcecf22bec528a8673a113ee9b0e134f91f1f96be57e913fa1f74e98e4449fa",
        strip_prefix = "nanoflann-1.4.3",
        urls = [
            "https://github.com/jlblancoc/nanoflann/archive/refs/tags/v1.4.3.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "nanobench",
        build_file = "@dbscan//third_party:nanobench.BUILD.bazel",
        sha256 = "53a5a913fa695c23546661bf2cd22b299e10a3e994d9ed97daf89b5cada0da70",
        strip_prefix = "nanobench-4.3.11",
        urls = [
            "https://github.com/martinus/nanobench/archive/refs/tags/v4.3.11.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "pybind11",
        build_file = "@dbscan//third_party:pybind11.BUILD.bazel",
        sha256 = "832e2f309c57da9c1e6d4542dedd34b24e4192ecb4d62f6f4866a737454c9970",
        strip_prefix = "pybind11-2.10.4",
        urls = ["https://github.com/pybind/pybind11/archive/refs/tags/v2.10.4.tar.gz"],
    )

    maybe(
        http_archive,
        name = "doctest",
        url = "https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.tar.gz",
        strip_prefix = "doctest-2.4.11",
        sha256 = "632ed2c05a7f53fa961381497bf8069093f0d6628c5f26286161fbd32a560186",
    )
