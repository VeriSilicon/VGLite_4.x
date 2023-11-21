#! /bin/bash


export MCU_SDK_DIR="/mnt/c/Users/user/Desktop/sdk_rt700"
export ARMGCC_DIR="/mnt/c/Users/user/Desktop/gcc-arm-none-eabi-10.3-2021.10"


BUILD_PLTF=rt700
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
