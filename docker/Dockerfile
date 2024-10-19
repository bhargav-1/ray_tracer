
ARG DOCKER_BASE=ubuntu:focal
# hadolint global ignore=DL3004
# hadolint global ignore=DL3006
FROM $DOCKER_BASE as base
ARG PROJ_NAME
ARG HARDWARE=CPU
ARG UID=1000
ARG GID=1000

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

ARG USER_ARG=bob
ENV USER=${USER_ARG}

# Upgrade and install sudo so we can
#hadolint ignore=DL3015
RUN apt-get update \
    && apt-get upgrade -y \
    && apt-get clean \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y sudo ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Adding a low-privilege user
#hadolint ignore=SC2086
RUN groupadd -g ${GID} -r ${USER_ARG}; \
    useradd -m -l -r -u ${UID} -s /bin/bash -g ${GID} ${USER_ARG} --groups sudo; \
    chown -R ${USER_ARG} /home/${USER_ARG}; \
    echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers; \
    echo "${USER_ARG} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers;


# AFTER THIS LINE WE'RE RUNNING AS USER
# sudoers is config'd to let us do anything, but that only works if we invoke sudo
# REMEMBER TO ADD SUDO TO APTnv!
USER ${USER_ARG}

FROM base as dev
#
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

COPY ./docker/dependencies/dev-cpu.deps /tmp/
#hadolint ignore=SC2046
RUN sudo apt-get update && sudo DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get install -y \
  $(cat /tmp/dev-cpu.deps) \
&& sudo apt-get clean \
&& sudo rm -rf /var/lib/apt/lists/*

# Need to sudo pip3 install so they get installed in system and we don't have to modify path
COPY ./docker/dependencies/pip3.txt /tmp/
RUN sudo pip3 install --no-cache-dir -r /tmp/pip3.txt

# If Dev, install fixuid so we can fix ownership. Should only
#hadolint ignore=SC2059
RUN USER=${USER_ARG} && \
    GROUP=${USER_ARG} && \
    curl -SsL https://github.com/boxboat/fixuid/releases/download/v0.6.0/fixuid-0.6.0-linux-amd64.tar.gz | sudo tar -C /usr/local/bin -xzf - && \
    sudo chown root:root /usr/local/bin/fixuid && \
    sudo chmod 4755 /usr/local/bin/fixuid && \
    sudo mkdir -p /etc/fixuid && \
    printf "user: $USER\ngroup: $GROUP\n" | sudo tee -a /etc/fixuid/config.yml;

WORKDIR /home/${USER_ARG}/${PROJ_NAME}
COPY --chown=${UID}:${GID} ./ /home/${USER_ARG}/${PROJ_NAME}
COPY --chown=${UID}:${GID} ./invoke_scripts/ ./invoke_scripts
COPY --chown=${UID}:${GID} ./tasks.py .
RUN { \
    echo "cd /home/${USER_ARG}/${PROJ_NAME}/"; \
    } >> ~/.bashrc

ARG GIT_REV_ARG=""
ARG GIT_DIFF_ARG=""
ARG GIT_TAG_ARG="unknown"
ARG GIT_BRANCH_ARG="master"
ENV GIT_REV ${GIT_REV_ARG}
ENV GIT_DIFF ${GIT_DIFF_ARG}
ENV GIT_TAG ${GIT_TAG_ARG}
ENV GIT_BRANCH ${GIT_BRANCH_ARG}
ARG CI_CHECKS="false"
RUN if [ "${CI_CHECKS}" = "true" ]; then  \
      invoke app.build; \
    else \
      invoke app.build --preset="linux-notests" --sa-checks-off; \
    fi

WORKDIR /home/${USER}/${PROJ_NAME}/build/packages
# Generate a list of deps so we can use that later to install all the deps
# before installing the app to prevent docker cache misses
RUN ls && dpkg-deb -I chirp-app_ui*.deb | grep -E "Depends|Recommends|Suggests|Pre\-Depends" \
    | tr -d "|," | sed "s/([^)]*)/()/g" | tr -d "()" | tr " " "\n" \
    | grep -Ev "Depends|Recommends|Suggests|Pre\-Depends" | xargs > app.deps
WORKDIR /home/${USER_ARG}/${PROJ_NAME}
#
COPY --chown=${UID}:${GID} ./.pre-commit-config.yaml ./.pre-commit-config.yaml
COPY --chown=${UID}:${GID} ./.gitignore ./.gitignore

#hadolint ignore=DL3003,SC2164
RUN if [ "${CI_CHECKS}" = "true" ]; then  \
        source ~/.bashrc; \
        pre-commit install --install-hooks; \
        SKIP=no-commit-to-branch pre-commit run -a; \
    fi
#
RUN bash -c 'echo -e HARDWARE is $HARDWARE'

RUN if [ "$HARDWARE" = "GPU" ]; then \
      echo "HARDWARE intended for: $HARDWARE"; \
      echo "export NVIDIA_VISIBLE_DEVICES \${NVIDIA_VISIBLE_DEVICES:-all}" | sudo tee -a /etc/environment && \
      echo "export NVIDIA_DRIVER_CAPABILITIES \${NVIDIA_DRIVER_CAPABILITIES:+$NVIDIA_DRIVER_CAPABILITIES,}graphics" | sudo tee -a /etc/environment; \
    fi
#
ENV NVIDIA_VISIBLE_DEVICES \
    ${NVIDIA_VISIBLE_DEVICES:-all}
ENV NVIDIA_DRIVER_CAPABILITIES \
    ${NVIDIA_DRIVER_CAPABILITIES:+$NVIDIA_DRIVER_CAPABILITIES,}graphics

RUN ( \
   echo 'LogLevel DEBUG2'; \
   echo 'PermitRootLogin yes'; \
   echo 'PasswordAuthentication no'; \
   echo 'X11Forwarding yes'; \
   echo 'Port 1122'; \
 ) | sudo tee -a /etc/ssh/sshd_config.d/sshd_config_test_clion.conf \
 && sudo mkdir /run/sshd

#COPY ./docker/scripts/dev_entrypoint.sh /usr/local/bin
#RUN sudo chmod +x /usr/local/bin/dev_entrypoint.sh
#
#WORKDIR /home/${USER}/${PROJ_NAME}
#
#ENTRYPOINT [ "/usr/local/bin/dev_entrypoint.sh" ]
#CMD ["/bin/bash"]
#
#FROM base as prod
#
#SHELL ["/bin/bash", "-o", "pipefail", "-c"]
#
#WORKDIR /tmp
## We want to install the app, but have to install ~500MB of deps.
## If we just copied the.deb, we'd miss on the docker cache and have to re-install everytime
## So first pass just deps through in text file (which shouldn't change often)
## Then install app.
#COPY --from=dev /home/${USER}/${PROJ_NAME}/build/packages/app.deps .
##hadolint ignore=SC2046
#RUN sudo apt-get update && sudo DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get install -y \
#      $(cat app.deps) \
#    && sudo apt-get clean \
#    && sudo rm -rf /var/lib/apt/lists/*
#
#COPY --from=dev /home/${USER}/${PROJ_NAME}/build/packages/*.deb .
#RUN sudo apt-get update \
#    && sudo apt install -y ./chirp-app_ui*.deb \
#    && sudo apt-get clean && sudo rm -rf /var/lib/apt/lists/* && sudo rm chirp-app_ui*.deb
#
#WORKDIR /home/${USER}/
#RUN bash -c 'echo -e HARDWARE is $HARDWARE'
#
#RUN if [ "$HARDWARE" = "GPU" ]; then \
#      echo "HARDWARE intended for: $HARDWARE"; \
#      echo "export NVIDIA_VISIBLE_DEVICES \${NVIDIA_VISIBLE_DEVICES:-all}" | sudo tee -a /etc/environment && \
#      echo "export NVIDIA_DRIVER_CAPABILITIES \${NVIDIA_DRIVER_CAPABILITIES:+$NVIDIA_DRIVER_CAPABILITIES,}graphics" | sudo tee -a /etc/environment; \
#    fi
#
#ENV NVIDIA_VISIBLE_DEVICES \
#    ${NVIDIA_VISIBLE_DEVICES:-all}
#ENV NVIDIA_DRIVER_CAPABILITIES \
#    ${NVIDIA_DRIVER_CAPABILITIES:+$NVIDIA_DRIVER_CAPABILITIES,}graphics
#
##RUN ( \
##   echo 'LogLevel DEBUG2'; \
##   echo 'PermitRootLogin yes'; \
##   echo 'PasswordAuthentication no'; \
##   echo 'X11Forwarding yes'; \
##   echo 'Port 2222'; \
## ) | sudo tee -a /etc/ssh/sshd_config.d/sshd_config_test_clion.conf \
## && sudo mkdir /run/sshd
#
##COPY ./docker/scripts/prod_entrypoint.sh /usr/local/bin
##RUN sudo chmod +x /usr/local/bin/prod_entrypoint.sh
#
##ENTRYPOINT [ "/usr/local/bin/prod_entrypoint.sh" ]
#CMD ["app_ui"]
