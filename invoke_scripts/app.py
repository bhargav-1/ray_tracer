#!/usr/bin/python3
"""
app.py uses Python Invoke to implement useful project
tooling associated build, packaging, and running the raytracerchallenge
application.
"""

import os
import shlex

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
        f"{del_cmd} build install",
        pty=is_linux,
    )


@task(default=True)
def build(
    context,
    preset="",
    args="",
    sa_checks_off=False,
):
    # pylint: disable=too-many-arguments
    """
    Builds repo
    :param context: Python Invoke provided context for running commands
    :param preset: Which cmake workflow preset (defined in CMakePresets.json) to build
    :param args: Space-separated CMake cache settings (NAME=VALUE)
    :param sa_checks_off: Run without Static-Analysis checks
    :return:
    """
    if preset == "":
        preset = "linux"

    if preset not in ("linux", "linux-notests"):
        raise ValueError("Use the linux or linux-notests workflow")
    cmake_args = [f"-D{arg}" for arg in shlex.split(args)]
    if sa_checks_off:
        cmake_args.append("-DBUILD_WITH_SA_CHECKS=OFF")
    context.run(
        "cmake --preset cfg-linux " + shlex.join(cmake_args),
        pty=is_linux,
    )
    context.run("cmake --build --preset bld-linux", pty=is_linux)
    if preset == "linux":
        context.run("ctest --preset test-linux", pty=is_linux)
    context.run("cpack --preset pkg-linux", pty=is_linux)
    context.run("cmake --install build --config Debug", pty=is_linux)


@task()
def build_ci(context):
    """
    Builds repo for CI (sa_checks_off)
    :param context: Python Invoke provided context for running commands
    :return:
    """
    build(context, sa_checks_off=True)


build_coll = Collection("build")
build_coll.add_task(build)
build_coll.add_task(build_ci, "ci")


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
    Runs the application
    :param context: Python Invoke provided context for running commands
    :param args: Additional Arguments
    :return:
    """

    context.run(f"./build/src/app/light_and_shading_chapter6{args}")


run_coll = Collection("run")
run_coll.add_task(run)

ns = Collection(
    build_coll,
    clean,
    run_coll,
    lint_coll,
)
