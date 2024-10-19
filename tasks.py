#!/usr/bin/python3
"""
tasks.py uses Python Invoke to implement useful project
tooling associated with the bhargav-ulc/app_ui project.

It provides a high-level abstraction away from tooling
specific commands (catkin, docker, pre-commit etc)
"""

# pylint: disable=import-error
from invoke import Collection
from invoke_scripts import docker
from invoke_scripts import app
from invoke_scripts import vcpkg

namespace = Collection()
namespace.add_collection(docker)
namespace.add_collection(app)
namespace.add_collection(vcpkg)
