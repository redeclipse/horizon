#!/bin/sh
if [ -z "${HORIZON_PATH+isset}" ]; then HORIZON_PATH="$(cd "$(dirname "$0")" && pwd)"; fi
HORIZON_BINARY="horizon_server"
export HORIZON_BINARY
HORIZON_CALLED="true" . "${HORIZON_PATH}/horizon.sh" $@
