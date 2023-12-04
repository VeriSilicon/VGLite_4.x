#!/bin/bash

# Example: 
#          cd SW/VGLite/Hubi.dev/release
#          ./vglite_Cmodel_package.sh  gc265  0x425  Vivante_VGLite_Bin_Cmodel_0x425_4.0.51  Vivante_VGLite_Src_drv_gc265_0x425_4.0.51

if [ $# -lt 4 ]; then
    echo
    echo "Usage example:  $0  gc265  0x425 Vivante_VGLite_Bin_Cmodel_0x425_4.0.51 Vivante_VGLite_Src_drv_gc265_0x425_4.0.51"
    echo
    exit 1
fi

### Copy the chip specific header file

cp -f ../VGLite/Series/$1/$2/vg_lite_options.h  ../VGLite/


### Create release package

rm -rf ./../../../../$4
mkdir  ./../../../../$4
mkdir  ./../../../../$4/BUILD_vgHuBi

cp -f  ./*                                 ../../../../$4
cp -f  ../Makefile*                        ../../../../$4
cp -f  ../VGLite.def                       ../../../../$4
cp -f  ./patch/0x425/VGLite*               ../../../../$4
cp -f  ./patch/0x425/gc265_vs2019_425.bat  ../../../../$4
cp -rf ../inc/                             ../../../../$4
cp -rf ../VGLite/                          ../../../../$4
cp -rf ../VGLiteKernel/                    ../../../../$4
cp -rf ../../../../$3/$3/*                 ../../../../$4/BUILD_vgHuBi


rm -rf ../../../../$4/VGLite/Series
rm -rf ../../../../$4/VGLiteKernel/win32

rm -f  ../../../../$4/*.sh
rm -rf ../../../../$4/patch
cp -f  ../release/build_linux.sh            ../../../../$4



