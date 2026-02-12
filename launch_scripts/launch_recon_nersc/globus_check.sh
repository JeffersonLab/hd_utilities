#!/usr/bin/env bash

source globus_collections.sh

echo "--- Globus user identities"
globus whoami --verbose
echo "--- Globus session"
globus session show
echo

for COLLECTION_NAME in "${!GLOBUS_COLLECTIONS[@]}"
do
  COLLECTION_UUID="${GLOBUS_COLLECTIONS[${COLLECTION_NAME}]}"
  echo "--- Printing information for collection '${COLLECTION_NAME}' with UUID = ${COLLECTION_UUID}"
  globus gcs collection show "${COLLECTION_UUID}"
  if [[ ${?} -eq 0 ]]  # previous command was successful
  then
    echo -e "\nContent of root directory:"
    globus ls "${COLLECTION_UUID}"
  fi
  echo
done
