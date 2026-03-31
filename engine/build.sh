#!/bin/bash
# build script for supernova engine
set echo on
red="\033[31m"
green="\033[32m"
reset="\033[0m"

mkdir -p ../bin

cFilenames=$(find . -type f -name "*.c")

assembly="engine"
compilerFlags="-g -shared -fdeclspec -fPIC"

includeFlags="-Isrc -I$VULKAN_SDK/include"
linkerFlags="-lvulkan -lxcb -lX11-xcb -lxkbcommon -L$VULKAN_SDK/lib -L/usr/X11R6/lib"
defines="-D_DEBUG -DSNEXPORT"

echo -e "$green Building $assembly...$reset"
echo -e " Build command: clang $cFilenames $compilerFlags -o ../bin/lib$assembly.so $defines $includeFlags $linkerFlags"
clang $cFilenames $compilerFlags -o ../bin/lib$assembly.so $defines $includeFlags $linkerFlags
echo -e "$green $assembly built successfully.$reset"
