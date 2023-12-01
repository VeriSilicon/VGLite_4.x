#!/bin/bash

# Example: 
#          cd SW/VGLite/Hubi.dev
#          ./release/vglite_Cmodel_package.sh  gc265  0x425

if [ $# -lt 2 ]; then
    echo
    echo "Usage example:  $0  gc265  0x425"
    echo
    exit 1
fi

### Copy the chip specific header file

cp -f VGLite/Series/$1/$2/vg_lite_options.h  VGLite/


### Create release package

rm -rf package
mkdir package

cp -f  release/*        ./package
cp -f  Makefile*        ./package
cp -rf inc/             ./package
cp -rf VGLite/          ./package
cp -rf VGLiteKernel/    ./package
cp -rf VGLite.def       ./package

rm -rf ./package/VGLite/Series
rm -rf ./package/VGLiteKernel/win32

rm -f ./package/*.sh
cp -f  release/build_linux.sh        ./package


