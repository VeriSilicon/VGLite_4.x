##############################################################################
#
#    The MIT License (MIT)
#
#    Copyright (c) 2014 - 2026 Vivante Corporation
#
#    Permission is hereby granted, free of charge, to any person obtaining a
#    copy of this software and associated documentation files (the "Software"),
#    to deal in the Software without restriction, including without limitation
#    the rights to use, copy, modify, merge, publish, distribute, sublicense,
#    and/or sell copies of the Software, and to permit persons to whom the
#    Software is furnished to do so, subject to the following conditions:
#
#    The above copyright notice and this permission notice shall be included in
#    all copies or substantial portions of the Software.
#
#    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#    DEALINGS IN THE SOFTWARE.
#
##############################################################################


OBJS = linux/vg_lite_hal.o vg_lite_kernel.o linux/platform/$(PLATFORM).o

ifeq ($(DEBUG), 1)
ccflags-y += -g -O0 -DDEBUG -D_DEBUG
else
ccflags-y += -O2 -Werror
endif

ifeq ($(ENABLE_PCIE), 1)
ccflags-y += -DENABLE_PCIE
endif

ifeq ($(ENABLE_PCIE_EMU), 1)
EXTRA_CFLAGS += -DUSE_LINUX_PCIE_EMU
endif

ifeq ($(gcdIRQ_SHARED), 1)
ccflags-y += -DgcdIRQ_SHARED
endif

ifeq ($(USE_RESERVE_MEMORY), 1)
ccflags-y += -DUSE_RESERVE_MEMORY
endif

ifeq ($(KERNELRELEASE),)

ARCH ?= arm
KERNEL_DIR ?= /opt/linux-2.6.38-imx
CROSS_COMPILE ?= ~/CodeSourcery/bin/arm-none-linux-gnueabi-
PWD = $(shell pwd)
OUT = ../bin

all: $(OUT)
	@$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) V=$(V) modules
	@cp vg_lite.ko $(OUT)

clean:
	@rm -fr $(OBJS) *.o vg_lite.ko .*.cmd modules.order Module.symvers vg_lite.mod.c .tmp_versions $(OUT)/vg_lite.ko
	@rm -f ./linux/.*.cmd

$(OUT):
	@mkdir -p $(OUT)

else

obj-m := vg_lite.o
vg_lite-objs := $(OBJS)

ccflags-y += -I$(M)/../inc -I$(M)/linux -I$(M) -I$(M)/../VGLite/linux

endif
