/****************************************************************************
*
*    Copyright (c) 2005 - 2023 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
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
#ifdef ENABLE_PCIE
#include <linux/pci.h>
#endif

#define VG_DEVICE_NAME "vg_lite"

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
    uint irq_line;
    struct page *pages;
    unsigned int order;
    void *virtual;
    uint64_t physical;
    uint32_t size;
    struct memory_heap heap;
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
};

typedef struct vg_module_parameters
{
    uint     irq_line;

    ulong    register_mem_base;
    uint     register_mem_size;

    ulong    contiguous_base;
    uint     contiguous_size;
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
