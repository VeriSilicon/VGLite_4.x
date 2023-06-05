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


#ifndef _vg_lite_debug_h_
#define _vg_lite_debug_h_

#include "vg_lite_kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VGL_DEBUG
#define VGL_TRACE

#ifdef VGL_DEBUG
# define vg_lite_kernel_print(fmt, arg...) printk("[VGL DEBUG] "fmt,##arg)
#else
# define vg_lite_kernel_print(fmt, arg...) do{}while(VG_FALSE)
#endif

#ifdef VGL_TRACE
# define vg_lite_kernel_trace() printk("[VGL TRACE] [%s, %d, %s]", __FUNCTION__, __LINE__, __FILE__)
#else
# define vg_lite_kernel_trace() do{}while(VG_FALSE)
#endif

#define vg_lite_kernel_error(fmt, arg...) printk("[VGL ERROR] "fmt,##arg)
#define vg_lite_kernel_hintmsg(fmt, arg...) printk("[VGL HINTMSG] "fmt,##arg)

#define VG_IS_SUCCESS(error) (error == VG_LITE_SUCCESS)
#define VG_IS_ERROR(error)   (error != VG_LITE_SUCCESS)

#define ONERROR(func) \
        do \
        { \
            error = func; \
            if (VG_IS_ERROR(error)) \
            { \
                vg_lite_kernel_error("%d %s(%d)\n", error, __FUNCTION__, __LINE__); \
                goto on_error; \
            } \
        } \
        while (VG_FALSE)

#define ASSERT(arg) \
           do \
           { \
               if (!(arg)) \
               { \
                   error = VG_LITE_INVALID_ARGUMENT;\
                   goto on_error; \
               } \
           } \
           while (VG_FALSE)

#ifdef __cplusplus
}
#endif

#endif
