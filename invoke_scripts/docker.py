#!/usr/bin/python3
"""
docker.py uses Python Invoke to implement useful project
tooling associated with the bhargavulc/raytraerchallenge project.

In particular docker.py is focused on implementing building
and running of docker containers

Note: to run GUI applications on host run `xhost + local:root`
"""
# pylint: disable=invalid-name
# pylint: disable=import-error
# pylint: disable=fixme
import dataclasses
import os
from invoke import task

proj = "raytracerchallenge"

is_linux = os.name != "nt"


@dataclasses.dataclass
class GitVersion:
    """
    GitVersion contains 4 components of current git HEAD

    :param branch: The branch name of HEAD
    :type branch: str

    :param rev: 6-digit checksum of HEAD
    :type rev: str

    :param tag: Most recent git tag
    :type tag: str

    :param diff: Any uncommitted changes? Either '' (no changes) or '+' (changes)
    :type diff: str
    """

    branch: str
    rev: str
    tag: str
    diff: str

    def __init__(self, context):
        """
        Constructor uses provided context (from Python Invoke)
        to get various components of git version
        :param context:
        """
        self.branch = context.run(
            "git rev-parse --abbrev-ref HEAD", hide="out"
        ).stdout.strip()
        self.rev = context.run(
            "git log --pretty=format:'%h' -n 1", hide="out"
        ).stdout.strip()
        tag_result = context.run(
            "git describe --tags --abbrev=0", hide=True, warn=True
        )
        # Untagged clones still need a numeric version for CMake and CPack.
        self.tag = tag_result.stdout.strip() if tag_result.ok else "0.0.0"
        self.diff = context.run(
            "git diff --quiet --exit-code || echo +", hide="out"
        ).stdout.strip()


@task
def build(ctx, hw="GPU", mode="dev", ci_checks=False):
    """
    Builds docker container
    :param context:
    :param hw: CPU or GPU
    :param mode: dev
    :param ci_checks: Execute pre-commit during build?
    :return:
    """
    git = GitVersion(ctx)
    ci_checks_str = "false"
    if ci_checks:
        ci_checks_str = "true"
    docker_base = "ubuntu:focal"
    dockerfile = "docker/Dockerfile"
    image_name = f"bhargavulc/{proj}:{hw}-{mode}"
    ctx.run(
        f"docker build --network=host --target {mode} -f {dockerfile}"
        + f" --build-arg PROJ_NAME={proj}"
        + f" --build-arg DOCKER_BASE={docker_base}"
        + f" --build-arg HARDWARE={hw}"
        + f" --build-arg CI_CHECKS={ci_checks_str}"
        + f' --build-arg GIT_REV_ARG="{git.rev}"'
        + f' --build-arg GIT_DIFF_ARG="{git.diff}"'
        + f' --build-arg GIT_TAG_ARG="{git.tag}"'
        + f' --build-arg GIT_BRANCH_ARG="{git.branch}"'
        + f" -t {image_name} ."
    )


@task
def login(ctx):
    """Open an interactive SSH session in the container as the current user."""
    ctx.run(
        'ssh -o StrictHostKeyChecking=accept-new "$USER@localhost" -p 1122',
        pty=True,
    )


@task
def up(
    ctx,
    use_local_image=False,
    hw="GPU",
    mode="dev",
    recent=False,
    gpu=False,
):
    # pylint: disable=too-many-arguments
    """
    Creates and starts the Compose container in the background
    :param context:
    :param use_local_image:
    :param hw: Image variant (CPU or GPU)
    :param mode: dev
    :param recent:
    :param gpu: Enable the NVIDIA container runtime
    :return:
    """
    # TODO make this global var but overridable
    image_name = f"bhargavulc/{proj}:{hw}-{mode}"
    runtime = "nvidia" if gpu else "runc"

    if use_local_image:
        image_name = f"raytracerchallenge:{hw}-{mode}"

    else:
        if recent:
            ctx.run(f"docker pull {image_name} ")

    print(image_name)
    print(
        f"IMAGE_NAME={image_name} RUNTIME={runtime}"
        + r" DUID=$(id -u) DGID=$(id -g) docker compose"
        + " -f docker-compose.yml up -d"
    )
    ctx.run(
        f"DOCKER_BUILDKIT=0 IMAGE_NAME={image_name} RUNTIME={runtime}"
        + r" DUID=$(id -u) DGID=$(id -g) docker compose"
        + " -f docker-compose.yml up -d",
        pty=is_linux,
    )


@task
def down(
    ctx,
    use_local_image=False,
    hw="GPU",
    mode="dev",
    recent=False,
):
    """
    Stops and removes the Compose container
    :param context:
    :param use_local_image:
    :param hw: CPU or GPU
    :param mode: dev
    :param recent:
    :return:
    """
    # TODO make this global var but overridable
    image_name = f"bhargavulc/{proj}:{hw}-{mode}"
    runtime = "runc"

    if use_local_image:
        image_name = f"{proj}:{hw}-{mode}"

    else:
        if recent:
            ctx.run(f"docker pull {image_name} ")

    print(image_name)
    print(
        f"IMAGE_NAME={image_name} RUNTIME={runtime}"
        + r" DUID=$(id -u) DGID=$(id -g) docker compose -f docker-compose.yml down"
    )
    ctx.run(
        f"IMAGE_NAME={image_name} RUNTIME={runtime}"
        + r" DUID=$(id -u) DGID=$(id -g) docker compose -f docker-compose.yml down",
        pty=is_linux,
    )
