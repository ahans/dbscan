workspace(name = "dbscan")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "aspect_bazel_lib",
    sha256 = "97fa63d95cc9af006c4c7b2123ddd2a91fb8d273012f17648e6423bae2c69470",
    strip_prefix = "bazel-lib-1.30.2",
    url = "https://github.com/aspect-build/bazel-lib/releases/download/v1.30.2/bazel-lib-v1.30.2.tar.gz",
)

load("@aspect_bazel_lib//lib:repositories.bzl", "aspect_bazel_lib_dependencies")

aspect_bazel_lib_dependencies()

http_archive(
    name = "rules_python",
    sha256 = "a644da969b6824cc87f8fe7b18101a8a6c57da5db39caa6566ec6109f37d2141",
    strip_prefix = "rules_python-0.20.0",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.20.0/rules_python-0.20.0.tar.gz",
)

load("@rules_python//python:repositories.bzl", "py_repositories", "python_register_toolchains")

py_repositories()

python_register_toolchains(
    name = "python3",
    python_version = "3.10",
)

load("@python3//:defs.bzl", "interpreter")

http_archive(
    name = "pybind11_bazel",
    # patch_args = ["-p1"],
    # patches = ["//third_party/pybind11_bazel:python3.10_support.patch"],
    # sha256 = "fc56ce8a8b51e3dd941139d329b63ccfea1d304b",
    strip_prefix = "pybind11_bazel-fc56ce8a8b51e3dd941139d329b63ccfea1d304b",
    urls = ["https://github.com/pybind/pybind11_bazel/archive/fc56ce8a8b51e3dd941139d329b63ccfea1d304b.zip"],
)

http_archive(
    name = "pybind11",
    build_file = "@pybind11_bazel//:pybind11.BUILD",
    sha256 = "832e2f309c57da9c1e6d4542dedd34b24e4192ecb4d62f6f4866a737454c9970",
    strip_prefix = "pybind11-2.10.4",
    urls = ["https://github.com/pybind/pybind11/archive/refs/tags/v2.10.4.tar.gz"],
)

load("@pybind11_bazel//:python_configure.bzl", "python_configure")

python_configure(
    name = "local_config_python",
    python_interpreter_target = interpreter,
)

load("@rules_python//python:pip.bzl", "pip_parse")

pip_parse(
    name = "pypi",
    python_interpreter_target = interpreter,
    quiet = False,
    requirements_lock = "//python:requirements_lock.txt",
)

load("@pypi//:requirements.bzl", "install_deps")

install_deps()

http_archive(
    name = "nanoflann",
    build_file = "@//third_party:nanoflann.BUILD.bazel",
    sha256 = "cbcecf22bec528a8673a113ee9b0e134f91f1f96be57e913fa1f74e98e4449fa",
    strip_prefix = "nanoflann-1.4.3",
    urls = [
        "https://github.com/jlblancoc/nanoflann/archive/refs/tags/v1.4.3.tar.gz",
    ],
)
