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
import hashlib
import os
from pathlib import Path
import shlex
import shutil
import tempfile
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
            "git describe --tags --abbrev=0"
            " --match '[0-9]*.[0-9]*.[0-9]*' --match 'v[0-9]*.[0-9]*.[0-9]*'",
            hide=True,
            warn=True,
        )
        # Untagged clones still need a numeric version for CMake and CPack.
        self.tag = tag_result.stdout.strip() if tag_result.ok else "0.0.0"
        self.diff = context.run(
            "git diff --quiet --exit-code || echo +", hide="out"
        ).stdout.strip()


@task
def build(ctx, hw="GPU", mode="dev", ci=False):
    """
    Builds docker container
    :param context:
    :param hw: CPU or GPU
    :param mode: dev
    :param ci: Use the published development image, run CI checks, and copy packages to dist
    :return:
    """
    git = GitVersion(ctx)
    ci_checks_str = "true" if ci else "false"
    docker_base = "ubuntu:noble"
    dockerfile = "docker/Dockerfile"
    image_name = f"bhargavulc/{proj}:{hw}-{mode}"
    ctx.run(
        f"docker build --network=host --target {mode} -f {dockerfile}"
        + f" --build-arg PROJ_NAME={proj}"
        + f" --build-arg DOCKER_BASE={docker_base}"
        + f" --build-arg HARDWARE={hw}"
        + f" --build-arg CI_CHECKS={ci_checks_str}"
        + (
            " --build-arg DEV_ENV_IMAGE=bhargavulc/ray_tracer:dev-env"
            if ci
            else ""
        )
        + f' --build-arg GIT_REV_ARG="{git.rev}"'
        + f' --build-arg GIT_DIFF_ARG="{git.diff}"'
        + f' --build-arg GIT_TAG_ARG="{git.tag}"'
        + f' --build-arg GIT_BRANCH_ARG="{git.branch}"'
        + f" -t {image_name} ."
    )
    if ci:
        _copy_artifacts(ctx, image_name)


def _copy_artifacts(ctx, image_name, output="dist"):
    """Extract Debian packages and checksums from a built image without starting it."""
    image = shlex.quote(image_name)
    workdir = ctx.run(
        f"docker image inspect {image} --format '{{{{.Config.WorkingDir}}}}'",
        hide=True,
    ).stdout.strip()
    container = ctx.run(
        f"docker create --entrypoint /bin/true {image}", hide=True
    ).stdout.strip()
    try:
        with tempfile.TemporaryDirectory(prefix="raytracer-packages-") as temporary:
            source = shlex.quote(f"{container}:{workdir}/build/packages/.")
            ctx.run(f"docker cp {source} {shlex.quote(temporary)}")
            packages = sorted(Path(temporary).glob("*.deb"))
            if not packages:
                raise RuntimeError("The image contains no Debian packages")
            destination = Path(output)
            destination.mkdir(parents=True, exist_ok=True)
            checksums = []
            for package in packages:
                shutil.copy2(package, destination / package.name)
                checksum = hashlib.sha256(package.read_bytes()).hexdigest()
                checksums.append(f"{checksum}  {package.name}\n")
            (destination / "SHA256SUMS").write_text(
                "".join(checksums), encoding="utf-8"
            )
    finally:
        ctx.run(f"docker rm {shlex.quote(container)}", hide=True)


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
