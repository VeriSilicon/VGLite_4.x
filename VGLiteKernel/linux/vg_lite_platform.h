/****************************************************************************
*
*    Copyright (c) 2014 - 2022 Vivante Corporation
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/
#ifndef _vg_lite_platform_h_
#define _vg_lite_platform_h_

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/clk.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/reset.h>
#include <linux/regulator/consumer.h>
#include "vg_lite_kernel.h"
#include "vg_lite_hal.h"
#include "vg_lite_debug.h"
#include "vg_lite_type.h"
#include "vg_lite_option.h"
#ifdef ENABLE_PCIE
#include <linux/pci.h>
#endif
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rtc.h>

#define VG_DEVICE_NAME          "vg_lite"
#define VG_SYSTEM_RESERVE_COUNT 1         /* Set according to the reserved memory number */

/* Struct definitions. */
struct heap_node {
    struct list_head list;
    uint32_t offset;
    unsigned long size;
    int32_t status;
    vg_lite_vidmem_pool_t pool;
};

struct memory_heap {
    uint32_t free;
    struct list_head list;
};

struct vg_lite_device {
    void *register_base_mapped;             /* Register memory base */
    ulong register_mem_base;
    uint register_mem_size;
    ulong contiguous_base;
    uint contiguous_size;
    ulong contiguous_bases[VG_SYSTEM_RESERVE_COUNT];
    void *contiguous_bases_logical[VG_SYSTEM_RESERVE_COUNT];
    uint contiguous_sizes[VG_SYSTEM_RESERVE_COUNT];
    uint irq_line;
    struct page *pages;
    unsigned int order;
    void *virtual[VG_SYSTEM_RESERVE_COUNT];
    uint64_t physical[VG_SYSTEM_RESERVE_COUNT];
    uint32_t size[VG_SYSTEM_RESERVE_COUNT];
    struct memory_heap heap[VG_SYSTEM_RESERVE_COUNT];
    int irq_enabled;
    volatile uint32_t int_flags;
    wait_queue_head_t int_queue;
    void *device;
    struct device *dev;
    struct platform_device *pdev;
    int registered;
    int major;
    struct class *class;
    int created;
#ifdef ENABLE_PCIE
    struct pci_dev *p_dev;
    int pci_registered;
#endif
#ifdef CONFIG_DEBUG_FS
    struct dentry *root;
    uint32_t start_pm;
#endif
    struct rtc_device *rtc;
    uint32_t pm_count;
    vg_lite_gpu_execute_state_t gpu_execute_state;
};

typedef struct vg_module_parameters
{
    uint     irq_line;

    ulong    register_mem_base;
    uint     register_mem_size;

    ulong    contiguous_base;
    uint     contiguous_size;

    ulong    contiguous_bases[VG_SYSTEM_RESERVE_COUNT];
    uint     contiguous_sizes[VG_SYSTEM_RESERVE_COUNT];
}
vg_module_parameters_t;


typedef struct vg_platform vg_platform_t;

typedef struct vg_linux_operations
{
    /*******************************************************************************
    **
    **  adjust_param
    **
    **  Override content of arguments, if a argument is not changed here, it will
    **  keep as default value or value set by insmod command line.
    */
    int
    (*adjust_param)(
        vg_platform_t *platform,
        vg_module_parameters_t *args
        );

    /*******************************************************************************
    **
    **  get_power
    **
    **  Prepare power and clock operation.
    */
    int
    (*get_power)(
        vg_platform_t *platform
        );

    /*******************************************************************************
    **
    **  set_power
    **
    **  Set power state of specified GPU.
    **
    **  INPUT:
    **
    **      n2d_int32_t GPU
    **          GPU neeed to config.
    **
    **      gceBOOL Enable
    **          Enable or disable power.
    */
    int
    (*set_power)(
        vg_platform_t *platform,
        vg_lite_bool_t enable
        );

        /*******************************************************************************
    **
    **  put_power
    **
    **  Put power of specified GPU.
    **
    **  INPUT:
    **
    **      n2d_int32_t GPU
    **          GPU neeed to config.
    **
    **      gceBOOL Enable
    **          Enable or disable power.
    */
    int
    (*put_power)(
        vg_platform_t *platform
        );

    /*******************************************************************************
    **
    **  set_clock
    **
    **  Set clock state of specified GPU.
    **
    **  INPUT:
    **
    **      n2d_int32_t GPU
    **          GPU neeed to config.
    **
    **      gceBOOL Enable
    **          Enable or disable clock.
    */
    int
    (*set_clock)(
        vg_platform_t *platform,
        vg_lite_bool_t enable
        );

    /*******************************************************************************
    **
    **  adjustDriver
    **
    **  Override content of platform_driver which will be registered.
    */
    int
    (*adjust_driver)(
        vg_platform_t *platform);
}
vg_linux_operations_t;

struct vg_platform
{
    struct platform_device *device;
    struct platform_driver *driver;
    struct vg_lite_device  *vg_device;

    const char             *name;
    vg_linux_operations_t  *ops;
    void                   *priv;
};

int vg_kernel_platform_init(struct platform_driver *pdrv, vg_platform_t **platform);
int vg_kernel_platform_terminate(vg_platform_t *platform);

#endif
