/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2014 - 2026 Vivante Corporation
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

#ifndef _vg_lite_context_h_
#define _vg_lite_context_h_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <assert.h>
#include "vg_lite.h"
#include "vg_lite_kernel.h"
#include "vg_lite_options.h"

#define DUMP_CAPTURE                            0
#define DUMP_API                                0
#define DUMP_LAST_CAPTURE                       0

#if DUMP_LAST_CAPTURE
#define DUMP_LAST_FRAME_CAPTURE(api_id) record_api(api_id)
#else
#define DUMP_LAST_FRAME_CAPTURE(api_id) ((vg_lite_void)0)
#endif

#if DUMP_API
#include "dumpAPI.h"
#define DUMP_API_CALL(func, ...) do { if (dump_api_flag) { FUNC_DUMP(func)(__VA_ARGS__); } } while(0)
#else
#define DUMP_API_CALL(func, ...) ((vg_lite_void)0)
#endif

#define VGLITE_LOG    printf

#if gcFEATURE_VG_TRACE_API
#define VG_LITE_TRACE_API(...) VGLITE_LOG(__VA_ARGS__)
#else
#define VG_LITE_TRACE_API(...) ((vg_lite_void)0)
#endif

/*** Global Context Access ***/
#define GET_CONTEXT()               &s_context

/*** Default command buffer size is 32KB. Double command buffer is used.
     App can call vg_lite_set_command_buffer_size(size) before vg_lite_init()
     to overwrite the default command buffer size.
***/
#define VG_LITE_COMMAND_BUFFER_SIZE (32 << 10)

#define CMDBUF_BUFFER(context)      (context).command_buffer[(context).command_buffer_current]
#define CMDBUF_INDEX(context)       (context).command_buffer_current
#define CMDBUF_SIZE(context)        (context).command_buffer_size
#define CMDBUF_OFFSET(context)      (context).command_offset[(context).command_buffer_current]
#define CMDBUF_SWAP(context)        (context).command_buffer_current = \
                                        ((context).command_buffer_current + 1) % CMDBUF_COUNT

/*** Command macros ***/
#define VG_LITE_END(interrupt)      (0x00000000 | interrupt)
#define VG_LITE_SEMAPHORE(id)       (0x10000000 | id)
#define VG_LITE_STALL(id)           (0x20000000 | id)
#define VG_LITE_STATE(address)      (0x30010000 | address)
#define VG_LITE_STATES(count, address)  (0x30000000 | ((count) << 16) | address)
#define VG_LITE_DATA(count)         (0x40000000 | count)
#define VG_LITE_CALL(count)         (0x60000000 | count)
#define VG_LITE_RETURN()            (0x70000000)
#define VG_LITE_NOP()               (0x80000000)

#define FC_BURST_BYTES              64
#define FC_BIT_TO_BYTES             64

#define STATES_COUNT                208
#define MIN_TS_SIZE                 8 << 10

#define VG_LITE_RETURN_ERROR(func) \
    do { \
        if ((error = func) != VG_LITE_SUCCESS) \
            return error; \
    } while (VGL_FALSE)

#define VG_LITE_BREAK_ERROR(func) \
    do { \
        if ((error = func) != VG_LITE_SUCCESS) \
            break; \
    } while (VGL_FALSE)

#define VG_LITE_ERROR_HANDLER(func) \
    do { \
        if ((error = func) != VG_LITE_SUCCESS) \
            goto ErrorHandler; \
    } while (VGL_FALSE)

#define VG_LITE_CHECK_NULL_POINTER(a) \
    do { \
        if ((a) == NULL) \
            return VG_LITE_INVALID_ARGUMENT; \
    } while (VGL_FALSE)

#define VG_LITE_CHECK_NULL_POINTER2(a, b) \
    do { \
        if ((a) == NULL || (b) == NULL) \
            return VG_LITE_INVALID_ARGUMENT; \
    } while (VGL_FALSE)

#define VG_LITE_CHECK_NULL_POINTER3(a, b, c) \
    do { \
        if ((a) == NULL || (b) == NULL || (c) == NULL) \
            return VG_LITE_INVALID_ARGUMENT; \
    } while (VGL_FALSE)

#define VG_LITE_CHECK_NULL_POINTER4(a, b, c, d) \
    do { \
        if ((a) == NULL || (b) == NULL || (c) == NULL || (d) == NULL) \
            return VG_LITE_INVALID_ARGUMENT; \
    } while (VGL_FALSE)

/*** Shortcuts. ***/
#define A(color)                    (color) >> 24
#define R(color)                    ((color) & 0x00ff0000) >> 16
#define G(color)                    ((color) & 0x0000ff00) >> 8
#define B(color)                    ((color) & 0xff)
#define ARGB(a, r, g, b)            ((a) << 24) | ((r) << 16) | ((g) << 8 ) | (b)
#define ARGB4(a, r, g, b)           (((a) & 0xf0) << 8) | (((r) & 0xf0) << 4) | (((g) & 0xf0)) | ((b) >> 4)

#define MIN(a, b)                   (a) > (b) ? (b) : (a)
#define MAX(a, b)                   (a) > (b) ? (a) : (b)

#define LERP(v1, v2, w)             ((v1) * (w) + (v2) * (1.0f - (w)))
#define CLAMP(x, min, max)          (((x) < (min)) ? (min) : ((x) > (max)) ? (max) : (x))

#define COLOR_FROM_RAMP(ColorRamp)  (((vg_lite_float_t *) ColorRamp) + 1)

#define MATRIX_ROWS                 3
#define GET_MATRIX_VALUES(Pointer)  ((vg_lite_float_t *) (Pointer))
#define MAT(Matrix, Row, Column)    (GET_MATRIX_VALUES(Matrix)[Row * MATRIX_ROWS + Column])
#define PI                          3.141592653589793238462643383279502f

/********************************* Combined Features Define ******************************/
#define gcFEATURE_COMBO_VG_SUPPORT_DEC_COMPRESS \
    (gcFEATURE_VG_DEC_COMPRESS || gcFEATURE_VG_DEC_COMPRESS_2_0 || gcFEATURE_VG_DEC_COMPRESS_2_1 \
     || gcFEATURE_VG_DEC_COMPRESS_2_2 || gcFEATURE_VG_DEC_COMPRESS_3_0)

#define gcFEATURE_COMBO_VG_SUPPORT_RADIAL_GRADIENT \
    (gcFEATURE_VG_RADIAL_GRADIENT && gcFEATURE_VG_IM_INPUT)

/* Legacy LVGL blend path: alpha had to be forced to 0xFF. */
#define gcFEATURE_COMBO_VG_WAR_FOR_LEGACY_LVGL_BLEND \
    (!gcFEATURE_VG_NEW_FACTOR && gcFEATURE_VG_GLOBAL_ALPHA)

#define gcFEATURE_COMBO_VG_NOT_SUPPORT_PLANAR_YUV_NV24 \
    (gcFEATURE_VG_YUV_INPUT && !gcFEATURE_VG_NV24_INPUT)

#define gcFEATURE_COMBO_VG_MESH_BLT_WITH_SW_LVGL_BLEND \
    (!gcFEATURE_VG_LVGL_SUPPORT && (gcFEATURE_VG_SIMPLE_BLT || gcFEATURE_VG_EXTERNAL_DMA_MESH))

#define gcFEATURE_COMBO_VG_MESH_RENDERING_WITH_BLIT \
    (gcFEATURE_VG_MESH_FOR_FRAME && (gcFEATURE_VG_SIMPLE_BLT || gcFEATURE_VG_EXTERNAL_DMA_MESH))

#define gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW \
    (!gcFEATURE_VG_24BIT_PLANAR && gcFEATURE_VG_24BIT_PLANAR_SW)

#define gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW \
    (!gcFEATURE_VG_MATH_PRECISION_FIX_DISABLE && ((CHIPID == 0x555) || gcFEATURE_VG_BOUNDARY_FILTER_BYPASS))

#define gcFEATURE_COMBO_VG_SPLIT_PATH_SUPPORT_BY_SW \
    (!gcFEATURE_VG_SPLIT_PATH_DISABLE || !gcFEATURE_VG_PARALLEL_PATHS_DISABLE \
     || !gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE || !gcFEATURE_VG_512_HALF_SPLIT_DISABLE)

#if DUMP_LAST_CAPTURE
typedef enum vg_lite_api_id
{
    VG_LITE_INIT_API = 0,
    VG_LITE_CLOSE_API,
    VG_LITE_RESET_API,
    VG_LITE_GET_INFO_API,
    VG_LITE_GET_PRODUCT_INFO_API,
    VG_LITE_QUERY_FEATURE_API,
    VG_LITE_FINISH_API,
    VG_LITE_FLUSH_API,
    VG_LITE_GET_REGISTER_API,
    VG_LITE_GET_TRANSFORM_MATRIX_API,
    VG_LITE_ALLOCATE_API,
    VG_LITE_FREE_API,
    VG_LITE_UPLOAD_BUFFER_API,
    VG_LITE_MAP_API,
    VG_LITE_UNMAP_API,
    VG_LITE_FLUSH_MAPPED_BUFFER_API,
    VG_LITE_CLEAR_API,
    VG_LITE_BLIT_API,
    VG_LITE_BLIT_RECT_API,
    VG_LITE_BLIT2_API,
    VG_LITE_COPY_IMAGE_API,
    VG_LITE_DRAW_API,
    VG_LITE_SET_STROKE_API,
    VG_LITE_UPDATE_STROKE_API,
    VG_LITE_SET_PATH_TYPE_API,
    VG_LITE_CLEAR_PATH_API,
    VG_LITE_UPLOAD_PATH_API,
    VG_LITE_INIT_PATH_API,
    VG_LITE_INIT_ARC_PATH_API,
    VG_LITE_GET_PATH_LENGTH_API,
    VG_LITE_APPEND_PATH_API,
    VG_LITE_SET_CLUT_API,
    VG_LITE_DRAW_PATTERN_API,
    VG_LITE_INIT_GRAD_API,
    VG_LITE_CLEAR_GRAD_API,
    VG_LITE_UPDATE_GRAD_API,
    VG_LITE_GET_GRAD_MATRIX_API,
    VG_LITE_SET_GRAD_API,
    VG_LITE_DRAW_GRAD_API,
    VG_LITE_CLEAR_LINEAR_GRAD_API,
    VG_LITE_UPDATE_LINEAR_GRAD_API,
    VG_LITE_GET_LINEAR_GRAD_MATRIX_API,
    VG_LITE_DRAW_LINEAR_GRAD_API,
    VG_LITE_CLEAR_RADIAL_GRAD_API,
    VG_LITE_UPDATE_RADIAL_GRAD_API,
    VG_LITE_GET_RADIAL_GRAD_MATRIX_API,
    VG_LITE_SET_RADIAL_GRAD_API,
    VG_LITE_DRAW_RADIAL_GRAD_API,
    VG_LITE_IDENTITY_API,
    VG_LITE_TRANSLATE_API,
    VG_LITE_SCALE_API,
    VG_LITE_ROTATE_API,
    VG_LITE_SET_SCISSOR_API,
    VG_LITE_SCISSOR_RECTS_API,
    VG_LITE_ENABLE_SCISSOR_API,
    VG_LITE_DISABLE_SCISSOR_API,
    VG_LITE_GET_MEM_SIZE_API,
    VG_LITE_SOURCE_GLOBAL_ALPHA_API,
    VG_LITE_DEST_GLOBAL_ALPHA_API,
    VG_LITE_SET_COLOR_KEY_API,
    VG_LITE_ENABLE_DITHER_API,
    VG_LITE_DISABLE_DITHER_API,
    VG_LITE_SET_TESS_BUFFER_API,
    VG_LITE_SET_COMMAND_BUFFER_SIZE_API,
    VG_LITE_SET_COMMAND_BUFFER_API,
    VG_LITE_SET_PIXEL_MATRIX_API,
    VG_LITE_GAUSSIAN_FILTER_API,
    VG_LITE_ENABLE_MASKLAYER_API,
    VG_LITE_DISABLE_MASKLAYER_API,
    VG_LITE_SET_MASKLAYER_API,
    VG_LITE_DESTROY_MASKLAYER_API,
    VG_LITE_CREATE_MASKLAYER_API,
    VG_LITE_FILL_MASKLAYER_API,
    VG_LITE_BLEND_MASKLAYER_API,
    VG_LITE_RENDER_MASKLAYER_API,
    VG_LITE_SET_MIRROR_API,
    VG_LITE_SET_GAMMA_API,
    VG_LITE_ENABLE_COLOR_TRANSFORM_API,
    VG_LITE_DISABLE_COLOR_TRANSFORM_API,
    VG_LITE_SET_COLOR_TRANSFORM_API,
    VG_LITE_FLEXA_SET_STREAM_API,
    VG_LITE_FLEXA_BG_BUFFER_API,
    VG_LITE_FLEXA_ENABLE_API,
    VG_LITE_FLEXA_DISABLE_API,
    VG_LITE_FLEXA_STOP_FRAME_API,
    VG_LITE_DUMP_COMMAND_BUFFER_API,
    VG_LITE_DUMP_PNG_API,
    VG_LITE_GET_PARAMETER_API,
    VG_LITE_SET_MEMORY_POOL_API,
    VG_LITE_FRAME_DELIMITER_API,
    VG_LITE_CACHE_COMMAND_API,
    VG_LITE_SPLIT_PATH_API,
    VG_LITE_SET_DUMP_API
} vg_lite_api_id_t;

#define LAST_CALL_API_NUMS 50
extern vg_lite_uint8_t last_api_call[LAST_CALL_API_NUMS];
extern vg_lite_uint8_t api_call_nums;

extern vg_lite_void record_api(vg_lite_api_id_t api_id);
#endif

#if (CHIPID == 0X555) || (CHIPID == 0X355)
#define VG_PRE_UPLOAD_PATH_SUPPORT 0
#else
#define VG_PRE_UPLOAD_PATH_SUPPORT 1
#endif

/* Driver implementation internal structures.
*/
typedef struct vg_lite_states {
    vg_lite_uint32_t                    state;
    vg_lite_uint8_t                     init;
} vg_lite_states_t;

typedef struct vg_lite_hardware {
    vg_lite_states_t            hw_states[STATES_COUNT];
} vg_lite_hardware_t;

/* Tessellation buffer information. */
typedef struct vg_lite_tess_buffer
{
    vg_lite_uint32_t            physical_addr;         /*! Physical address for tessellation buffer. */
    vg_lite_uint8_t            *logical_addr;          /*! Logical address for tessellation buffer. */
    vg_lite_uint32_t            tessbuf_size;          /*! Buffer size for tessellation buffer */
    vg_lite_uint32_t            countbuf_size;         /*! Buffer size for VG count buffer */
    vg_lite_uint32_t            tess_w_h;              /*! Combination of buffer width and height. */
    vg_lite_uint32_t            tess_x_y;              /*! Combination of buffer origin x and y. */
    /* gc355 Specific fields below */
    vg_lite_uint32_t            L1_phyaddr;            /*! L1 physical address. */
    vg_lite_uint32_t            L2_phyaddr;            /*! L2 physical address. */
    vg_lite_uint8_t            *L1_logical;            /*! L1 Logical address. */
    vg_lite_uint8_t            *L2_logical;            /*! L2 Logical address. */
    vg_lite_uint32_t            L1_size;               /*! L1 size for tessellation buffer */
    vg_lite_uint32_t            L2_size;               /*! L2 size for tessellation buffer */
    vg_lite_uint32_t            tess_stride;           /*! Stride for tessellation buffer */
} vg_lite_tess_buffer_t;

typedef struct vg_lite_cache_cmd_info
{
    vg_lite_uint32_t                    fb_command_offset_start;
    vg_lite_uint32_t                    fb_command_offset_end;
    vg_lite_uint32_t                    special_register_offset_start;
    vg_lite_uint32_t                    special_register_address;
    struct vg_lite_cache_cmd_info      *next;
} vg_lite_cache_cmd_info;

typedef struct vg_lite_context {
    vg_lite_kernel_context_t            context;
    vg_lite_hardware_t                  hw;
    vg_lite_capabilities_t              capabilities;
    vg_lite_uint8_t                    *command_buffer[CMDBUF_COUNT];
    vg_lite_uint32_t                    command_buffer_size;
    vg_lite_uint32_t                    command_offset[CMDBUF_COUNT];
    vg_lite_uint32_t                    command_buffer_current;
    vg_lite_memory_pool_t               command_buffer_pool;

    vg_lite_tess_buffer_t               tessbuf;
    vg_lite_memory_pool_t               tess_buffer_pool;

    vg_lite_buffer_t                   *rtbuffer;                   /* DDRLess: this is used as composing buffer. */
    vg_lite_memory_pool_t               render_buffer_pool;

    vg_lite_float_t                     path_lastX;
    vg_lite_float_t                     path_lastY;
    vg_lite_uint32_t                    scissor_set;
    vg_lite_uint32_t                    scissor_enable;
    vg_lite_uint32_t                    scissor_dirty;
    vg_lite_int32_t                     scissor[4];                 /* Scissor area: x, y, right, bottom. */
    vg_lite_buffer_t                   *scissor_layer;
    vg_lite_int32_t                     scissor_layer_range[4];
    vg_lite_uint32_t                    src_alpha_mode;
    vg_lite_uint32_t                    src_alpha_value;
    vg_lite_uint32_t                    dst_alpha_mode;
    vg_lite_uint32_t                    dst_alpha_value;
    vg_lite_blend_t                     blend_mode;

    vg_lite_uint32_t                    sbi_mode;
    vg_lite_uint32_t                    sync_mode;
    vg_lite_uint32_t                    stream_id;
    vg_lite_uint32_t                    segment_address;
    vg_lite_uint32_t                    segment_count;
    vg_lite_uint32_t                    segment_offset;
    vg_lite_uint32_t                    segment_size;
    vg_lite_uint32_t                    flexa_mesh_size;
    vg_lite_uint32_t                    stop_flag;
    vg_lite_uint8_t                     flexa_dirty;
    vg_lite_uint32_t                    start_flag;
    vg_lite_uint32_t                    reset_flag;
    vg_lite_uint32_t                    consumer0_start_timeout_mode;
    vg_lite_uint32_t                    consumer1_start_timeout_mode;
    vg_lite_uint32_t                    producer0_start_timeout_mode;
    vg_lite_uint32_t                    producer1_start_timeout_mode;
    vg_lite_uint32_t                    consumer0_request_timeout_mode;
    vg_lite_uint32_t                    consumer1_request_timeout_mode;
    vg_lite_uint32_t                    producer0_request_timeout_mode;
    vg_lite_uint32_t                    producer1_request_timeout_mode;
    vg_lite_uint32_t                    consumer0_consumer_id;
    vg_lite_uint32_t                    consumer1_consumer_id;
    vg_lite_uint32_t                    producer0_consumer_id;
    vg_lite_uint32_t                    producer1_consumer_id;
    vg_lite_uint8_t                     custom_cmdbuf;
    vg_lite_uint8_t                     custom_tessbuf;
    vg_lite_uint32_t                    enable_mask;
    vg_lite_buffer_t                   *mask_layer;
    vg_lite_uint32_t                    matrix_enable;
    vg_lite_uint32_t                    tess_width;
    vg_lite_uint32_t                    tess_height;
    vg_lite_uint32_t                    target_width;
    vg_lite_uint32_t                    target_height;
    vg_lite_uint8_t                     enable_scissor;
    vg_lite_uint32_t                    mirror_orient;
    vg_lite_uint32_t                    mirror_dirty;
    vg_lite_uint32_t                    gamma_value;
    vg_lite_uint32_t                    gamma_dirty;
    vg_lite_uint32_t                    gamma_src;
    vg_lite_uint32_t                    gamma_dst;
    vg_lite_uint32_t                    gamma_stencil;
    vg_lite_uint32_t                    color_transform;
    vg_lite_uint32_t                    path_counter;
    vg_lite_filter_t                    filter;
    vg_lite_pointer                     last_command_buffer_logical;
    size_t                              Physical;
    vg_lite_uint32_t                    last_command_size;
    vg_lite_frame_flag_t                frame_flag;
    vg_lite_mesh_mode_t                 mesh_mode;
    vg_lite_uint32_t                    mesh_height;
    vg_lite_uint8_t                     mesh_count;
    vg_lite_uint8_t                     mesh_dirty;
    vg_lite_uint8_t                     mesh_mode_dirty;
    vg_lite_uint32_t                    backup_fb_command_flag;
    vg_lite_uint8_t                    *fb_command_buffer;
    vg_lite_uint32_t                    fb_command_buffer_physical;
    vg_lite_uint32_t                    fb_command_buffer_size;
    vg_lite_uint32_t                    fb_command_offset;
    vg_lite_uint32_t                    fb_command_buffer_index;    
    vg_lite_uint32_t                    fb_finish_flag;
    vg_lite_cache_cmd_info             *fb_command_buffer_start;
    vg_lite_cache_cmd_info             *fb_command_buffer_end;

    vg_lite_uint32_t                    split_path;
    
    vg_lite_uint8_t                     state;
    vg_lite_porter_duff_config_t        porter_duff_config;
    vg_lite_bool_t                      porter_duff_enable;
} vg_lite_context_t;

typedef struct vg_lite_ftable {
    vg_lite_uint32_t                    ftable[gcFEATURE_COUNT];
} vg_lite_ftable_t;

typedef struct vg_factor_config {
    vg_lite_uint32_t factor_src_alpha;
    vg_lite_uint32_t factor_src_color;
    vg_lite_uint32_t factor_dst_alpha;
    vg_lite_uint32_t factor_dst_color;
    vg_lite_uint32_t final_equation_opcode;
    vg_lite_uint32_t dstchannelmode;
    vg_lite_uint32_t srcchannelmode;
}vg_factor_config_t;

extern vg_lite_context_t        s_context;
extern vg_lite_ftable_t         s_ftable;
extern vg_lite_char             dump_api_flag;

extern vg_lite_error_t set_render_target(vg_lite_buffer_t* target);
extern vg_lite_error_t push_state(vg_lite_context_t* context, vg_lite_uint32_t address, vg_lite_uint32_t data);
extern vg_lite_error_t push_state_ptr(vg_lite_context_t* context, vg_lite_uint32_t address, vg_lite_pointer data_ptr);
extern vg_lite_error_t push_call(vg_lite_context_t* context, vg_lite_uint32_t address, vg_lite_uint32_t bytes);
extern vg_lite_error_t push_data(vg_lite_context_t* context, vg_lite_uint32_t size, vg_lite_pointer data);
extern vg_lite_error_t push_clut(vg_lite_context_t* context, vg_lite_uint32_t address, vg_lite_uint32_t count, vg_lite_uint32_t* data);
extern vg_lite_error_t push_stall(vg_lite_context_t* context, vg_lite_uint32_t module);
extern vg_lite_pointer vg_lite_os_malloc(size_t size);
extern vg_lite_void  vg_lite_os_free(vg_lite_pointer memory);
extern vg_lite_void set_gamma_dest_only(vg_lite_buffer_t *target, vg_lite_int32_t stencil);
extern vg_lite_void save_st_gamma_src_dest(vg_lite_buffer_t* source, vg_lite_buffer_t* target);
extern vg_lite_void get_st_gamma_src_dest(vg_lite_buffer_t* source, vg_lite_buffer_t* target);
extern vg_lite_void setup_lvgl_image(vg_lite_buffer_t* dst, vg_lite_buffer_t* src, vg_lite_buffer_t* temp, vg_lite_blend_t operation);
extern vg_lite_void calculate_step_value(vg_lite_filter_t filter, vg_lite_matrix_t* inverse_matrix, vg_lite_int32_t width, vg_lite_int32_t height,
                                         vg_lite_float_t x_step[3], vg_lite_float_t y_step[3], vg_lite_float_t c_step[3]);
extern vg_lite_float_t _calc_decnano_compress_ratio(vg_lite_buffer_format_t format, vg_lite_compress_mode_t compress_mode);
extern vg_lite_buffer_format_t convert_24bit_format(vg_lite_buffer_format_t format);
extern vg_lite_error_t vg_lite_convert_24bitplanar_to_24bit(vg_lite_buffer_t* source, vg_lite_buffer_t* target);
vg_lite_uint8_t is_packed_yuy_format(vg_lite_buffer_format_t format);
vg_lite_uint8_t is_lvgl_blend_mode(vg_lite_blend_t blend);

#if defined(__ZEPHYR__)
extern vg_lite_void * vg_lite_os_fopen(const vg_lite_char *__restrict path, const vg_lite_char *__restrict mode);
extern vg_lite_int32_t vg_lite_os_fclose(vg_lite_void * fp);
extern size_t vg_lite_os_fread(vg_lite_void *__restrict ptr, size_t size, size_t nmemb, vg_lite_void *__restrict fp);
extern size_t vg_lite_os_fwrite(const vg_lite_void *__restrict ptr, size_t size, size_t nmemb, vg_lite_void * fp);
extern vg_lite_int32_t vg_lite_os_fseek(vg_lite_void * fp, long offset, vg_lite_int32_t whence);
extern vg_lite_int32_t vg_lite_os_fflush(vg_lite_void *fp);
extern vg_lite_int32_t vg_lite_os_fprintf(vg_lite_void *__restrict fp, const vg_lite_char *__restrict format, ...);
extern vg_lite_int32_t vg_lite_os_getpid(vg_lite_void);
#elif defined(_WINDLL) || defined(__linux__)
extern vg_lite_int32_t   vg_lite_os_fseek(FILE* Stream, long Offset, vg_lite_int32_t Origin);
extern FILE* vg_lite_os_fopen(vg_lite_char const* FileName, vg_lite_char const* Mode);
extern long  vg_lite_os_ftell(FILE* Stream);
extern size_t vg_lite_os_fread(vg_lite_pointer Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream);
extern size_t vg_lite_os_fwrite(vg_lite_void const* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream);
extern vg_lite_int32_t    vg_lite_os_close(FILE* Stream);
extern vg_lite_int32_t    vg_lite_os_fflush(FILE* fp);
#endif
extern void vg_flush_previous_rt(void);

/**************************** Dump command, image ********************************************/

#define DUMP_COMMAND                            0
#define DUMP_IMAGE                              0

#if DUMP_COMMAND || DUMP_IMAGE
#ifdef __linux__
#include <unistd.h>
#endif
FILE* fp;
vg_lite_char filename[30];
#endif

/**************************** Dump Capture ****************************************************/

#ifndef vgliteDUMP_PATH
#   define vgliteDUMP_PATH                      "./"
#endif

#ifndef vgliteDUMP_KEY
#   define vgliteDUMP_KEY                       "process"
#endif

#if DUMP_LAST_CAPTURE
vg_lite_void _SetDumpFileInfo();
vg_lite_error_t vglitefDumpBuffer_single(vg_lite_char* Tag, size_t Physical, vg_lite_pointer Logical, size_t Offset, size_t Bytes);
#define vglitemDUMP_single                      vglitefDump
#define vglitemDUMP_BUFFER_single               vglitefDumpBuffer_single
#endif 
#if DUMP_CAPTURE
vg_lite_void _SetDumpFileInfo();
vg_lite_error_t vglitefDump(vg_lite_char* String, ...);
vg_lite_error_t vglitefDumpBuffer(vg_lite_char* Tag, size_t Physical, vg_lite_pointer Logical, size_t Offset, size_t Bytes);
#define vglitemDUMP                             vglitefDump
#define vglitemDUMP_BUFFER                      vglitefDumpBuffer
#endif

#endif