#!/bin/bash
# Convenience script for Android

set -e
scriptDir="$(dirname "$(readlink -f "$0")")"
cd "${scriptDir}"

source ../imagine/tools/androidFuncs.sh

parseCMakeActionArgs "$@"
executeCMakeActions
