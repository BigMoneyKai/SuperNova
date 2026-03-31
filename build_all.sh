#!/bin/bash
# build script for rebuilding everything
set echo on
red="\033[31m"
green="\033[32m"
reset="\033[0m"
echo "Building everything..."

echo
cd engine
source build.sh
cd ..

ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "$red Error:$reset"$ERRORLEVEL && exit
fi

echo
cd testbed
source build.sh
cd ..

ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "$red Error:$reset"$ERRORLEVEL && exit
fi

echo
echo "All assemblies built successfully."
