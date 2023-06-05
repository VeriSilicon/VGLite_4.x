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


#ifndef _VG_LITE_PLATFORM_H
#define _VG_LITE_PLATFORM_H

#include "stdint.h"
#include "stdlib.h"

#define _BAREMETAL 0

/*!
@brief Initialize the hardware mem setting.
*/
void vg_lite_init_mem(uint32_t register_mem_base,
                      uint32_t gpu_mem_base,
                      volatile void * contiguous_mem_base,
                      uint32_t contiguous_mem_size);

/*!
@brief The hardware IRQ handler.
*/
void vg_lite_IRQHandler(void);

#endif
