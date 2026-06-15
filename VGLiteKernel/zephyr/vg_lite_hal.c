/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * Copyright 2026 NXP
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vg_lite_platform.h"
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/sys/util.h>
#include <zephyr/init.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>

#include <vg_lite_kernel.h>
#include <vg_lite_hal.h>
#include <vg_lite_hw.h>
#include <vg_lite.h>

#define VGLITE_GPU_NODE         DT_NODELABEL(gpu2d)
#define VGLITE_GPU_BASE         DT_REG_ADDR(VGLITE_GPU_NODE)
#define VGLITE_GPU_IRQN         DT_IRQN(VGLITE_GPU_NODE)
#define CLOCK_ID_GPU            DT_CLOCKS_CELL_BY_IDX(VGLITE_GPU_NODE, 0, name)

#define VGLITE_TESS_H                   CONFIG_VGLITE_TESS_HEIGHT
#define VGLITE_TESS_W                   CONFIG_VGLITE_TESS_WIDTH
#define VGLITE_COMMAND_BUF_SIZE         CONFIG_VGLITE_CMD_BUF_SIZE
#define VGLITE_CONTIGUOUS_AREA_ALIGN    CONFIG_VGLITE_CONTIGUOUS_ALIGN

#ifdef CONFIG_VGLITE_HEAP_USE_CUSTOM_SECTION
#define ATTRIBUTE_VG_LITE_HEAP __attribute__((section(".vg_lite_heap")))
#else
#define ATTRIBUTE_VG_LITE_HEAP
#endif

LOG_MODULE_REGISTER(GPU, LOG_LEVEL_INF);

#undef VGLITE_MEM_ALIGNMENT
#define VGLITE_MEM_ALIGNMENT 64
#define VGLITE_ATTRIBUTE_MEM_ALIGN __aligned(VGLITE_MEM_ALIGNMENT)

static char __nocache vg_lite_heap_mem[CONFIG_VG_LITE_K_MEM_POOL_SIZE]
  VGLITE_ATTRIBUTE_MEM_ALIGN
  ATTRIBUTE_VG_LITE_HEAP;

struct vg_lite_dev_data {
    struct sys_heap heap;
    struct k_spinlock heap_lock;
    struct k_sem wait_sem;

    volatile uint32_t int_flags;

    vg_lite_gpu_execute_state_t gpu_execute_state;
    int8_t gpu_clken_cnt;

    uint32_t register_base;
    void *device;
};


static struct vg_lite_dev_data * gp_dev_data = NULL;

void vg_lite_set_gpu_clock_state(int enabled);
extern void clear_cache_op(void);

void *vg_lite_os_malloc(size_t size)
{
    if (!gp_dev_data)
        return NULL;

    k_spinlock_key_t key;

    key = k_spin_lock(&gp_dev_data->heap_lock);
    void *ptr = sys_heap_alloc(&gp_dev_data->heap, size);
    k_spin_unlock(&gp_dev_data->heap_lock, key);
    if (!ptr) {
        printk("VG Lite heap out of memory! Requested %zu bytes\n", size);
    }

    return ptr;
}

void vg_lite_os_free(void *memory)
{
    k_spinlock_key_t key;

    if (gp_dev_data && memory) {
        key = k_spin_lock(&gp_dev_data->heap_lock);
        sys_heap_free(&gp_dev_data->heap, memory);
        k_spin_unlock(&gp_dev_data->heap_lock, key);
    }
}

#if gcdVG_ENABLE_DEBUG
static char g_log_buffer[128];
#endif

void vg_lite_hal_delay(uint32_t milliseconds)
{
    /* cannot sleep during suspend/resume */
    k_busy_wait(milliseconds);
}

void vg_lite_hal_barrier(void)
{
    /* flush the write buffer for uncache and write through memory */
    clear_cache_op();
}

void vg_lite_hal_initialize(void)
{
    /* Turn on the clock. */
    vg_lite_set_gpu_clock_state(1);

    /*
     * Harware issue:
     * After powergate on, the interrupt line is undetermined,
     * so clear the pending here.
     */
#if defined(CONFIG_CPU_CORTEX_M)
    NVIC_ClearPendingIRQ((IRQn_Type)VGLITE_GPU_IRQN);
#endif

    /* Enable interrupt. */
    irq_enable(VGLITE_GPU_IRQN);

    vg_lite_hal_print("power on\n");
}

void vg_lite_hal_deinitialize(void)
{
    /* Disable interrupt. */
    irq_disable(VGLITE_GPU_IRQN);

    /* Remove clock. */
    vg_lite_set_gpu_clock_state(0);

    vg_lite_hal_print("power off\n");
}

void vg_lite_hal_print(char *format, ...)
{
#if gcdVG_ENABLE_DEBUG
    va_list args;

    va_start(args, format);
    vsnprintf(g_log_buffer, sizeof(g_log_buffer) - 1, format, args);
    va_end(args);

    g_log_buffer[sizeof(g_log_buffer) - 1] = 0;
    printk("[GPU] %s", g_log_buffer);
#endif
}

void vg_lite_hal_trace(char *format, ...)
{
#if gcdVG_ENABLE_DEBUG
    va_list args;

    va_start(args, format);
    vsnprintf(g_log_buffer, sizeof(g_log_buffer) - 1, format, args);
    va_end(args);

    g_log_buffer[sizeof(g_log_buffer) - 1] = 0;
    printk("[GPU] %s", g_log_buffer);
#endif
}

const char *vg_lite_hal_Status2Name(vg_lite_error_t status)
{
    switch (status) {
    case VG_LITE_SUCCESS:
        return "VG_LITE_SUCCESS";
    case VG_LITE_INVALID_ARGUMENT:
        return "VG_LITE_INVALID_ARGUMENT";
    case VG_LITE_OUT_OF_MEMORY:
        return "VG_LITE_OUT_OF_MEMORY";
    case VG_LITE_NO_CONTEXT:
        return "VG_LITE_NO_CONTEXT";
    case VG_LITE_TIMEOUT:
        return "VG_LITE_TIMEOUT";
    case VG_LITE_OUT_OF_RESOURCES:
        return "VG_LITE_OUT_OF_RESOURCES";
    case VG_LITE_GENERIC_IO:
        return "VG_LITE_GENERIC_IO";
    case VG_LITE_NOT_SUPPORT:
        return "VG_LITE_NOT_SUPPORT";
    case VG_LITE_ALREADY_EXISTS:
        return "VG_LITE_ALREADY_EXISTS";
    case VG_LITE_NOT_ALIGNED:
        return "VG_LITE_NOT_ALIGNED";
    case VG_LITE_FLEXA_TIME_OUT:
        return "VG_LITE_FLEXA_TIME_OUT";
    case VG_LITE_FLEXA_OUTOFSYNC:
        return "VG_LITE_FLEXA_OUTOFSYNC";
    case VG_LITE_SYSTEM_CALL_FAIL:
        return "VG_LITE_SYSTEM_CALL_FAIL";
    default:
        return "nil";
    }
}

vg_lite_error_t vg_lite_hal_allocate(uint32_t size, void **memory)
{
    struct vg_lite_dev_data *data = gp_dev_data;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    k_spinlock_key_t key;

    assert(data != NULL);

    if (size == 0 || NULL == memory) {
        ONERROR(VG_LITE_INVALID_ARGUMENT);
    }

    key = k_spin_lock(&data->heap_lock);
    *memory = sys_heap_alloc(&data->heap, size);
    k_spin_unlock(&data->heap_lock, key);

    if (NULL == memory) {
        ONERROR(VG_LITE_OUT_OF_MEMORY);
    }

on_error:
    return error;
}

vg_lite_error_t vg_lite_hal_free(void *memory)
{
    struct vg_lite_dev_data *data = gp_dev_data;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    k_spinlock_key_t key;

    assert(data != NULL);

    if (memory) {
        key = k_spin_lock(&data->heap_lock);
        sys_heap_free(&data->heap, memory);
        k_spin_unlock(&data->heap_lock, key);
    }

    return error;
}

vg_lite_error_t vg_lite_hal_allocate_contiguous(unsigned long size, vg_lite_vidmem_pool_t pool, void **logical, void **klogical, uint32_t *physical, void **node)
{
    struct vg_lite_dev_data *data = gp_dev_data;
    unsigned long aligned_size;
    k_spinlock_key_t key;
    assert(data != NULL);
    ARG_UNUSED(pool);
    ARG_UNUSED(node);

    /* Align the size to 64 bytes. */
    aligned_size = VG_LITE_ALIGN(size, VGLITE_MEM_ALIGNMENT);

    key = k_spin_lock(&data->heap_lock);
    *logical = sys_heap_aligned_alloc(&data->heap, VGLITE_MEM_ALIGNMENT, aligned_size);
    k_spin_unlock(&data->heap_lock, key);

    if (*logical == NULL) {
      LOG_ERR("gpu alloc %lu bytes failed!\n", size);
      return VG_LITE_OUT_OF_MEMORY;
    }

    *klogical = *logical;
    /*
    * i.MX RT series uses Cortex-M7/M4 with MPU (not MMU), so there is
    * no virtual memory address translation. CPU logical address equals
    * physical address which equals GPU address.
    */
    *physical = (uint32_t)(uintptr_t)(*logical);

    return VG_LITE_SUCCESS;
}

void vg_lite_hal_free_contiguous(void *memory_handle)
{
    struct vg_lite_dev_data *data = gp_dev_data;
    k_spinlock_key_t key;

    assert(data != NULL);

    key = k_spin_lock(&data->heap_lock);
    sys_heap_free(&data->heap, memory_handle);
    k_spin_unlock(&data->heap_lock, key);
}

void vg_lite_hal_free_os_heap(void)
{
    /* TODO: Remove unfree node. */
}

uint32_t vg_lite_hal_peek(uint32_t address)
{
    /* Read data from the GPU register. */
    return sys_read32(VGLITE_GPU_BASE + address);
}

void vg_lite_hal_poke(uint32_t address, uint32_t data)
{
    /* Write data to the GPU register. */
    sys_write32(data, VGLITE_GPU_BASE + address);
}

vg_lite_error_t vg_lite_hal_query_mem(vg_lite_kernel_mem_t *mem)
{
    mem->bytes = 0;
    return VG_LITE_NOT_SUPPORT;
}

vg_lite_error_t vg_lite_hal_map_memory(vg_lite_kernel_map_memory_t *node)
{
    node->logical = (void *)(uintptr_t)node->physical;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_hal_unmap_memory(vg_lite_kernel_unmap_memory_t *node)
{
    ARG_UNUSED(node);

    return VG_LITE_SUCCESS;
}

void * vg_lite_hal_map(uint32_t flags, uint32_t bytes, void *logical, uint32_t physical, int32_t dma_buf_fd, uint32_t *gpu)
{
    ARG_UNUSED(flags);
    ARG_UNUSED(bytes);
    ARG_UNUSED(logical);
    ARG_UNUSED(physical);
    ARG_UNUSED(dma_buf_fd);
    ARG_UNUSED(gpu);

    return (void *)0;
}

void vg_lite_hal_unmap(void *handle)
{
    ARG_UNUSED(handle);
}

vg_lite_error_t vg_lite_hal_operation_cache(void *handle, vg_lite_cache_op_t cache_op)
{
    ARG_UNUSED(handle);
    ARG_UNUSED(cache_op);

    return VG_LITE_NOT_SUPPORT;
}

vg_lite_error_t vg_lite_hal_memory_export(int32_t *fd)
{
    return VG_LITE_SUCCESS;
}

void vg_lite_set_gpu_execute_state(vg_lite_gpu_execute_state_t state)
{
    struct vg_lite_dev_data *data = gp_dev_data;
    assert(data != NULL);

    if (state != data->gpu_execute_state) {
        data->gpu_execute_state = state;
        vg_lite_set_gpu_clock_state(state == VG_LITE_GPU_RUN);
    }
}

void vg_lite_set_gpu_clock_state(int enabled)
{
    struct vg_lite_dev_data *data = gp_dev_data;
    assert(data != NULL);

    if (enabled) {
        if (++data->gpu_clken_cnt == 1) {
            vg_lite_hal_trace("clk enabled\n");
            CLOCK_EnableClock(CLOCK_ID_GPU);
            /**
             * FIXME: should add any delay to wait clock stable ?
             *
             * k_busy_wait(10);
             */
        }
    } else {
        if (--data->gpu_clken_cnt == 0) {
            vg_lite_hal_trace("clk disabled\n");
            CLOCK_DisableClock(CLOCK_ID_GPU);
        }
    }

    assert(data->gpu_clken_cnt >= 0);
}


int32_t vg_lite_hal_wait_interrupt(uint32_t timeout, uint32_t mask, uint32_t * value)
{
    struct vg_lite_dev_data *data = gp_dev_data;
    assert(data != NULL);

    int result = k_sem_take(&data->wait_sem,
            (timeout == VG_LITE_INFINITE) ? K_FOREVER : K_MSEC(timeout));

    if (!result) {
      k_spinlock_key_t key = k_spin_lock(&data->heap_lock);
      if (value != NULL) {
        *value = data->int_flags & mask;
      }

      data->int_flags = 0U;
      k_spin_unlock(&data->heap_lock, key);

      return 1;
    }

    return 0;
}

static void vg_lite_dev_isr(const void *arg)
{
    const struct device *dev = arg;
    struct vg_lite_dev_data *data = dev->data;

    /* Read interrupt status. */
    uint32_t flags = sys_read32(VGLITE_GPU_BASE + VG_LITE_INTR_STATUS);

    if (flags) {
        k_spinlock_key_t key = k_spin_lock(&data->heap_lock);
        /* Combine with current interrupt flags. */
        data->int_flags |= flags;
        k_spin_unlock(&data->heap_lock, key);
        /* Wake up any waiters. */
        k_sem_give(&data->wait_sem);
#if gcdVG_RECORD_HARDWARE_RUNNING_TIME
        record_running_time();
#endif

    }

#if 0
    if(flags = VGLITE_EVENT_FRAME_END){
    /* A callback function can be added here to inform that gpu is idle.*/
        (*callback)();
    }

#endif
}

DEVICE_DECLARE(gpu);

static int vg_lite_dev_init(const struct device *dev)
{
    vg_lite_error_t err;
    struct vg_lite_dev_data *data = dev->data;

    data->gpu_execute_state = VG_LITE_GPU_STOP;

    sys_heap_init(&data->heap, vg_lite_heap_mem, sizeof(vg_lite_heap_mem));
    k_sem_init(&data->wait_sem, 0, 1);

    data->register_base = VGLITE_GPU_BASE;
    k_sem_init(&data->wait_sem, 0, 1);
    data->int_flags = 0U;
    data->gpu_clken_cnt = 0;

    IRQ_CONNECT(VGLITE_GPU_IRQN, 0, vg_lite_dev_isr, DEVICE_GET(gpu), 0);

    gp_dev_data = data;

    err = vg_lite_init(VGLITE_TESS_W, VGLITE_TESS_H);
    if (err != VG_LITE_SUCCESS) {
      return -ENODEV;
    }

    err = vg_lite_set_command_buffer_size(VGLITE_COMMAND_BUF_SIZE);
    if (err != VG_LITE_SUCCESS) {
      return -ENODEV;
    }

    return 0;
}

#ifdef CONFIG_PM_DEVICE
static int vg_lite_dev_suspend(const struct device *dev)
{
    struct vg_lite_dev_data *data = dev->data;

    if (data->gpu_execute_state == VG_LITE_GPU_RUN) {
        vg_lite_hal_print("suspend fail!\n");
        return -EBUSY;
    }

    vg_lite_set_gpu_clock_state(1);

    /* shutdown gpu */
    vg_lite_kernel(VG_LITE_CLOSE, NULL);

    /* shutdown power and clock  */
    vg_lite_hal_deinitialize();

    vg_lite_hal_trace("suspend success!\n");

    return 0;
}

static int vg_lite_dev_resume(const struct device *dev)
{
#if !gcdVG_ENABLE_DELAY_RESUME
    vg_lite_hal_initialize();
    /* open gpu interrupt and recovery gpu register */
    vg_lite_kernel(VG_LITE_RESET, NULL);
    vg_lite_set_gpu_clock_state(0);
    vg_lite_hal_trace("resume success!\n");
#else
    vg_lite_set_gpu_clock_state(1);
    vg_lite_kernel_delay_resume_t resume;
    resume.set_delay_resume = 1;
    vg_lite_kernel(VG_LITE_SET_DELAY_RESUME, &resume);
    vg_lite_set_gpu_clock_state(0);
    vg_lite_hal_trace("configure delay resume success!\n");
#endif

    return 0;
}

static int vg_lite_dev_pm_control(const struct device *dev, enum pm_device_action action)
{
    int ret = 0;

    switch (action) {
    case PM_DEVICE_ACTION_SUSPEND:
        ret = vg_lite_dev_suspend(dev);
        break;
    case PM_DEVICE_ACTION_RESUME:
        ret = vg_lite_dev_resume(dev);
        break;
    default:
        break;
    }

    return ret;
}
#endif /* CONFIG_PM_DEVICE */

#if CONFIG_GPU_DEV
static struct vg_lite_dev_data vg_lite_dev_data;

#ifdef CONFIG_PM_DEVICE
PM_DEVICE_DEFINE(gpu, vg_lite_dev_pm_control);

DEVICE_DEFINE(gpu, CONFIG_GPU_DEV_NAME, vg_lite_dev_init,
        PM_DEVICE_GET(gpu), &vg_lite_dev_data, NULL, POST_KERNEL,
        CONFIG_KERNEL_INIT_PRIORITY_DEVICE, NULL);
#else
DEVICE_DEFINE(gpu, CONFIG_GPU_DEV_NAME, vg_lite_dev_init,
        NULL, &vg_lite_dev_data, NULL, POST_KERNEL,
        CONFIG_KERNEL_INIT_PRIORITY_DEVICE, NULL);
#endif /* CONFIG_PM_DEVICE */

#endif /* CONFIG_GPU_DEV */
