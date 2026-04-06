#!/usr/bin/env bash

# consent to all necessary Globus scopes for the collections defined in `globus_collections.sh`


THIS_SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"  # get the directory of this script
GLOBUS_COLLECTIONS="${THIS_SCRIPT_DIR}/globus_collections.sh"
echo "Reading Globus collections from '${GLOBUS_COLLECTIONS}'"
source "${GLOBUS_COLLECTIONS}"

echo "--- Globus user identities"
globus whoami --verbose
echo "--- Globus session"
globus session show
echo

for COLLECTION_NAME in "${!GLOBUS_COLLECTIONS[@]}"
do
  COLLECTION_UUID="${GLOBUS_COLLECTIONS[${COLLECTION_NAME}]}"
  ENDPOINT_UUID=$(globus api transfer GET "/endpoint/${COLLECTION_UUID}" --jq 'non_functional_endpoint_id || host_endpoint_id' --format unix)
  echo "--- Consent to 'transfer:all', 'data_access', and 'manage_collection' for collection '${COLLECTION_NAME}' with UUID = ${COLLECTION_UUID} and endpoint UUID = ${ENDPOINT_UUID}"
  globus gcs collection show "${COLLECTION_UUID}" > /dev/null 2>&1  # run silently to test whether consent is needed
  if [[ ${?} -eq 0 ]]  # previous command was successful
  then
    echo "Consent was already given"
  else
    globus session consent \
      'urn:globus:auth:scope:transfer.api.globus.org:all[*https://auth.globus.org/scopes/'"${COLLECTION_UUID}"'/data_access]' \
      'urn:globus:auth:scope:'"${ENDPOINT_UUID}"':manage_collections[*https://auth.globus.org/scopes/'"${COLLECTION_UUID}"'/data_access]'
  fi
  echo
done
