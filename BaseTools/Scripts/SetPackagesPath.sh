
#
# Copyright (c) 2020, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
# This script calls GetPackagesPath.py to collect all package paths under
# specified directories and append them to PACKAGES_PATH environment
# variable. A sub directory is a qualified package path when an EDKII
# Package can be found under it.
#
# Note: This script must be \'sourced\' so the environment can be changed:
# source SetPackagesPath.sh
# . SetPackagesPath.sh

function Usage()
{
    echo "Usage: source SetPackagesPath.sh directory [directory ...]"
    echo "Copyright(c) 2020, Intel Corporation. All rights reserved."
    echo "Options:"
    echo "  --help, -h     Print this help screen and exit"
    echo "Please note: This script must be \'sourced\' so the environment can be changed."
    echo ". SetPackagesPath.sh"
    echo "source SetPackagesPath.sh"
}

function SetEnv()
{
    local paths=$(python $EDK_TOOLS_PATH/Scripts/GetPackagesPath.py $@)
    if [ "$PACKAGES_PATH" ]; then
        PACKAGES_PATH=$PACKAGES_PATH:$paths
    else
        PACKAGES_PATH=$paths
    fi
}

if [ $# -eq 0 -o "$1" == "-h" -o "$1" == "--help" -o "$1" == "/?" ]; then
    Usage
else
    SetEnv $@
fi
