#!/usr/bin/python3
"""
app.py uses Python Invoke to implement useful project
tooling associated build, packaging, and running the raytracerchallenge
application.
"""

import os
import shlex
import shutil
from pathlib import Path
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from functools import partial

# pylint: disable=import-error
# pylint: disable=no-name-in-module
from invoke import task, Collection

is_linux = os.name != "nt"


@task
def clean(context):
    """
    Cleans repo by deleting all build artifacts
    :param context:
    :return:
    """
    del_cmd = "rm -rf"
    if not is_linux:
        del_cmd = "RMDIR /S /Q"
    context.run(
        f"{del_cmd} build build-web install",
        pty=is_linux,
    )


@task(default=True)
def build(
    context,
    preset="",
    args="",
    sa_checks_off=False,
    build_type="Debug",
):
    # pylint: disable=too-many-arguments
    """
    Builds repo
    :param context: Python Invoke provided context for running commands
    :param preset: Which cmake workflow preset (defined in CMakePresets.json) to build
    :param args: Space-separated CMake cache settings (NAME=VALUE)
    :param sa_checks_off: Run without Static-Analysis checks
    :param build_type: CMake build configuration (Debug or Release)
    :return:
    """
    if preset == "":
        preset = "linux"

    if preset not in ("linux", "linux-notests"):
        raise ValueError("Use the linux or linux-notests workflow")
    settings = dict(arg.split("=", 1) for arg in shlex.split(args))
    # Preserve callers that already select the configuration through --args.
    build_type = settings.pop("CMAKE_BUILD_TYPE", build_type)
    if build_type not in ("Debug", "Release", "RelWithDebInfo", "MinSizeRel"):
        raise ValueError("Unsupported CMake build type")
    settings.setdefault("BUILD_WITH_SA_CHECKS", "ON")
    if sa_checks_off:
        settings["BUILD_WITH_SA_CHECKS"] = "OFF"
    cmake_args = [f"-DCMAKE_BUILD_TYPE={build_type}"]
    cmake_args.extend(f"-D{key}={value}" for key, value in settings.items())
    context.run(
        "cmake --preset cfg-linux " + shlex.join(cmake_args),
        pty=is_linux,
    )
    config = shlex.quote(build_type)
    context.run(f"cmake --build --preset bld-linux --config {config}", pty=is_linux)
    if preset == "linux":
        context.run(f"ctest --preset test-linux -C {config}", pty=is_linux)
    context.run(f"cpack --preset pkg-linux -C {config}", pty=is_linux)
    context.run(f"cmake --install build --config {config}", pty=is_linux)


@task()
def build_ci(context):
    """
    Builds and tests Release packages, then runs static analysis through pre-commit.
    :param context: Python Invoke provided context for running commands
    :return:
    """
    build(context, build_type="Release", sa_checks_off=True)
    context.run(
        "pre-commit run --all-files --show-diff-on-failure",
        env={"SKIP": "no-commit-to-branch"},
        pty=is_linux,
    )


@task
def build_web(context):
    """Build the shared C++/ImGui editor for browsers (requires an activated Emscripten SDK)."""
    if shutil.which("emcmake") is None:
        raise RuntimeError(
            "Activate the Emscripten SDK first: source /path/to/emsdk/emsdk_env.sh"
        )
    context.run(
        "emcmake cmake -S . -B build-web -G Ninja "
        "-DCMAKE_BUILD_TYPE=Release -DBUILD_WITH_SA_CHECKS=OFF",
        pty=is_linux,
    )
    context.run(
        "cmake --build build-web --target scene_editor --parallel 4", pty=is_linux
    )
    print("Web app built in build-web/web. Start it with: invoke app.run.web")


build_coll = Collection("build")
build_coll.add_task(build)
build_coll.add_task(build_ci, "ci")
build_coll.add_task(build_web, "web")


@task(default=True)
def lint(context):
    """
    Runs Linting checks
    :param context: Python Invoke provided context for running commands
    :return:
    """
    pre_commit(context)


@task()
def pre_commit(context):
    """
    Runs Pre-commit lint checks
    :param context: Python Invoke provided context for running commands
    :return:
    """
    context.run(
        "pre-commit",
        pty=is_linux,
    )


lint_coll = Collection("lint")
lint_coll.add_task(lint, "lint")
lint_coll.add_task(pre_commit, "pre-commit")


@task(default=True)
def run(context, args=""):
    """
    Runs the scene editor
    :param context: Python Invoke provided context for running commands
    :param args: Additional Arguments
    :return:
    """

    context.run(f"./build/src/app/raytracerchallenge {args}".rstrip())


@task
def run_web(_context, port=8000):
    """Serve the web build on localhost, with headers required for browser threads."""
    directory = Path("build-web/web").resolve()
    if not (directory / "index.html").is_file():
        raise RuntimeError("Build the web app first: invoke app.build.web")

    class WebHandler(SimpleHTTPRequestHandler):
        """Allow the shared WebAssembly memory used by C++ rendering threads."""

        def end_headers(self):
            self.send_header("Cross-Origin-Opener-Policy", "same-origin")
            self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
            super().end_headers()

    handler = partial(WebHandler, directory=str(directory))
    print(f"Open http://localhost:{port} (Ctrl+C to stop)")
    with ThreadingHTTPServer(("127.0.0.1", int(port)), handler) as server:
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass


run_coll = Collection("run")
run_coll.add_task(run)
run_coll.add_task(run_web, "web")

ns = Collection(
    build_coll,
    clean,
    run_coll,
    lint_coll,
)
