#!/bin/bash
                                                                                               
usage()
{
    echo
    echo "Usage: $0 BOARD ChipID CID"
    echo
    echo "  BOARD: X86 PCIE-GEN6 ZC702 IMX6Q35"
    echo
    echo "  Example:  ./build_linux.sh X86 gc555 0x41A"
    echo
}

if [ $# -lt 3 ]; then
    usage
    exit 1
fi


####  Set VG HW Series/Chip names here ####

export Series=$2
export Chip=$3

cp -f VGLite/Series/${Series}/${Chip}/vg_lite_options.h VGLite/

BOARD=$1
case "$BOARD" in

ZC702)
    export SDK_DIR=`pwd`/../build.s2c/sdk
    export CROSS_COMPILE=/home/software/Linux/zync/arm-vivante-linux-gnueabihf/bin/arm-vivante-linux-gnueabihf-
    export KERNEL_DIR=/home/software/Linux/zync/git/linux-s2c
    export CPU_ARCH=armv7-a
    export ARCH=arm
    export ENABLE_PCIE=0
    export USE_RESERVE_MEMORY=1
    export BACKUP_COMMAND=0
    export gcdIRQ_SHARED=1
    export CC=${CROSS_COMPILE}gcc
    export PLATFORM=vivante/vg_lite_platform_default
;;

PCIE-GEN6)
    export SDK_DIR=`pwd`/../build.s2c/sdk
    export TOOLCHAIN=/usr
    export CROSS_COMPILE=""
    export KERNEL_DIR=/home/software/Linux/x86_pcie/linux-headers-4.8.0-41-generic/
    export ENABLE_PCIE=1
    export BACKUP_COMMAND=0
    export USE_RESERVE_MEMORY=1
    export CPU_ARCH=0
    export ARCH=x86
    export gcdIRQ_SHARED=1
    export PLATFORM=vivante/vg_lite_platform_default
;;

X86)
    export SDK_DIR=./build
    export TOOLCHAIN=/usr
    export CROSS_COMPILE=""
    export KERNEL_DIR=/home/software/Linux/x86_pcie/linux-headers-4.8.0-41-generic/
    export ENABLE_PCIE=1
    export BACKUP_COMMAND=0
    export USE_RESERVE_MEMORY=1
    export CPU_ARCH=0
    export ARCH=x86
    export gcdIRQ_SHARED=1
    export PLATFORM=vivante/vg_lite_platform_default
;;

X86_51510)
    export SDK_DIR=./build
    export TOOLCHAIN=/usr
    export CROSS_COMPILE=""
    export KERNEL_DIR=/home/software/Linux/x86_pcie/linux-5.15.10/
    export ENABLE_PCIE=1
    export BACKUP_COMMAND=0
    export USE_RESERVE_MEMORY=1
    export CPU_ARCH=0
    export ARCH=x86
    export gcdIRQ_SHARED=1
    export PLATFORM=vivante/vg_lite_platform_default
;;

IMX6Q35)
    export SDK_DIR=./build
    export KERNEL_DIR=/home/software/Linux/freescale/L5.15.52_RC2_20220919/Kernel/32/linux-lts-nxp
    export TOOLCHAIN=/home/software/Linux/freescale/L5.15.52_RC2_20220919/Toolchain/32/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi
    export CROSS_COMPILE=/home/software/Linux/freescale/L5.15.52_RC2_20220919/Toolchain/32/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
    export CPU_TYPE=cortex-a9
    export CPU_ARCH=0
    export ARCH_TYPE=arm
    export ARCH=arm
    export ENABLE_PCIE=0
    export USE_RESERVE_MEMORY=0
    export BACKUP_COMMAND=0
    export gcdIRQ_SHARED=1
    export PLATFORM=freescale/vg_lite_platform_imx6
    export SYSROOTFS=/home/software/Linux/freescale/L5.15.52_RC2_20220919/Toolchain/32/sysroots/cortexa9t2hf-neon-poky-linux-gnueabi
    export ROOTFS_USR=$SYSROOTFS/usr
    export CFLAGS="-D__ARM_PCS_VFP --sysroot=$SYSROOTFS"
    export PFLAGS="--sysroot=$SYSROOTFS"
    export LDFLAGS="--sysroot=$SYSROOTFS"

    source  /home/software/Linux/freescale/L5.15.52_RC2_20220919/Toolchain/32/environment-setup-cortexa9t2hf-neon-poky-linux-gnueabi
    export YOCTO_BUILD=1
;;
*)
    echo
    echo "ERROR: Unknown [ $BOARD ], or not support so far."
    usage
;;
esac;

make clean
make install

