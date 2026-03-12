#!/usr/bin/env bash
set -euo pipefail

# Compute a hash for each file in a directory tree and print hash and
# path sorted by path. Use if a tool such as hashdeep is not
# available.


ROOT_DIR="${1:-.}"  # root directory to hash (default is current directory)

HASHER="sha256sum"  # choose algorithm

find "${ROOT_DIR}" -type f -print0 |  # find all regular files in tree
sort -z |  # sort by path to ensure deterministic order
while IFS= read -r -d '' FILE
do
  "${HASHER}" "${FILE}"  # hash each file and print hash and path
done
