#! /bin/bash


export MCU_SDK_DIR="/home/adrian/workspace/Nxp/Vivante/SDK_2.13.10_MIMXRT1176xxxxx"
export ARMGCC_DIR="/home/adrian/workspace/Armgcc/gcc-arm-none-eabi-10-2020-q4-major/"


BUILD_PLTF=rt1170
BUILD_TYPE=debug
BUILD_DIR=__build_${BUILD_TYPE}_${BUILD_PLTF}
INSTALL_DIR=$PWD/__install_${BUILD_TYPE}_${BUILD_PLTF}

rm -rf "${BUILD_DIR}"
rm -rf "${INSTALL_DIR}"

cmake                                 \
    -DCMAKE_TOOLCHAIN_FILE="${MCU_SDK_DIR}/tools/cmake_toolchain_files/armgcc.cmake" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}  \
    -DVGLITE_PLATFORM=${BUILD_PLTF}   \
    -G "Unix Makefiles"               \
    -B ${BUILD_DIR}

cmake --build ${BUILD_DIR}
cmake --install ${BUILD_DIR} --prefix "${INSTALL_DIR}"
