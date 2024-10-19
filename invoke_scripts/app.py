#!/usr/bin/python3
"""
app.py uses Python Invoke to implement useful project
tooling associated build, packaging, and running the app_ui
application.
"""

import os

# pylint: disable=import-error
# pylint: disable=no-name-in-module
from invoke import task, Collection
from . import vcpkg

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
    :param args: Additional Arguments for buildtool
    :param cmake_args: Additional arguments to pass to CMake via buildtool
    :param sa_checks_off: Run without Static-Analysis checks
    :return:
    """
    vcpkg.init_vcpkg(context)
    if preset == "":
        if not is_linux:
            preset = "windows"
        else:
            preset = "linux"

    cmake_args = ""
    for arg in args.split(" "):
        if arg != "":
            cmake_args += " -D{arg}"
    if sa_checks_off:
        cmake_args += " -DBUILD_WITH_SA_CHECKS=false"
    print(f"preset: {preset}, cmake_args: {cmake_args}")
    context.run(
        f"cmake --workflow --preset {preset} {cmake_args}",
        pty=is_linux,
    )
    context.run(
        f"cmake --install build {cmake_args} --config Debug",
        pty=is_linux,
    )


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
    Runs/Starts simulation of robot
    :param context: Python Invoke provided context for running commands
    :param args: Additional Arguments
    :return:
    """

    context.run(f"./build/src/app_ui/app_ui {args}")


run_coll = Collection("run")
run_coll.add_task(run)

ns = Collection(
    build_coll,
    clean,
    run_coll,
    lint_coll,
)
