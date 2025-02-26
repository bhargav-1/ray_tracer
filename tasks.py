#!/usr/bin/python3
"""
tasks.py uses Python Invoke to implement useful project
tooling associated with the bhargav-ulc/raytracerchallenge project.

It provides a high-level abstraction away from tooling
specific commands (catkin, docker, pre-commit etc)
"""

# pylint: disable=import-error
from invoke import Collection
from invoke_scripts import docker
from invoke_scripts import app

namespace = Collection()
namespace.add_collection(docker)
namespace.add_collection(app)
