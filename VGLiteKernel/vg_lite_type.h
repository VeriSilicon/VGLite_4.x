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


#ifndef _vg_lite_type_h_
#define _vg_lite_type_h_

#include <asm/bitsperlong.h>
#include "vg_lite_kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VG_FALSE                   0
#define VG_TRUE                    1

#define VG_SUCCESS                 0
#define VG_FAIL                   -1

typedef int                 vg_lite_bool_t;
typedef unsigned char       vg_lite_uint8_t;
typedef char                vg_lite_int8_t;
typedef short               vg_lite_int16_t;
typedef unsigned short      vg_lite_uint16_t;
typedef int                 vg_lite_int32_t;
typedef unsigned int        vg_lite_uint32_t;
typedef unsigned long long  vg_lite_uint64_t;
typedef float               vg_lite_float_t;
typedef double              vg_lite_double_t;
typedef char                vg_lite_char;
typedef char*               vg_lite_string;
typedef void*               vg_lite_pointer;
typedef void                vg_lite_void;
typedef unsigned int        vg_lite_color_t;
typedef unsigned long       vg_lite_flag_t;
typedef unsigned long       vg_lite_long_t;

#if __KERNEL__
#if BITS_PER_LONG == 64
    typedef unsigned long long vg_lite_uintptr_t;
# else
    typedef unsigned int       vg_lite_uintptr_t;
# endif
#endif

#ifdef __cplusplus
}
#endif

#endif

