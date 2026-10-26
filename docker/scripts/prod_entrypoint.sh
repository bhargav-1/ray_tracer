#!/usr/bin/env bash
set -e
set -o allexport

# docker-compose environment variables are passed to ENTRYPOINT and docker exec
# but aren't exported for ssh connections. Need a way to add to ssh connections
echo "DISPLAY: ${DISPLAY}"
echo "Adding to /etc/environment"
echo "export DISPLAY=${DISPLAY}" | sudo tee -a /etc/environment
echo "export QT_DEBUG_PLUGINS=${QT_DEBUG_PLUGINS}" | sudo tee -a /etc/environment

eval "$( fixuid )"
echo "Command: $*"

if [ -z "${STARTUP_SLEEP}" ]; then
  echo "Not Sleeping..."
else
  sleep "${STARTUP_SLEEP}"
fi
exec "$@"
