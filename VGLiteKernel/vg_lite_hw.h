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


#ifndef _vg_lite_hw_h
#define _vg_lite_hw_h

#define VG_LITE_HW_CLOCK_CONTROL    0x000
#define VG_LITE_HW_IDLE             0x004
#define VG_LITE_INTR_STATUS         0x010
#define VG_LITE_INTR_ENABLE         0x014
#define VG_LITE_HW_CHIP_ID          0x020
#define VG_LITE_HW_CMDBUF_ADDRESS   0x500
#define VG_LITE_HW_CMDBUF_SIZE      0x504

#define VG_LITE_EXT_WORK_CONTROL    0x520
#define VG_LITE_EXT_VIDEO_SIZE      0x524
#define VG_LITE_EXT_CLEAR_VALUE     0x528

#define VG_LITE_EXT_VIDEO_CONTROL   0x51C

typedef struct clock_control {
    uint32_t reserved0 : 1;
    uint32_t clock_gate : 1;
    uint32_t scale : 7;
    uint32_t scale_load : 1;
    uint32_t reserved10 : 2;
    uint32_t soft_reset : 1;
    uint32_t reserved13 : 6;
    uint32_t isolate : 1;
} clock_control_t;

typedef union vg_lite_hw_clock_control {
    clock_control_t control;
    uint32_t        data;
} vg_lite_hw_clock_control_t;

#define VG_LITE_HW_IDLE_STATE       0x0B05

#endif /* defined(_vg_lite_hw_h) */
