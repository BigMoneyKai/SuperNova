#!/bin/bash
# build script for supernova testbed
set echo on
red="\033[31m"
green="\033[32m"
reset="\033[0m"
mkdir -p ../bin

cFilenames=$(find . -type f -name "*.c")

assembly="testbed"
compilerFlags="-g -fdeclspec -fPIC"

includeFlags="-Isrc -I../engine/src"
linkerFlags="-L../bin/ -lengine -Wl,-rpath,\$ORIGIN"
defines="-D_DEBUG -DSNIMPORT"

echo -e "$green Building $assembly...$reset"
echo "Build command: clang $cFilenames $compilerFlags -o ../bin/$assembly $defines $includeFlags $linkerFlags"
clang $cFilenames $compilerFlags -o ../bin/$assembly $defines $includeFlags $linkerFlags
echo -e "$green $assembly built successfully.$reset"
