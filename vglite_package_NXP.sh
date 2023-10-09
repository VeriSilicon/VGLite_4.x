#!/bin/bash

# Example: 
#          cd SW/VGLite/Hubi.dev
#          ./release/vglite_package.sh  gc555  0x41A

if [ $# -lt 2 ]; then
    echo
    echo "Usage example:  $0  gc555  0x41A"
    echo
    exit 1
fi

### Create release package

rm -rf package
mkdir package

cp -f  release/*        ./package
cp -f  Makefile*        ./package
cp -rf inc/             ./package
cp -rf VGLite/          ./package
cp -rf VGLiteKernel/    ./package
cp -rf vendor/          ./package

rm -rf ./package/VGLite/Series/gc265
rm -rf ./package/VGLite/Series/gc355/0x40*
rm -rf ./package/VGLite/Series/gc555/0x41A

rm -rf ./package/VGLite/win32
rm -rf ./package/VGLiteKernel/win32

rm -f ./package/vglite_package*.sh

