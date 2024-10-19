#!/usr/bin/env bash
set -e
set -o allexport

# docker-compose environment variables are passed to ENTRYPOINT and docker exec
# but aren't exported for ssh connections. Need a way to add to ssh connections
echo "DISPLAY: ${DISPLAY}"
echo "Adding to /etc/environment"
echo "export DISPLAY=${DISPLAY}" | sudo tee -a /etc/environment
echo "export QT_DEBUG_PLUGINS=${QT_DEBUG_PLUGINS}" | sudo tee -a /etc/environment

# Creates id_rsa ssh key in ~/.ssh/id_rsa for ssh connection
# docker-compose volume mount provides this to host for seamless connection
mkdir -p ~/.ssh
sudo chmod 700 ~/.ssh
ssh-keygen -q -t rsa -N '' -f ~/.ssh/id_rsa <<<y
sudo chmod 600 ~/.ssh/id_rsa
sudo chmod 644 ~/.ssh/id_rsa.pub
touch ~/.ssh/authorized_keys
cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys

# Want to add ssh_config without blowing away any current config, so include files in config.d
# and just create file
grep -qxF 'Include config.d/*' ~/.ssh/config || echo 'Include config.d/*' >> ~/.ssh/config
mkdir -p ~/.ssh/config.d
( \
   echo "Host localhost"; \
   echo "  User ${USER}"; \
   echo "  IdentityFile ~/.ssh/id_rsa"; \
   echo "  IdentitiesOnly yes"; \
 ) |tee -a ~/.ssh/config.d/aitd2_ui_config

eval "$( fixuid )"

if [ -z "${SSH_PORT}" ]; then
  echo "Leaving Port as 2222";
else
  echo "Setting SSH Port to: ${SSH_PORT}"
  sudo sed -i "s/^Port .*/Port ${SSH_PORT}/g" /etc/ssh/sshd_config.d/sshd_config_test_clion.conf
fi

cat /etc/ssh/sshd_config.d/sshd_config_test_clion.conf

sudo  /usr/sbin/sshd -e &

if [ -d /home/"${USER}"/app_ui/.git ]; then
  # .git exists, so a valid git repo has been mounted
  # Remove old version variables
  unset GIT_REV
  unset GIT_DIFF
  unset GIT_TAG
  unset GIT_BRANCH
fi


echo "Command: $*"

if [ -z "${STARTUP_SLEEP}" ]; then
  echo "Not Sleeping..."
else
  sleep "${STARTUP_SLEEP}"
fi
eval "$@"
