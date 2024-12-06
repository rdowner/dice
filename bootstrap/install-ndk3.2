#! /usr/bin/env bash

# This script installs the required files from the Amiga Native Developer Kit (NDK) version 3.2, into the DICE directory tree.

set -euo pipefail

# Parse options - source: https://stackoverflow.com/a/14203146/1031460

POSITIONAL_ARGS=()
SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
BASE_DIR="$(dirname "$SCRIPT_DIR")"

while [[ $# -gt 0 ]]; do
  case $1 in
    -d|--dice)
      BASE_DIR="$2"
      shift # past argument
      shift # past value
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      POSITIONAL_ARGS+=("$1") # save positional arg
      shift # past argument
      ;;
  esac
done

# Check we have exactly one positional argument: the path to the NDK
if [[ ${#POSITIONAL_ARGS[@]} -ne 1 ]]; then
  echo "Usage: $0 [ --dice <override path of DICE directory> ] <path to Amiga NDK 3.2>"
  exit 1
fi
NDK_DIR="${POSITIONAL_ARGS[0]}"
DINCLUDE="${BASE_DIR}/include"
DLIB="${BASE_DIR}/dlib"

which rsync > /dev/null || { echo "rsync required by this script but is not found"; exit 1; }

rsync \
  --verbose --archive --delete \
  "${NDK_DIR}/Include_H/" "${DINCLUDE}/amiga32"

rsync \
  --verbose --archive \
  "${NDK_DIR}/FD/" "${DINCLUDE}/amiga32/fd"

rsync \
  --verbose --archive \
  "${NDK_DIR}/SFD/" "${DINCLUDE}/amiga32/sfd"

mkdir -p "${DLIB}"
rsync \
  --verbose --archive \
  "${NDK_DIR}/lib/amiga.lib" "${DLIB}/amiga32l.lib"
