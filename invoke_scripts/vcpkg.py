#!/usr/bin/python3
"""
vcpkg.py uses Python Invoke to implement useful project
tooling associated with vcpkg.
"""
import os

# pylint: disable=import-error
from invoke import task, Collection

is_linux = os.name != "nt"


def is_vcpkg_installed():
    """
    Is vcpkg installed?
    :return: true if vcpkg is installed
    """
    return os.path.exists(".vcpkg") and os.path.isdir(".vcpkg")


@task(default=True)
def init_vcpkg(context):
    """
    Initializes vcpkg
    :param context: Python Invoke provided context for running commands
    :return:
    """
    if not is_vcpkg_installed():
        print("VCPKG isn't Initialized, initializing...")
        context.run("git clone --depth=1 https://github.com/Microsoft/vcpkg.git .vcpkg")
    with context.cd(".vcpkg"):
        print("Updating VCPKG...")
        context.run("git pull origin")
        if is_linux:
            context.run("./bootstrap-vcpkg.sh")
            # Set to manifest mode, instead of classic
            context.run("./vcpkg x-update-baseline")
        else:
            context.run("bootstrap-vcpkg.bat")
            # Set to manifest mode, instead of classic
            context.run("vcpkg x-update-baseline")


ns = Collection()
ns.add_task(init_vcpkg, "init", default=True)
