#!/bin/bash

# UEFI build script for NXP LS SoCs
#
# Copyright 2017 NXP
#
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#

# Global Defaults
ARCH=AARCH64
TARGET_TOOLS=`echo $GCC_ARCH_PREFIX | cut -d _ -f 1`
BASE_DIR=../../..

[ -z "$TARGET_TOOLS" ] && {
  echo "TARGET_TOOLS not found. Please run \"source Env.cshrc\" ."
  exit 1
}

print_usage_banner()
{
    echo ""
    echo "This shell script expects:"
    echo "    Arg 1 (mandatory): Board Type (can be LS1043 / LS1046 / LS2088)."
    echo "    Arg 2 (mandatory): Build candidate (can be RELEASE or DEBUG). By
              default we build the RELEASE candidate."
    echo "    Arg 3 (optional): clean - To do a 'make clean' operation."
}

# Check for total num of input arguments
if [[ "$#" -gt 3 ]]; then
  echo "Illegal number of parameters"
  print_usage_banner
  exit
fi

# Check for third parameter to be clean only
if [[ "$3" && $3 != "clean" ]]; then
  echo "Error ! Either clean or emplty"
  print_usage_banner
  exit
fi

# Check for input arguments
if [[ $1 == "" || $2 == "" ]]; then
  echo "Error !"
  print_usage_banner
  exit
fi

# Check for input arguments
if [[ $1 != "LS1043" && $1 != "LS1046" && $1 != "LS2088" ]]; then
  echo "Error ! Incorrect Board Type specified."
  print_usage_banner
  exit
fi

# Check for input arguments
if [[ $2 != "RELEASE" ]]; then
  if [[ $2 != "DEBUG" ]]; then
    echo "Error ! Incorrect build target specified."
    print_usage_banner
    exit
  fi
fi

PKG="aRdbPkg"

echo ".........................................."
echo "Welcome to $1$PKG UEFI Build environment"
echo ".........................................."

if [[ $3 == "clean" ]]; then
  echo "Cleaning up the build directory '$BASE_DIR/Build/$1$PKG/'.."
  rm -rf $BASE_DIR/Build/$1$PKG/*
  exit
fi

# Clean-up
set -e
shopt -s nocasematch

#
# Setup workspace now
#
echo Initializing workspace
cd $BASE_DIR

# Use the BaseTools in edk2
export EDK_TOOLS_PATH=`pwd`/BaseTools
source edksetup.sh BaseTools


build -p "$WORKSPACE/edk2-platforms/Platform/NXP/$1$PKG/$1$PKG.dsc" -a $ARCH -t $TARGET_TOOLS -b $2
