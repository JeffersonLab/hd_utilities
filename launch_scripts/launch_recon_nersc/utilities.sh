# Collection of utility functions for the launch scripts.


source_config_file() {
  # sources the given configuration .env file and ensures that it exists
  local LAUNCH_ENV_FILE="${1}"
  if [ -z "${LAUNCH_ENV_FILE}" ]
  then
    echo "You must provide the configuration .env file as the first argument."
    exit 1
  fi
  if [ ! -f "${LAUNCH_ENV_FILE}" ]
  then
    echo "The provided configuration .env file '${LAUNCH_ENV_FILE}' does not exist."
    exit 1
  fi
  echo "Reading production parameters from .env file '${LAUNCH_ENV_FILE}'"
  source "${LAUNCH_ENV_FILE}"
}
