Build instructions for RT1170
=============================

1. Download and extract the [mimxRT1170-SDK](https://mcuxpresso.nxp.com/)
2. Download and extract the [Arm GNU MPACBTI Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
3. Set MCU_SDK_DIR and ARMGCC_DIR to point to your mimxRT1170-SDK and Toolchain paths:
4. Set the correct paths for MCU_SDK_DIR and ARMGCC_DIR in build_rt1170.sh.

```
export MCU_SDK_DIR="path-to-your/SDK_2.13.10_MIMXRT1176xxxxx"
export ARMGCC_DIR="path-to-your/Armgcc/gcc-arm-none-eabi-10-2020-q4-major/"

BUILD_TYPE=debug
BUILD_DIR=__build_${BUILD_TYPE}_rt1170
INSTALL_DIR=$PWD/__install_${BUILD_TYPE}_rt1170

rm -rf "${BUILD_DIR}"
rm -rf "${INSTALL_DIR}"

cmake                                 \
    -DCMAKE_TOOLCHAIN_FILE="${MCU_SDK_DIR}/tools/cmake_toolchain_files/armgcc.cmake" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}  \
    -DVGLITE_PLATFORM=rt1170          \
    -G "Unix Makefiles"               \
    -B ${BUILD_DIR}

cmake --build ${BUILD_DIR}
cmake --install ${BUILD_DIR} --prefix "${INSTALL_DIR}"
```



Build instructions for RT500
=============================

1. Download and extract the [mimxRT500-SDK](https://mcuxpresso.nxp.com/)
2. Download and extract the [Arm GNU MPACBTI Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
3. Set MCU_SDK_DIR and ARMGCC_DIR to point to your mimxRT500-SDK and Toolchain paths:
4. Set the correct paths for MCU_SDK_DIR and ARMGCC_DIR in build_rt500.sh.

```
export MCU_SDK_DIR="path-to-your/SDK_2_13_1_EVK-MIMXRT595"
export ARMGCC_DIR="path-to-your/Armgcc/gcc-arm-none-eabi-10-2020-q4-major/"


BUILD_TYPE=debug
BUILD_DIR=__build_${BUILD_TYPE}_rt500
INSTALL_DIR=$PWD/__install_${BUILD_TYPE}_rt500

rm -rf "${BUILD_DIR}"
rm -rf "${INSTALL_DIR}"

cmake                                 \
    -DCMAKE_TOOLCHAIN_FILE="${MCU_SDK_DIR}/tools/cmake_toolchain_files/armgcc.cmake" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}  \
    -DVGLITE_PLATFORM=rt500           \
    -G "Unix Makefiles"               \
    -B ${BUILD_DIR}

cmake --build ${BUILD_DIR}
cmake --install ${BUILD_DIR} --prefix "${INSTALL_DIR}"
```






Build instructions for RT700
=============================

1. Download and extract the [mimxRT700-SDK](https://mcuxpresso.nxp.com/)
2. Download and extract the [Arm GNU MPACBTI Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
3. Set MCU_SDK_DIR and ARMGCC_DIR to point to your mimxRT700-SDK and Toolchain paths:
4. Set the correct paths for MCU_SDK_DIR and ARMGCC_DIR in build_rt500.sh.

```
export MCU_SDK_DIR="path-to-your/SDK_2_16_1_EVK-MIMXRT798"
export ARMGCC_DIR="path-to-your/Armgcc/gcc-arm-none-eabi-10-2020-q4-major/"


BUILD_TYPE=debug
BUILD_DIR=__build_${BUILD_TYPE}_rt700
INSTALL_DIR=$PWD/__install_${BUILD_TYPE}_rt700

rm -rf "${BUILD_DIR}"
rm -rf "${INSTALL_DIR}"

cmake                                 \
    -DCMAKE_TOOLCHAIN_FILE="${MCU_SDK_DIR}/tools/cmake_toolchain_files/armgcc.cmake" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}  \
    -DVGLITE_PLATFORM=rt700           \
    -G "Unix Makefiles"               \
    -B ${BUILD_DIR}

cmake --build ${BUILD_DIR}
cmake --install ${BUILD_DIR} --prefix "${INSTALL_DIR}"