
# Building `libvglite.a` for Zephyr Platforms

This guide explains how to generate a standalone `libvglite.a` static library for 
**NXP RT1170**, **RT700**, and **RT500** Zephyr platforms.

---

## Prerequisites

### Install Zephyr SDK manually  
Follow the official Zephyr SDK installation guide.

---

# Build Instructions

---

## Build Instructions for **NXP RT1170**, **NXP RT700**, **NXP RT500**

### A) Generate Zephyr build files

To generate a static library for a specific platform (RT1170, RT700, or RT500), a VG-Lite application must first be built for the corresponding target.

For example, to generate a static library for the RT1170 platform, a VG-Lite application should first be built for an RT1170 board using Zephyr.


### B) Shell script to build static library

```bash
#!/bin/bash

set -e

# Usage function
usage() {
    echo "Usage: $0 [PLATFORM]"
    echo ""
    echo "PLATFORM options:"
    echo "  rt500        - Build for RT500"
    echo "  rt700        - Build for RT700"
    echo "  rt1170       - Build for RT1170 (default)"
    echo ""
    echo "Note: Always builds in Debug mode"
    echo ""
    echo "Examples:"
    echo "  $0                    # Build RT1170 Debug"
    echo "  $0 rt500              # Build RT500 Debug"
    echo "  $0 rt700              # Build RT700 Debug"
    echo ""
    exit 1
}

# Parse command line arguments
BUILD_PLTF=${1:-rt1170}

# Validate platform
case "$BUILD_PLTF" in
    rt500|rt700|rt1170)
        ;;
    -h|--help)
        usage
        ;;
    *)
        echo "ERROR: Invalid platform '$BUILD_PLTF'"
        echo ""
        usage
        ;;
esac

# Always Debug build
BUILD_TYPE=Debug

# Configuration
export ZEPHYR_SDK_DIR=${ZEPHYR_SDK_DIR:-/opt/zephyr-sdk-0.17.1}
export ZEPHYR_BASE=${ZEPHYR_BASE:-/opt/zephyrproject/zephyr}
export ZEPHYR_BUILD_DIR=${ZEPHYR_BUILD_DIR:-/opt/zephyrproject/build}

# Remove trailing slashes
ZEPHYR_BASE="${ZEPHYR_BASE%/}"
ZEPHYR_BUILD_DIR="${ZEPHYR_BUILD_DIR%/}"

BUILD_DIR=__build_debug_${BUILD_PLTF}_zephyr
INSTALL_DIR=$PWD/__install_vglite_${BUILD_PLTF}_zephyr

echo "========================================="
echo "VGLite Zephyr Debug Build"
echo "========================================="
echo "Platform:         ${BUILD_PLTF}"
echo "Build Type:       ${BUILD_TYPE}"
echo "Build Dir:        ${BUILD_DIR}"
echo "Install Dir:      ${INSTALL_DIR}"
echo ""
echo "ZEPHYR_BASE:      $ZEPHYR_BASE"
echo "ZEPHYR_BUILD_DIR: $ZEPHYR_BUILD_DIR"
echo "ZEPHYR_SDK_DIR:   $ZEPHYR_SDK_DIR"
echo "========================================="
echo ""

# Clean
rm -rf "${BUILD_DIR}"
rm -rf "${INSTALL_DIR}"

# Extract flags from compile_commands.json
COMPILE_COMMANDS="${ZEPHYR_BUILD_DIR}/compile_commands.json"

if [ ! -f "$COMPILE_COMMANDS" ]; then
    echo "ERROR: Cannot find compile_commands.json at ${COMPILE_COMMANDS}"
    echo "Please ensure you have built a Zephyr application first"
    exit 1
fi

echo "Extracting compiler flags from compile_commands.json..."

# Extract a complete compile command and parse it
SAMPLE_COMMAND=$(grep -m1 '"command":' "$COMPILE_COMMANDS" | sed 's/.*"command": "\(.*\)".*/\1/')

# Extract flags (everything between compiler and -o flag)
ALL_C_FLAGS=$(echo "$SAMPLE_COMMAND" | sed 's/.*gcc //' | sed 's/ -o .*//' | sed 's/ -c .*//')

echo "Extracted flags (first 200 chars): ${ALL_C_FLAGS:0:200}..."
echo ""

# Set toolchain
TOOLCHAIN_PREFIX=${ZEPHYR_SDK_DIR}/arm-zephyr-eabi/bin/arm-zephyr-eabi-

# Verify source directory exists
SOURCE_DIR="vendor/nxp/${BUILD_PLTF}_zephyr"
if [ ! -d "$SOURCE_DIR" ]; then
    echo "ERROR: Source directory not found: $SOURCE_DIR"
    echo ""
    echo "Available platforms:"
    ls -d vendor/nxp/*_zephyr 2>/dev/null | xargs -n1 basename | sed 's/_zephyr$//' | sed 's/^/  /'
    exit 1
fi

echo "Building from source: $SOURCE_DIR"
echo ""

# Build
cmake \
    -DCMAKE_SYSTEM_NAME=Generic \
    -DCMAKE_SYSTEM_PROCESSOR=arm \
    -DCMAKE_C_COMPILER=${TOOLCHAIN_PREFIX}gcc \
    -DCMAKE_CXX_COMPILER=${TOOLCHAIN_PREFIX}g++ \
    -DCMAKE_AR=${TOOLCHAIN_PREFIX}ar \
    -DCMAKE_RANLIB=${TOOLCHAIN_PREFIX}ranlib \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_C_FLAGS="${ALL_C_FLAGS}" \
    -G "Unix Makefiles" \
    -S ${SOURCE_DIR} \
    -B ${BUILD_DIR}

echo ""
echo "Building with $(nproc) parallel jobs..."
cmake --build ${BUILD_DIR} -j$(nproc)

# Manual install
echo ""
echo "Installing to ${INSTALL_DIR}..."
mkdir -p "${INSTALL_DIR}/lib"
mkdir -p "${INSTALL_DIR}/include"

cp ${BUILD_DIR}/libvglite.a "${INSTALL_DIR}/lib/"
cp -r inc/* "${INSTALL_DIR}/include/"

echo ""
echo "========================================="
echo "=== Debug Build Complete ==="
echo "========================================="
echo "Platform:      ${BUILD_PLTF}"
echo "Build Type:    ${BUILD_TYPE}"
echo "Install Dir:   ${INSTALL_DIR}"
echo ""
echo "========================================="

```



## C) Build Shell Script

Use the `build-gpu-vglite-library.sh` script to build the GPU VGLite static library for the required NXP RT platform.

### RT1170 Board
```bash
gpu-vglite$ ./build-gpu-vglite-library.sh rt1170
```
This will build the VGLite static library for RT1170 in Debug mode and install it to `__install_vglite_rt1170_zephyr/`. 
### RT500 Board 
```bash 
gpu-vglite$ ./build-gpu-vglite-library.sh rt500
```
This will build the VGLite static library for RT500 in Debug mode and install it to `__install_vglite_rt500_zephyr/`.
### RT700 Board
```bash
gpu-vglite$ ./build-gpu-vglite-library.sh rt700
```
This will build the VGLite static library for RT700 in Debug mode and install it to `__install_vglite_rt700_zephyr/`.


---

