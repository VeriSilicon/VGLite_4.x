#!/bin/bash
                                                                                               
usage()
{
    echo
    if [ -d VGLite/Series ];then
        echo "Usage: $0 <board> <cid>"
    else
        echo "Usage: $0 <board>"
    fi
    echo
    echo "<board>       Board config: X86/ARM/IMX6Q35"
    if [ -d VGLite/Series ];then
        cid_list=$(find VGLite/Series/*/* -type d -name '0x*' | xargs -I {} basename {} | paste -sd '/')
        echo "<cid>         Support customer ID: ${cid_list}"
        echo
        echo "Eg: $0 X86 0x430"
        echo
    else
        echo
        echo "Eg: $0 X86"
        echo
    fi
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h) 
            usage
            exit 0
        ;;
        *) 
            BOARD=$1
            shift
            cid=$1
            shift
        ;;
    esac
done

####  Set VG HW Series/Chip names here ####

if [ -z "${BOARD}" ];then
    echo "[Error] Must set board config"
    usage
    exit 1
fi

if [ -d VGLite/Series ];then
    if [ -z "${cid}" ];then
        echo "[Error] Must set Customer ID"
        usage
        exit 1
    fi

    cid_dir=$(find VGLite/Series -type d -name ${cid})
    if [ -z "${cid_dir}" ];then
        echo "[Error] Can not found option directory of ${cid}"
        usage
        exit 1
    fi
    cp -f ${cid_dir}/* VGLite/ || exit $?
fi

case "$BOARD" in

ARM)
    export SDK_DIR=`pwd`/../build.s2c/sdk
    export CROSS_COMPILE=/home/software/Linux/arm-vivante-linux-gnueabihf/bin/arm-vivante-linux-gnueabihf-
    export KERNEL_DIR=/home/software/Linux/linux-kernel
    export CPU_ARCH=armv7-a
    export ARCH=arm
    export ENABLE_PCIE=0
    export gcdIRQ_SHARED=1
    export USE_RESERVE_MEMORY=1
    export BACKUP_COMMAND=0
    export PLATFORM=vivante/vg_lite_platform_default
;;

X86)
    export SDK_DIR=`pwd`/../build.s2c/sdk
    export TOOLCHAIN=/usr
    export CROSS_COMPILE=""
    export KERNEL_DIR=/home/software/Linux/x86_pcie/linux-headers-4.8.0-41-generic/
    export ENABLE_PCIE=1
    export CPU_ARCH=0
    export ARCH=x86
    export gcdIRQ_SHARED=1
    export USE_RESERVE_MEMORY=1
    export BACKUP_COMMAND=0
    export PLATFORM=vivante/vg_lite_platform_default
;;

IMX6Q35)
    export SDK_DIR=`pwd`/../build.s2c/sdk
    export KERNEL_DIR=/home/software/Linux/freescale/L5.15.52_RC2_20220919/Kernel/32/linux-lts-nxp
    export TOOLCHAIN=/home/software/Linux/freescale/L5.15.52_RC2_20220919/Toolchain/32/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi
    export CROSS_COMPILE=/home/software/Linux/freescale/L5.15.52_RC2_20220919/Toolchain/32/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
    export CPU_TYPE=cortex-a9
    export CPU_ARCH=0
    export ARCH_TYPE=arm
    export ARCH=arm
    export ENABLE_PCIE=0
    export USE_RESERVE_MEMORY=0
    export gcdIRQ_SHARED=1
    export BACKUP_COMMAND=0
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
