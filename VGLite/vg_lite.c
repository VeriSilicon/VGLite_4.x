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

#include "vg_lite_context.h"
#include "vg_lite_chip.h"

/* Global context variables and feature table.
*/
vg_lite_context_t   s_context = { 0 };
vg_lite_uint32_t            submit_flag = 0;
vg_lite_char        dump_api_flag = 1;

#if gcFEATURE_VG_SINGLE_COMMAND_BUFFER
vg_lite_uint32_t            command_buffer_size = VG_LITE_COMMAND_BUFFER_SIZE * 2;
#else
vg_lite_uint32_t            command_buffer_size = VG_LITE_COMMAND_BUFFER_SIZE;
#endif

vg_lite_matrix_t    identity_mtx = {
        {
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f }
        },
        1.0f, 1.0f, 0.0f
    };

/* Initialize the feature table of a chip. */
vg_lite_ftable_t    s_ftable = {
    {
        gcFEATURE_VG_IM_INDEX_FORMAT,
        gcFEATURE_VG_SCISSOR,
        gcFEATURE_VG_BORDER_CULLING,
        gcFEATURE_VG_RGBA2_FORMAT,
        gcFEATURE_VG_QUALITY_8X,
        gcFEATURE_VG_IM_FASTCLEAR,
        gcFEATURE_VG_RADIAL_GRADIENT,
        gcFEATURE_VG_GLOBAL_ALPHA,
        gcFEATURE_VG_RGBA8_ETC2_EAC,
        gcFEATURE_VG_COLOR_KEY,
        gcFEATURE_VG_DOUBLE_IMAGE,
        gcFEATURE_VG_YUV_OUTPUT,
        gcFEATURE_VG_FLEXA,
        gcFEATURE_VG_24BIT,
        gcFEATURE_VG_DITHER,
        gcFEATURE_VG_USE_DST,
        gcFEATURE_VG_PE_CLEAR,
        gcFEATURE_VG_IM_INPUT,
        gcFEATURE_VG_DEC_COMPRESS,
        gcFEATURE_VG_LINEAR_GRADIENT_EXT,
        gcFEATURE_VG_MASK,
        gcFEATURE_VG_MIRROR,
        gcFEATURE_VG_GAMMA,
        gcFEATURE_VG_NEW_BLEND_MODE,
        gcFEATURE_VG_STENCIL,
        gcFEATURE_VG_SRC_PREMULTIPLIED,
        gcFEATURE_VG_HW_PREMULTIPLY,
        gcFEATURE_VG_COLOR_TRANSFORMATION,
        gcFEATURE_VG_LVGL_SUPPORT,
        gcFEATURE_VG_INDEX_ENDIAN,
        gcFEATURE_VG_24BIT_PLANAR,
        gcFEATURE_VG_PIXEL_MATRIX,
        gcFEATURE_VG_NEW_IMAGE_INDEX,
        gcFEATURE_VG_PARALLEL_PATHS_DISABLE,
        gcFEATURE_VG_STRIPE_MODE_DISABLE,
        gcFEATURE_VG_IM_DEC_INPUT,
        gcFEATURE_VG_GAUSSIAN_BLUR,
        gcFEATURE_VG_RECTANGLE_TILED_OUT,
        gcFEATURE_VG_TESSELLATION_TILED_OUT,
        gcFEATURE_VG_IM_REPEAT_REFLECT,
        gcFEATURE_VG_YUY2_INPUT,
        gcFEATURE_VG_YUV_INPUT,
        gcFEATURE_VG_YUV_TILED_INPUT,
        gcFEATURE_VG_AYUV_INPUT,
        gcFEATURE_VG_16PIXELS_ALIGNED,
        gcFEATURE_VG_DEC_COMPRESS_2_0,
        gcFEATURE_VG_NV24_INPUT,
        gcFEATURE_VG_TILED_LIMIT,
        gcFEATURE_VG_TILED_MODE,
        gcFEATURE_VG_SRC_ADDRESS_16BYTES_ALIGNED,
        gcFEATURE_VG_SRC_ADDRESS_64BYTES_ALIGNED,
        gcFEATURE_VG_SRC_ADDRESS_DETAIL_ALIGNED,
        gcFEATURE_VG_SRC_ADDRESS_DETAIL_ALIGNED_1,
        gcFEATURE_VG_SRC_TILE_4PIXELS_ALIGNED,
        gcFEATURE_VG_SRC_BUF_ALINGED,
        gcFEATURE_VG_DST_ADDRESS_64BYTES_ALIGNED,
        gcFEATURE_VG_DST_ADDRESS_DETAIL_ALIGNED,
        gcFEATURE_VG_DST_TILE_4PIXELS_ALIGNED,
        gcFEATURE_VG_DST_BUF_ALIGNED,
        gcFEATURE_VG_DST_24BIT_PLANAR_ALIGNED,
        gcFEATURE_VG_DST_BUFLEN_ALIGNED,
        gcFEATURE_VG_FORMAT_SUPPORT_CHECK,
        gcFEATURE_VG_YUV_ALIGNED_CHECK,
        gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE,
        gcFEATURE_VG_DEC_COMPRESS_2_1,
        gcFEATURE_VG_24BIT_PLANAR_SW,
        gcFEATURE_VG_RGB8_ETC2_EAC,
        gcFEATURE_VG_NEW_FACTOR,
        gcFEATURE_VG_NEW_ROI_MASK,
        gcFEATURE_VG_A124_A8L8_L4,
        gcFEATURE_VG_DEC_COMPRESS_2_2,
        gcFEATURE_VG_DEC_COMPRESS_3_0,
        gcFEATURE_VG_MESH_FOR_FRAME,
        gcFEATURE_VG_SIMPLE_BLT,
        gcFEATURE_VG_EXTERNAL_DMA_MESH,
    }
};

#if DUMP_LAST_CAPTURE
vg_lite_uint8_t last_api_call[LAST_CALL_API_NUMS];
vg_lite_uint8_t api_call_nums = 0;

vg_lite_void record_api(vg_lite_api_id_t api_id)
{
    if (api_call_nums < LAST_CALL_API_NUMS)
        last_api_call[api_call_nums++] = api_id;
    else {
        printf("The number of vg api calls in last frame exceeded LAST_CALL_API_NUMS, please increase it");
        api_call_nums++;
    }
}
#endif


static vg_lite_error_t check_hardware_chip_info(vg_lite_void)
{
    vg_lite_uint32_t chip_id = 0, chip_rev = 0, cid = 0, eco_id = 0;

    vg_lite_get_product_info(NULL, &chip_id, &chip_rev);
    vg_lite_get_register(0x30, &cid);
    vg_lite_get_register(0xE8, &eco_id);

    if (CHIPID != chip_id || REVISION != chip_rev || CID != cid || ECOID != eco_id) {
        printf("VGLite API initialization Error!!! \nHardware ChipId: 0x%X  ChipRevision: 0x%X  Cid: 0x%X Ecoid: 0x%X \n", chip_id, chip_rev, cid, eco_id);
        printf("NOT match vg_lite_options.h CHIPID: 0x%X  REVISION: 0x%X  CID: 0x%X Ecoid: 0x%X \n", CHIPID, REVISION, CID, ECOID);
        return VG_LITE_NOT_SUPPORT;
    }

    return VG_LITE_SUCCESS;
}

static inline vg_lite_int32_t has_valid_command_buffer(vg_lite_context_t *context)
{
    if (context == NULL)
        return 0;
    if (context->command_buffer_current >= CMDBUF_COUNT)
        return 0;
    if (context->command_buffer[context->command_buffer_current] == NULL)
        return 0;

    return 1;
}

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
vg_lite_void ptr_modify_cache_command(vg_lite_context_t *context, vg_lite_uint32_t offset, vg_lite_void *data_ptr)
{
    vg_lite_uint32_t data = *(vg_lite_uint32_t*)data_ptr;
    ((vg_lite_uint32_t*)(context->fb_command_buffer + offset))[1] = data;
}

vg_lite_error_t modify_matrix_cache_command(vg_lite_cache_cmd_info *execute_buf, vg_lite_matrix_t *value)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
 
    if (value == NULL)
        return VG_LITE_SUCCESS;
    else
    {
        vg_lite_matrix_t* matrix = value;
        vg_lite_float_t new_matrix[6];
        new_matrix[0] = matrix->m[0][0];
        new_matrix[1] = matrix->m[0][1];
        new_matrix[2] = matrix->m[0][2];
        new_matrix[3] = matrix->m[1][0];
        new_matrix[4] = matrix->m[1][1];
        new_matrix[5] = matrix->m[1][2];

        ptr_modify_cache_command(&s_context, execute_buf->special_register_offset_start, (vg_lite_pointer)&new_matrix[0]);
        ptr_modify_cache_command(&s_context, execute_buf->special_register_offset_start + 8, (vg_lite_pointer)&new_matrix[1]);
        ptr_modify_cache_command(&s_context, execute_buf->special_register_offset_start + 16, (vg_lite_pointer)&new_matrix[2]);
        ptr_modify_cache_command(&s_context, execute_buf->special_register_offset_start + 24, (vg_lite_pointer)&new_matrix[3]);
        ptr_modify_cache_command(&s_context, execute_buf->special_register_offset_start + 32, (vg_lite_pointer)&new_matrix[4]);
        ptr_modify_cache_command(&s_context, execute_buf->special_register_offset_start + 40, (vg_lite_pointer)&new_matrix[5]);
        ptr_modify_cache_command(&s_context, execute_buf->special_register_offset_start + 48, (vg_lite_pointer)&matrix->m[0][2]);
        ptr_modify_cache_command(&s_context, execute_buf->special_register_offset_start + 56, (vg_lite_pointer)&matrix->m[1][2]);
    }

    return error;
}
#endif

typedef vg_lite_float_t  FLOATVECTOR4[4];
static vg_lite_void ClampColor(FLOATVECTOR4 Source, FLOATVECTOR4 Target, vg_lite_uint8_t Premultiplied)
{
    vg_lite_float_t colorMax;
    /* Clamp the alpha channel. */
    Target[3] = CLAMP(Source[3], 0.0f, 1.0f);

    /* Determine the maximum value for the color channels. */
    colorMax = Premultiplied ? Target[3] : 1.0f;

    /* Clamp the color channels. */
    Target[0] = CLAMP(Source[0], 0.0f, colorMax);
    Target[1] = CLAMP(Source[1], 0.0f, colorMax);
    Target[2] = CLAMP(Source[2], 0.0f, colorMax);
}

static vg_lite_uint8_t PackColorComponent(vg_lite_float_t value)
{
    /* Compute the rounded normalized value. */
    vg_lite_float_t rounded = value * 255.0f + 0.5f;

    /* Get the integer part. */
    vg_lite_int32_t roundedInt = (vg_lite_int32_t)rounded;

    /* Clamp to 0..1 range. */
    vg_lite_uint8_t clamped = (vg_lite_uint8_t)CLAMP(roundedInt, 0, 255);

    /* Return result. */
    return clamped;
}

#if DUMP_IMAGE
static vg_lite_void dump_img(vg_lite_void * memory, vg_lite_int32_t width, vg_lite_int32_t height, vg_lite_buffer_format_t format)
{
    FILE * fp;
    vg_lite_char imgname[255] = {'\0'};
    static vg_lite_int32_t num = 1;
    vg_lite_uint32_t* pt = (vg_lite_uint32_t*) memory;
    vg_lite_int32_t i;

    sprintf(imgname, "img_pid%d_%d.txt", getpid(), num++);
    
    fp = fopen(imgname, "w");
    
    if (fp == NULL)
        printf("error!\n");
    

    switch (format) {
        case VG_LITE_INDEX_1:
            for(i = 0; i < width * height / 32; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;
            
        case VG_LITE_INDEX_2:
            for(i = 0; i < width * height / 16; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;
            
        case VG_LITE_INDEX_4:
            for(i = 0; i < width * height / 8; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;
            
        case VG_LITE_INDEX_8:
            for(i = 0; i < width * height / 4; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;

        case VG_LITE_RGBA2222:
            for(i = 0; i < width * height / 4; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;

        case VG_LITE_RGBA4444:
        case VG_LITE_BGRA4444:
        case VG_LITE_RGB565:
        case VG_LITE_BGR565:
            for(i = 0; i < width * height / 2; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;
            
        case VG_LITE_RGBA8888:
        case VG_LITE_BGRA8888:
        case VG_LITE_RGBX8888:
        case VG_LITE_BGRX8888:
            for(i = 0; i < width * height; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;
            
        default:
            break;
    }
    fclose(fp);
    fp = NULL;
}
#endif

static vg_lite_uint32_t rgb_to_l(vg_lite_uint32_t color, vg_lite_buffer_t* target)
{
    vg_lite_uint32_t l = 0;
    switch (target->yuv.yuv2rgb) {
        case VG_LITE_YUV601:
            l = (vg_lite_uint32_t)((0.299f * (vg_lite_float_t)(color & 0xFF)) +
                (0.587f * (vg_lite_float_t)((color >> 8) & 0xFF)) +
                (0.114f * (vg_lite_float_t)((color >> 16) & 0xFF)));
            break;
        case VG_LITE_YUV709:
            l = (vg_lite_uint32_t)((0.2126f * (vg_lite_float_t)(color & 0xFF)) +
                (0.7152f * (vg_lite_float_t)((color >> 8) & 0xFF)) +
                (0.0722f * (vg_lite_float_t)((color >> 16) & 0xFF)));
            break;
        default:
            break;
    }

    switch (target->format) {
        case VG_LITE_L8:
            l = l | l << 24;
            break;
        case VG_LITE_A8L8:
            l = l | (color & 0xFF000000);
            break;
        default:
            break;
    }

    return l;
}

/* Get the bpp information of a color format. */
vg_lite_void get_format_bytes(vg_lite_buffer_format_t format,
                             vg_lite_uint32_t *mul,
                             vg_lite_uint32_t *div,
                             vg_lite_uint32_t *bytes_align)
{
    *mul = *div = 1;
    *bytes_align = 4;
    switch (format) {
        case VG_LITE_A1:
            *div = 8;
            break;

        case VG_LITE_A2:
            *div = 4;
            break;

        case VG_LITE_L4:
        case VG_LITE_A4:
        case VG_LITE_RGB888_ETC2_EAC:
            *div = 2;
            break;

        case VG_LITE_L8:
        case VG_LITE_A8:
        case VG_LITE_RGBA8888_ETC2_EAC:
            break;

        case VG_LITE_ABGR1555:
        case VG_LITE_ARGB1555:
        case VG_LITE_BGRA5551:
        case VG_LITE_RGBA5551:
        case VG_LITE_RGBA4444:
        case VG_LITE_BGRA4444:
        case VG_LITE_ABGR4444:
        case VG_LITE_ARGB4444:
        case VG_LITE_RGB565:
        case VG_LITE_BGR565:
        case VG_LITE_YUYV:
        case VG_LITE_YUY2:
        case VG_LITE_YUY2_TILED:
        /* AYUY2 buffer memory = YUY2 + alpha. */
        case VG_LITE_AYUY2:
        case VG_LITE_AYUY2_TILED:
        /* ABGR8565_PLANAR buffer memory = RGB565 + alpha. */
        case VG_LITE_ABGR8565_PLANAR:
        case VG_LITE_ARGB8565_PLANAR:
        case VG_LITE_RGBA5658_PLANAR:
        case VG_LITE_BGRA5658_PLANAR:
        case VG_LITE_A8L8:
            *mul = 2;
            break;

        case VG_LITE_RGBA8888:
        case VG_LITE_BGRA8888:
        case VG_LITE_ABGR8888:
        case VG_LITE_ARGB8888:
        case VG_LITE_RGBX8888:
        case VG_LITE_BGRX8888:
        case VG_LITE_XBGR8888:
        case VG_LITE_XRGB8888:
            *mul = 4;
            break;

        case VG_LITE_NV12:
        case VG_LITE_NV12_TILED:
            *mul = 1;
            break;

        case VG_LITE_ANV12:
        case VG_LITE_ANV12_TILED:
            *mul = 4;
            break;

        case VG_LITE_INDEX_1:
            *div = 8;
            *bytes_align = 8;
            break;

        case VG_LITE_INDEX_2:
            *div = 4;
            *bytes_align = 8;
            break;

        case VG_LITE_INDEX_4:
            *div = 2;
            *bytes_align = 8;
            break;

        case VG_LITE_INDEX_8:
            *bytes_align = 1;
            break;

        case VG_LITE_RGBA2222:
        case VG_LITE_BGRA2222:
        case VG_LITE_ABGR2222:
        case VG_LITE_ARGB2222:
            *mul = 1;
            break;

        case VG_LITE_RGB888:
        case VG_LITE_BGR888:
        case VG_LITE_ABGR8565:
        case VG_LITE_BGRA5658:
        case VG_LITE_ARGB8565:
        case VG_LITE_RGBA5658:
            *mul = 3;
            break;

        /* OpenVG format*/
        case OPENVG_sRGBX_8888:
        case OPENVG_sRGBX_8888_PRE:
        case OPENVG_sRGBA_8888:
        case OPENVG_sRGBA_8888_PRE:
        case OPENVG_lRGBX_8888:
        case OPENVG_lRGBX_8888_PRE:
        case OPENVG_lRGBA_8888:
        case OPENVG_lRGBA_8888_PRE:
        case OPENVG_sXRGB_8888:
        case OPENVG_sARGB_8888:
        case OPENVG_sARGB_8888_PRE:
        case OPENVG_lXRGB_8888:
        case OPENVG_lARGB_8888:
        case OPENVG_lARGB_8888_PRE:
        case OPENVG_sBGRX_8888:
        case OPENVG_sBGRA_8888:
        case OPENVG_sBGRA_8888_PRE:
        case OPENVG_lBGRX_8888:
        case OPENVG_lBGRA_8888:
        case OPENVG_sXBGR_8888:
        case OPENVG_sABGR_8888:
        case OPENVG_lBGRA_8888_PRE:
        case OPENVG_sABGR_8888_PRE:
        case OPENVG_lXBGR_8888:
        case OPENVG_lABGR_8888:
        case OPENVG_lABGR_8888_PRE:
            *mul = 4;
            break;

        case OPENVG_sRGBA_5551:
        case OPENVG_sRGBA_5551_PRE:
        case OPENVG_lRGBA_5551:
        case OPENVG_lRGBA_5551_PRE:
        case OPENVG_sRGBA_4444:
        case OPENVG_sRGBA_4444_PRE:
        case OPENVG_lRGBA_4444:
        case OPENVG_lRGBA_4444_PRE:
        case OPENVG_sARGB_1555:
        case OPENVG_sARGB_4444:
        case OPENVG_sBGRA_5551:
        case OPENVG_sBGRA_4444:
        case OPENVG_sABGR_1555:
        case OPENVG_sABGR_4444:
        case OPENVG_sRGB_565:
        case OPENVG_sRGB_565_PRE:
        case OPENVG_sBGR_565:
        case OPENVG_lRGB_565:
        case OPENVG_lRGB_565_PRE:
            * mul = 2;
            break;

        case OPENVG_sL_8:
        case OPENVG_lL_8:
        case OPENVG_A_8:
            break;

        case OPENVG_BW_1:
        case OPENVG_A_4:
        case OPENVG_A_1:
            * div = 2;
            break;

        default:
            break;
    }
}

vg_lite_uint8_t is_packed_yuy_format(vg_lite_buffer_format_t format)
{
    return (vg_lite_uint8_t)((format == VG_LITE_YUYV || format == VG_LITE_YUY2 || format == VG_LITE_YUY2_TILED
        || format == VG_LITE_AYUY2 || format == VG_LITE_AYUY2_TILED) ? 1U : 0U);
}

vg_lite_uint8_t is_lvgl_blend_mode(vg_lite_blend_t blend)
{
    return (vg_lite_uint8_t)((blend >= VG_LITE_BLEND_NORMAL_LVGL && blend <= VG_LITE_BLEND_MULTIPLY_LVGL) ? 1U : 0U);
}

/* Packed YUY render targets do not support blending in blit APIs. */
static vg_lite_error_t blit_check_blend_on_yuy_target(vg_lite_blend_t blend, vg_lite_buffer_format_t target_format)
{
    if (blend && is_packed_yuy_format(target_format)) {
        return VG_LITE_NOT_SUPPORT;
    }
    return VG_LITE_SUCCESS;
}

/* Convert VGLite target color format to HW value. */
static vg_lite_uint32_t convert_target_format(vg_lite_buffer_format_t format)
{
    switch (format) {
        case VG_LITE_A1:
            return 0x401;

        case VG_LITE_A2:
            return 0x402;

        case VG_LITE_A4:
            return 0x403;

        case VG_LITE_A8:
            return 0x0;

        case VG_LITE_L8:
            return 0x6;

        case VG_LITE_A8L8:
            return 0x400;

        case VG_LITE_ABGR4444:
            return 0x14;

        case VG_LITE_ARGB4444:
            return 0x34;

        case VG_LITE_RGBA4444:
            return 0x24;

        case VG_LITE_BGRA4444:
            return 0x4;

        case VG_LITE_RGB565:
            return 0x21;

        case VG_LITE_BGR565:
            return 0x1;

        case VG_LITE_ABGR8888:
            return 0x13;

        case VG_LITE_ARGB8888:
            return 0x33;

        case VG_LITE_RGBA8888:
            return 0x23;

        case VG_LITE_BGRA8888:
            return 0x3;

        case VG_LITE_RGBX8888:
            return 0x22;

        case VG_LITE_BGRX8888:
            return 0x2;

        case VG_LITE_XBGR8888:
            return 0x12;

        case VG_LITE_XRGB8888:
            return 0x32;

        case VG_LITE_ABGR1555:
            return 0x15;

        case VG_LITE_RGBA5551:
            return 0x25;

        case VG_LITE_ARGB1555:
            return 0x35;

        case VG_LITE_BGRA5551:
            return 0x5;

        case VG_LITE_YUYV:
        case VG_LITE_YUY2:
        case VG_LITE_YUY2_TILED:
            return 0x8;

        case VG_LITE_NV12:
        case VG_LITE_NV12_TILED:
            return 0xB;

        case VG_LITE_ANV12:
        case VG_LITE_ANV12_TILED:
            return 0xE;

        case VG_LITE_BGRA2222:
            return 0x7;

        case VG_LITE_RGBA2222:
            return 0x27;

        case VG_LITE_ABGR2222:
            return 0x17;

        case VG_LITE_ARGB2222:
            return 0x37;

        case VG_LITE_ARGB8565:
            return 0x3A;

        case VG_LITE_RGBA5658:
            return 0x2A;

        case VG_LITE_ABGR8565:
            return 0x1A;

        case VG_LITE_BGRA5658:
            return 0x0A;

        case VG_LITE_ARGB8565_PLANAR:
            return 0x3C;

        case VG_LITE_RGBA5658_PLANAR:
            return 0x2C;

        case VG_LITE_ABGR8565_PLANAR:
            return 0x1C;

        case VG_LITE_BGRA5658_PLANAR:
            return 0x0C;

        case VG_LITE_RGB888:
            return 0x29;

        case VG_LITE_BGR888:
            return 0x09;

        case VG_LITE_AYUY2:
        case VG_LITE_AYUY2_TILED:
            return 0xF;

        /* OpenVG VGImageFormat */

        case OPENVG_sRGBX_8888:
        case OPENVG_sRGBX_8888_PRE:
            return 0x12;
            break;

        case OPENVG_sRGBA_8888:
        case OPENVG_sRGBA_8888_PRE:
            return 0x13;
            break;

        case OPENVG_sRGB_565:
        case OPENVG_sRGB_565_PRE:
            return 0x1;
            break;

        case OPENVG_sRGBA_5551:
        case OPENVG_sRGBA_5551_PRE:
            return 0x15;
            break;

        case OPENVG_sRGBA_4444:
        case OPENVG_sRGBA_4444_PRE:
            return 0x14;
            break;

        case OPENVG_sL_8:
            return 0x6;
            break;

        case OPENVG_lRGBX_8888:
        case OPENVG_lRGBX_8888_PRE:
            return 0x12;
            break;

        case OPENVG_lRGBA_8888:
        case OPENVG_lRGBA_8888_PRE:
            return 0x13;
            break;

        case OPENVG_lRGB_565:
        case OPENVG_lRGB_565_PRE:
            return 0x1;
            break;

        case OPENVG_lRGBA_5551:
        case OPENVG_lRGBA_5551_PRE:
            return 0x15;
            break;

        case OPENVG_lRGBA_4444:
        case OPENVG_lRGBA_4444_PRE:
            return 0x14;
            break;

        case OPENVG_lL_8:
            return 0x6;
            break;

        case OPENVG_A_8:
            return 0x0;
            break;

        case OPENVG_sXRGB_8888:
            return 0x2;
            break;

        case OPENVG_sARGB_8888:
            return 0x3;
            break;

        case OPENVG_sARGB_8888_PRE:
            return 0x3;
            break;

        case OPENVG_sARGB_1555:
            return 0x5;
            break;

        case OPENVG_sARGB_4444:
            return 0x4;
            break;

        case OPENVG_lXRGB_8888:
            return 0x2;
            break;

        case OPENVG_lARGB_8888:
            return 0x3;
            break;

        case OPENVG_lARGB_8888_PRE:
            return 0x3;
            break;

        case OPENVG_sBGRX_8888:
            return 0x32;
            break;

        case OPENVG_sBGRA_8888:
            return 0x33;
            break;

        case OPENVG_sBGRA_8888_PRE:
            return 0x33;
            break;

        case OPENVG_sBGR_565:
            return 0x21;
            break;

        case OPENVG_sBGRA_5551:
            return 0x35;
            break;

        case OPENVG_sBGRA_4444:
            return 0x34;
            break;

        case OPENVG_lBGRX_8888:
            return 0x32;
            break;

        case OPENVG_lBGRA_8888:
            return 0x33;
            break;

        case OPENVG_lBGRA_8888_PRE:
            return 0x33;
            break;

        case OPENVG_sXBGR_8888:
            return 0x22;
            break;

        case OPENVG_sABGR_8888:
            return 0x23;
            break;

        case OPENVG_sABGR_8888_PRE:
            return 0x23;
            break;

        case OPENVG_sABGR_1555:
            return 0x25;
            break;

        case OPENVG_sABGR_4444:
            return 0x24;
            break;

        case OPENVG_lXBGR_8888:
            return 0x22;
            break;

        case OPENVG_lABGR_8888:
            return 0x23;
            break;

        case OPENVG_lABGR_8888_PRE:
            return 0x23;
            break;

        default:
            return 0xFF;
    }
}

vg_lite_buffer_format_t convert_24bit_format(vg_lite_buffer_format_t format)
{
    switch (format) {
        case VG_LITE_ABGR8565_PLANAR:
            return VG_LITE_ABGR8565;
            break;
        case VG_LITE_BGRA5658_PLANAR:
            return VG_LITE_BGRA5658;
            break;
        case VG_LITE_ARGB8565_PLANAR:
            return VG_LITE_ARGB8565;
            break;
        case VG_LITE_RGBA5658_PLANAR:
            return VG_LITE_RGBA5658;
            break;
        default:
            return VG_LITE_ARGB8565;
            break;
    }
}

#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
/* Convert 24BIT to 24BIT_PLANAR buffer to enable software support 24BIT_PLANAR fotmat.*/
static vg_lite_error_t vg_lite_convert_planar(vg_lite_buffer_t* source, vg_lite_buffer_t* target)
{
    if((target->format >= VG_LITE_ABGR8565_PLANAR) && (target->format <= VG_LITE_RGBA5658_PLANAR))
    {
        if (!target->sw24bit_buffer)
            return VG_LITE_SUCCESS;
    }
    vg_lite_uint32_t i;
    int8_t* data_source = source->memory;
    int8_t* data_rgb = target->memory;
    int8_t* data_alpha = target->yuv.alpha_memory;
    if ((source->stride * source->height % 3) != 0)
        return VG_LITE_INVALID_ARGUMENT;
    vg_lite_uint32_t pixel_size = source->stride * source->height / 3;
    switch (source->format)
    {
    case VG_LITE_ABGR8565:
    case VG_LITE_ARGB8565:
        for (i = 0; i < pixel_size; i++)
        {
            *data_alpha = *data_source;
            data_alpha++;
            data_source++;

            *data_rgb = *data_source;
            data_rgb++;
            data_source++;
            *data_rgb = *data_source;
            data_rgb++;
            data_source++;
        }
        break;
    case VG_LITE_BGRA5658:
    case VG_LITE_RGBA5658:
        for (i = 0; i < pixel_size; i++)
        {
            *data_rgb = *data_source;
            data_rgb++;
            data_source++;
            *data_rgb = *data_source;
            data_rgb++;
            data_source++;

            *data_alpha = *data_source;
            data_alpha++;
            data_source++;
        }
        break;
    default:
        break;
    }
    
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_convert_24bitplanar_to_24bit(vg_lite_buffer_t* source, vg_lite_buffer_t* target)
{
    if ((source->format >= VG_LITE_ABGR8565_PLANAR) && (source->format <= VG_LITE_RGBA5658_PLANAR))
    {
        if (!source->sw24bit_buffer)
            return VG_LITE_SUCCESS;
    }
    vg_lite_uint32_t i;
    int8_t* data_target = target->memory;
    int8_t* data_rgb = source->memory;
    int8_t* data_alpha = source->yuv.alpha_memory;
    if ((target->stride * target->height % 3) != 0)
        return VG_LITE_INVALID_ARGUMENT;
    vg_lite_uint32_t pixel_size = target->stride * target->height / 3;
    switch (source->format)
    {
    case VG_LITE_ABGR8565_PLANAR:
    case VG_LITE_ARGB8565_PLANAR:
        for (i = 0; i < pixel_size; i++)
        {
            *data_target = *data_alpha;
            data_target++;
            data_alpha++;

            *data_target = *data_rgb;
            data_target++;
            data_rgb++;

            *data_target = *data_rgb;
            data_target++;
            data_rgb++;
        }
        break;
    case VG_LITE_BGRA5658_PLANAR:
    case VG_LITE_RGBA5658_PLANAR:
        for (i = 0; i < pixel_size; i++)
        {
            *data_target = *data_rgb;
            data_target++;
            data_rgb++;

            *data_target = *data_rgb;
            data_target++;
            data_rgb++;

            *data_target = *data_alpha;
            data_target++;
            data_alpha++;
        }
        break;
    default:
        break;
    }

    return VG_LITE_SUCCESS;
}
#endif

void vg_flush_previous_rt(void)
{
#if gcFEATURE_VG_SINGLE_COMMAND_BUFFER
    vg_lite_finish();
#else
#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((s_context.rtbuffer->format >= VG_LITE_ABGR8565) && (s_context.rtbuffer->format <= VG_LITE_RGBA5658))
    {
        if (s_context.rtbuffer->sw24bit_planar_buffer)
        {
            vg_lite_finish();
            vg_lite_convert_planar(s_context.rtbuffer, s_context.rtbuffer->sw24bit_planar_buffer);
        }
        else
        {
            vg_lite_flush();
        }
    }
#else  
    vg_lite_flush();
#endif
#endif /* gcFEATURE_VG_SINGLE_COMMAND_BUFFER */
}

#define FORMAT_ALIGNMENT(stride,align) \
    { \
        if ((stride) % (align) != 0) \
            return VG_LITE_INVALID_ARGUMENT; \
        return VG_LITE_SUCCESS; \
    }

#if gcFEATURE_VG_16PIXELS_ALIGNED
/* Determine source IM is aligned by specified bytes */
static vg_lite_error_t _check_source_aligned(vg_lite_buffer_format_t format,vg_lite_uint32_t stride)
{
    switch (format) {
        case VG_LITE_A4:
        case VG_LITE_INDEX_1:
        case VG_LITE_INDEX_2:
        case VG_LITE_INDEX_4:
            FORMAT_ALIGNMENT(stride,8);
            break;

        case VG_LITE_L8:
        case VG_LITE_A8:
        case VG_LITE_INDEX_8:
        case VG_LITE_RGBA2222:
        case VG_LITE_BGRA2222:
        case VG_LITE_ABGR2222:
        case VG_LITE_ARGB2222:
        case VG_LITE_RGBA8888_ETC2_EAC:
            FORMAT_ALIGNMENT(stride,16);
            break;

        case VG_LITE_RGBA4444:
        case VG_LITE_BGRA4444:
        case VG_LITE_ABGR4444:
        case VG_LITE_ARGB4444:
        case VG_LITE_RGB565:
        case VG_LITE_BGR565:
        case VG_LITE_BGRA5551:
        case VG_LITE_RGBA5551:
        case VG_LITE_ABGR1555:
        case VG_LITE_ARGB1555:
        case VG_LITE_YUYV:
        case VG_LITE_YUY2:
        case VG_LITE_NV12:
        case VG_LITE_YV12:
        case VG_LITE_YV24:
        case VG_LITE_YV16:
        case VG_LITE_NV16:
        case VG_LITE_NV24:
        case VG_LITE_ABGR8565_PLANAR:
        case VG_LITE_BGRA5658_PLANAR:
        case VG_LITE_ARGB8565_PLANAR:
        case VG_LITE_RGBA5658_PLANAR:
            FORMAT_ALIGNMENT(stride,32);
            break;

        case VG_LITE_RGB888:
        case VG_LITE_BGR888:
        case VG_LITE_ABGR8565:
        case VG_LITE_BGRA5658:
        case VG_LITE_ARGB8565:
        case VG_LITE_RGBA5658:
            FORMAT_ALIGNMENT(stride,48);
            break;

        case VG_LITE_RGBA8888:
        case VG_LITE_BGRA8888:
        case VG_LITE_ABGR8888:
        case VG_LITE_ARGB8888:
        case VG_LITE_RGBX8888:
        case VG_LITE_BGRX8888:
        case VG_LITE_XBGR8888:
        case VG_LITE_XRGB8888:
            FORMAT_ALIGNMENT(stride,64);
            break;

        default:
            return VG_LITE_SUCCESS;
    }
}
#endif

#if gcFEATURE_VG_SRC_BUF_ALINGED
static vg_lite_error_t _check_source_aligned_2(vg_lite_buffer_format_t format, vg_lite_uint32_t stride)
{
    switch (format) {
    case VG_LITE_A1:
    case VG_LITE_A2:
    case VG_LITE_A4:
    case VG_LITE_A8:
    case VG_LITE_L4:
    case VG_LITE_L8:
    case VG_LITE_INDEX_1:
    case VG_LITE_INDEX_2:
    case VG_LITE_INDEX_4:
    case VG_LITE_INDEX_8:
    case VG_LITE_RGBA2222:
    case VG_LITE_BGRA2222:
    case VG_LITE_ABGR2222:
    case VG_LITE_ARGB2222:
    case VG_LITE_RGBA8888_ETC2_EAC:
        FORMAT_ALIGNMENT(stride, 1);
        break;

    case VG_LITE_A8L8:
    case VG_LITE_RGBA4444:
    case VG_LITE_BGRA4444:
    case VG_LITE_ABGR4444:
    case VG_LITE_ARGB4444:
    case VG_LITE_RGB565:
    case VG_LITE_BGR565:
    case VG_LITE_BGRA5551:
    case VG_LITE_RGBA5551:
    case VG_LITE_ABGR1555:
    case VG_LITE_ARGB1555:
    case VG_LITE_YV16:
    case VG_LITE_NV16:
    case VG_LITE_YV12:
    case VG_LITE_NV12:
    case VG_LITE_YV24:
    case VG_LITE_ABGR8565_PLANAR:
    case VG_LITE_BGRA5658_PLANAR:
    case VG_LITE_ARGB8565_PLANAR:
    case VG_LITE_RGBA5658_PLANAR:
        FORMAT_ALIGNMENT(stride, 2);
        break;

    case VG_LITE_RGB888:
    case VG_LITE_BGR888:
    case VG_LITE_ABGR8565:
    case VG_LITE_BGRA5658:
    case VG_LITE_ARGB8565:
    case VG_LITE_RGBA5658:
        FORMAT_ALIGNMENT(stride, 3);
        break;

    case VG_LITE_YUY2:
    case VG_LITE_RGBA8888:
    case VG_LITE_BGRA8888:
    case VG_LITE_ABGR8888:
    case VG_LITE_ARGB8888:
    case VG_LITE_RGBX8888:
    case VG_LITE_BGRX8888:
    case VG_LITE_XBGR8888:
    case VG_LITE_XRGB8888:
        FORMAT_ALIGNMENT(stride, 4);
        break;

    default:
        return VG_LITE_SUCCESS;
    }
}

static vg_lite_error_t _check_source_aligned_3(vg_lite_buffer_format_t format, vg_lite_uint32_t stride)
{
    switch (format) {
    case VG_LITE_A1:
    case VG_LITE_A2:
    case VG_LITE_INDEX_1:
    case VG_LITE_INDEX_2:
        FORMAT_ALIGNMENT(stride, 1);
        break;

    case VG_LITE_A4:
    case VG_LITE_L4:
    case VG_LITE_INDEX_4:
    case VG_LITE_RGB888_ETC2_EAC:
        FORMAT_ALIGNMENT(stride, 2);
        break;

    case VG_LITE_A8:
    case VG_LITE_L8:
    case VG_LITE_YV24:
    case VG_LITE_INDEX_8:
    case VG_LITE_RGBA2222:
    case VG_LITE_BGRA2222:
    case VG_LITE_ABGR2222:
    case VG_LITE_ARGB2222:
    case VG_LITE_RGBA8888_ETC2_EAC:
        FORMAT_ALIGNMENT(stride, 4);
        break;

    case VG_LITE_A8L8:
    case VG_LITE_RGBA4444:
    case VG_LITE_BGRA4444:
    case VG_LITE_ABGR4444:
    case VG_LITE_ARGB4444:
    case VG_LITE_BGRA5551:
    case VG_LITE_RGBA5551:
    case VG_LITE_ABGR1555:
    case VG_LITE_ARGB1555:
    case VG_LITE_RGB565:
    case VG_LITE_BGR565:
    case VG_LITE_YUY2_TILED:
    case VG_LITE_NV12_TILED:
    case VG_LITE_ABGR8565_PLANAR:
    case VG_LITE_BGRA5658_PLANAR:
    case VG_LITE_ARGB8565_PLANAR:
    case VG_LITE_RGBA5658_PLANAR:
        FORMAT_ALIGNMENT(stride, 8);
        break;

    case VG_LITE_RGB888:
    case VG_LITE_BGR888:
    case VG_LITE_ABGR8565:
    case VG_LITE_BGRA5658:
    case VG_LITE_ARGB8565:
    case VG_LITE_RGBA5658:
        FORMAT_ALIGNMENT(stride, 12);
        break;

    case VG_LITE_RGBA8888:
    case VG_LITE_BGRA8888:
    case VG_LITE_ABGR8888:
    case VG_LITE_ARGB8888:
    case VG_LITE_RGBX8888:
    case VG_LITE_BGRX8888:
    case VG_LITE_XBGR8888:
    case VG_LITE_XRGB8888:
        FORMAT_ALIGNMENT(stride, 16);
        break;

    default:
        return VG_LITE_SUCCESS;
    }
}
#endif

#if gcFEATURE_VG_FORMAT_SUPPORT_CHECK
static vg_lite_error_t _check_format_support_1(vg_lite_buffer_format_t format)
{
    switch (format) {
    case VG_LITE_A8:
    case VG_LITE_L8:
    case VG_LITE_RGBA2222:
    case VG_LITE_BGRA2222:
    case VG_LITE_ABGR2222:
    case VG_LITE_ARGB2222:
    case VG_LITE_RGBA4444:
    case VG_LITE_BGRA4444:
    case VG_LITE_ABGR4444:
    case VG_LITE_ARGB4444:
    case VG_LITE_BGRA5551:
    case VG_LITE_RGBA5551:
    case VG_LITE_ABGR1555:
    case VG_LITE_ARGB1555:
    case VG_LITE_RGB565:
    case VG_LITE_BGR565:
    case VG_LITE_RGB888:
    case VG_LITE_BGR888:
    case VG_LITE_ABGR8565:
    case VG_LITE_BGRA5658:
    case VG_LITE_ARGB8565:
    case VG_LITE_RGBA5658:
    case VG_LITE_RGBA8888:
    case VG_LITE_BGRA8888:
    case VG_LITE_ABGR8888:
    case VG_LITE_ARGB8888:
    case VG_LITE_RGBX8888:
    case VG_LITE_BGRX8888:
    case VG_LITE_XBGR8888:
    case VG_LITE_XRGB8888:
        break;
    default:
        return VG_LITE_NOT_SUPPORT;
    }

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t _check_format_support_2(vg_lite_buffer_format_t format)
{
    switch (format) {
    case VG_LITE_INDEX_1:
    case VG_LITE_INDEX_2:
    case VG_LITE_INDEX_4:
    case VG_LITE_INDEX_8:
    case VG_LITE_A4:
    case VG_LITE_YUY2:
    case VG_LITE_YUY2_TILED:
    case VG_LITE_RGBA8888_ETC2_EAC:
        break;
    default:
        return VG_LITE_NOT_SUPPORT;
    }

    return VG_LITE_SUCCESS;
}
#endif

vg_lite_error_t peclear_align_check(vg_lite_buffer_t* target, vg_lite_int32_t y) {
    vg_lite_error_t error = VG_LITE_SUCCESS;

#if gcFEATURE_VG_DST_ADDRESS_DETAIL_ALIGNED
    if ((vg_lite_uint32_t)(target->address) % 64 != 0) {
        return VG_LITE_INVALID_ARGUMENT;
    }
#endif

    if (target->compress_mode == VG_LITE_DEC_DISABLE) {

#if gcFEATURE_VG_DST_BUFLEN_ALIGNED
        vg_lite_uint32_t align, mul, div;
        get_format_bytes(target->format, &mul, &div, &align);
        if ((mul / div != 3) && ((target->stride * y) % 64 != 0)) {
            return VG_LITE_INVALID_ARGUMENT;
        }
#endif

#if gcFEATURE_VG_24BIT
        vg_lite_uint32_t align1, mul1, div1;
        get_format_bytes(target->format, &mul1, &div1, &align1);
        if ((mul1 / div1 == 3) && ((target->stride * y) % 48 != 0)) {
            return VG_LITE_INVALID_ARGUMENT;
        }
#endif

    }
    else
        VG_LITE_RETURN_ERROR(check_compress_stride_align(target->format, target->stride * y));

    return error;
}

vg_lite_error_t srcbuf_align_check(vg_lite_buffer_t* source)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

#if gcFEATURE_VG_FORMAT_SUPPORT_CHECK
    if (_check_format_support_1(source->format) && _check_format_support_2(source->format)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif

#if gcFEATURE_VG_SRC_ADDRESS_64BYTES_ALIGNED
    if ((vg_lite_uint32_t)(source->address) % 64 != 0) {
        printf("buffer address need to be aglined to 64 bytes.");
        return VG_LITE_INVALID_ARGUMENT;
    }
#endif

#if gcFEATURE_VG_SRC_ADDRESS_DETAIL_ALIGNED
    vg_lite_uint32_t align, mul, div, bpp;
    get_format_bytes(source->format, &mul, &div, &align);
    bpp = 8 * mul / div;

    if (bpp == 8) {
        if ((vg_lite_uint32_t)(source->address) % 16 != 0) {
            printf("buffer address need to be aglined to 16 bytes.");
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
    else if (bpp == 16) {
        if ((vg_lite_uint32_t)(source->address) % 32 != 0) {
            printf("buffer address need to be aglined to 32 bytes.");
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
    else if (bpp == 24 || bpp == 32) {
        if ((vg_lite_uint32_t)(source->address) % 64 != 0) {
            printf("buffer address need to be aglined to 64 bytes.");
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
    else {
        if ((vg_lite_uint32_t)(source->address) % 8 != 0) {
            printf("buffer address need to be aglined to 8 bytes.");
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
#endif

#if gcFEATURE_VG_SRC_ADDRESS_DETAIL_ALIGNED_1
    vg_lite_uint32_t align, mul, div, bpp;
    get_format_bytes(source->format, &mul, &div, &align);
    bpp = 8 * mul / div;

    if (bpp == 32) {
        if ((vg_lite_uint32_t)(source->address) % 16 != 0) {
            printf("buffer address need to be aglined to 16 bytes.");
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
    else {
        if ((vg_lite_uint32_t)(source->address) % 8 != 0) {
            printf("buffer address need to be aglined to 8 bytes.");
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
#endif

#if gcFEATURE_VG_SRC_BUF_ALINGED
#if gcFEATURE_VG_SRC_ADDRESS_16BYTES_ALIGNED 
    if (source->format == VG_LITE_ARGB8888 ||
        source->format == VG_LITE_BGRA8888 ||
        source->format == VG_LITE_ABGR8888 ||
        source->format == VG_LITE_ARGB8888
        )
    {
        if ((vg_lite_uint32_t)(source->address) % 16 != 0) {
            printf("buffer address need to be aglined to 16 bytes.");
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
    else
#endif
    {
        if ((vg_lite_uint32_t)(source->address) % 8 != 0) {
            printf("buffer address need to be aglined to 8 bytes.");
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
#endif

    if (source->tiled == VG_LITE_TILED) {
#if gcFEATURE_VG_SRC_TILE_4PIXELS_ALIGNED
        vg_lite_uint32_t align, mul, div;
        get_format_bytes(source->format, &mul, &div, &align);
        if ((source->stride % (4 * mul / div) != 0) || (source->height % 4 != 0)) {
            return VG_LITE_INVALID_ARGUMENT;
        }
#endif

#if gcFEATURE_VG_SRC_BUF_ALINGED
        error = _check_source_aligned_3(source->format, source->stride);
        if (error != VG_LITE_SUCCESS) {
            return VG_LITE_INVALID_ARGUMENT;
        }
#endif

#if gcFEATURE_VG_YUV_ALIGNED_CHECK
        if (source->format == VG_LITE_YUY2) {

#if !gcFEATURE_VG_SRC_ADDRESS_DETAIL_ALIGNED_1
            if (source->address % 32 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
#endif

            if (source->tiled) {
                if (source->stride % 8 != 0) {
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
            else {
                if (source->stride % 32 != 0) {
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
        }
#endif

#if gcFEATURE_VG_YUV_DETAIL_ALIGNED_CHECK
        if (source->format == VG_LITE_NV12_TILED || source->format == VG_LITE_NV24_TILED) {
            if (source->yuv.uv_planar % 8 != 0 || source->yuv.uv_stride % 8 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
#endif
    }

    if (source->tiled == VG_LITE_LINEAR) {
#if gcFEATURE_VG_16PIXELS_ALIGNED
        vg_lite_uint32_t align, mul, div;
        get_format_bytes(source->format, &mul, &div, &align);
        if (source->stride % (16 * mul / div) != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
#endif

#if gcFEATURE_VG_SRC_BUF_ALINGED
        error = _check_source_aligned_2(source->format, source->stride);
        if (error != VG_LITE_SUCCESS) {
            return VG_LITE_INVALID_ARGUMENT;
        }
#endif

#if gcFEATURE_VG_YUV_ALIGNED_CHECK
#if !gcFEATURE_VG_SRC_ADDRESS_DETAIL_ALIGNED_1
        if (source->format == VG_LITE_NV12 || source->format == VG_LITE_NV16) {
            if (source->address % 32 != 0 || source->stride % 32 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }

            if (source->yuv.uv_planar % 32 != 0 || source->yuv.uv_stride % 32 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }

        if (source->format == VG_LITE_YV12 || source->format == VG_LITE_YV16) {
            if (source->address % 32 != 0 || source->stride % 32 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }

            if (source->yuv.uv_planar % 16 != 0 || source->yuv.uv_stride % 16 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }

            if (source->yuv.v_planar % 16 != 0 || source->yuv.v_stride % 16 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }

        if (source->format == VG_LITE_YV24) {
            if (source->address % 32 != 0 || source->stride % 32 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }

            if (source->yuv.uv_planar % 32 != 0 || source->yuv.uv_stride % 32 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }

            if (source->yuv.v_planar % 32 != 0 || source->yuv.v_stride % 32 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
#endif
#endif

#if gcFEATURE_VG_YUV_DETAIL_ALIGNED_CHECK
        if (source->format == VG_LITE_NV12 || source->format == VG_LITE_NV16 || source->format == VG_LITE_NV24) {
             if (source->yuv.uv_planar % 8 != 0 || source->yuv.uv_stride % 2 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }

        if (source->format == VG_LITE_YV12 || source->format == VG_LITE_YV16 || source->format == VG_LITE_YV24) {
            if (source->yuv.uv_planar % 8 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }

            if (source->yuv.v_planar % 8 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
#endif

#if gcFEATURE_VG_YUV_HEIGHT_ALIGNED_CHECK
        if (source->format == VG_LITE_NV12) {
            if (source->height % 2 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
#endif
    }

    if (source->compress_mode != VG_LITE_DEC_DISABLE) {
        VG_LITE_RETURN_ERROR(check_compress_stride_align(source->format, source->stride* source->height));
    }

    return error;
}

vg_lite_error_t dstbuf_align_check(vg_lite_buffer_t* target)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t align, mul, div;
#if gcFEATURE_VG_TILED_LIMIT || gcFEATURE_VG_DST_BUF_ALIGNED
    vg_lite_uint32_t bpp;
#endif
#if gcFEATURE_VG_TILED_LIMIT
    vg_lite_uint32_t tile_flag = 0;
    vg_lite_uint32_t tile_flag1 = 0;
#endif
    get_format_bytes(target->format, &mul, &div, &align);
#if gcFEATURE_VG_TILED_LIMIT || gcFEATURE_VG_DST_BUF_ALIGNED
    bpp = 8 * mul / div;
#endif

#if gcFEATURE_VG_FORMAT_SUPPORT_CHECK
    if (_check_format_support_1(target->format)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
    
#if gcFEATURE_VG_DST_TILE_4PIXELS_ALIGNED
    if (target->tiled == VG_LITE_TILED) {
        if ((target->stride % (4 * mul / div) != 0) || (target->height % 4 != 0)) {
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
#endif

    if (target->compress_mode == VG_LITE_DEC_DISABLE) {
#if gcFEATURE_VG_DST_BUF_ALIGNED
        if (target->tiled == VG_LITE_TILED) {
            if (bpp == 8 || bpp == 16 || bpp == 32) {
                if (target->stride % (4 * mul / div)) {
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }

            if (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658) {
                if (target->stride % 12 != 0) {
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }

            if (target->format >= VG_LITE_ABGR8565_PLANAR && target->format <= VG_LITE_RGBA5658_PLANAR) {
                if (target->stride % 8 != 0) {
                    return VG_LITE_INVALID_ARGUMENT;
                }
#if gcFEATURE_VG_DST_24BIT_PLANAR_ALIGNED_1
                if (target->alpha_stride % 4 != 0) {
                    return VG_LITE_INVALID_ARGUMENT;
                }
#endif
            }
        }
        else {
            if (bpp == 8 || bpp == 16 || bpp == 32) {
                if (target->stride % (mul / div)) {
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }

            if (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658) {
                if (target->stride % 3 != 0) {
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }

            if (target->format >= VG_LITE_ABGR8565_PLANAR && target->format <= VG_LITE_RGBA5658_PLANAR) {
                if (target->stride % 2 != 0) {
                    return VG_LITE_INVALID_ARGUMENT;
                }
            }
        }

        if (bpp == 8 || bpp == 16 || bpp == 32)
        {
            if ((vg_lite_uint32_t)(target->address) % 4 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }

        if (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658) {
            if ((vg_lite_uint32_t)(target->address) % 64 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
#endif

#if gcFEATURE_VG_DST_ADDRESS_64BYTES_ALIGNED
        if ((vg_lite_uint32_t)(target->address) % 64 != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
#endif

#if gcFEATURE_VG_DST_24BIT_PLANAR_ALIGNED
        if (target->format >= VG_LITE_ABGR8565_PLANAR && target->format <= VG_LITE_RGBA5658_PLANAR) {
            if ((vg_lite_uint32_t)(target->address) % 32 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
            if ((vg_lite_uint32_t)(target->yuv.alpha_planar) % 16 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
#endif

#if gcFEATURE_VG_DST_24BIT_PLANAR_ALIGNED_1
        if (target->format >= VG_LITE_ABGR8565_PLANAR && target->format <= VG_LITE_RGBA5658_PLANAR) {
            if ((vg_lite_uint32_t)(target->address) % 64 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
            if ((vg_lite_uint32_t)(target->yuv.alpha_planar) % 64 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
#endif

    }
    else {
        VG_LITE_RETURN_ERROR(check_compress_target_address_align(target->address));
        VG_LITE_RETURN_ERROR(check_compress_stride_align(target->format, target->stride * target->height));
    }

#if gcFEATURE_VG_DST_ADDRESS_DETAIL_ALIGNED
    if (bpp == 24 || target->tiled == VG_LITE_TILED) {
        if ((vg_lite_uint32_t)(target->address) % 64 != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
    else {
        if ((vg_lite_uint32_t)(target->address) % 4 != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
    }

    if (target->tiled == VG_LITE_TILED) {
        if (target->height % 4 != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
        if (bpp == 24) {
            if ((vg_lite_uint32_t)(target->stride) % 12 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        else {
            if ((vg_lite_uint32_t)(target->stride) % 16 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
    }
    else {
        if ((vg_lite_uint32_t)(target->stride) % (bpp / 8) != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
#endif

#if gcFEATURE_VG_TILED_LIMIT
    if (target->tiled == VG_LITE_TILED) {
#if gcFEATURE_VG_RECTANGLE_TILED_OUT
        tile_flag1 = 1;
#else
        tile_flag1 = 0;
#endif
        tile_flag = 1;
    }
#endif

#if (gcFEATURE_VG_TILED_LIMIT == 1)
    if (tile_flag1 ^ tile_flag) {
        if (bpp != 24) {
            if (target->stride % 64 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        else {
            if (target->stride % 48 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
    }
#elif (gcFEATURE_VG_TILED_LIMIT == 2)
    if (tile_flag1 ^ tile_flag) {
        return VG_LITE_INVALID_ARGUMENT;
    }
#elif (gcFEATURE_VG_TILED_LIMIT == 3)
    if (target->address % 4 != 0) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    if (mul / div)
    {
        if (target->stride % (mul / div) != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
    }

    if (target->compress_mode || bpp == 24) {
        if (target->address % 64 != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
    if (target->tiled) {
        if (bpp != 24) {
            if (target->stride % 16 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        else {
            if (target->stride % 12 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        if (target->address % 64 != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
    }

    if (tile_flag1 ^ tile_flag) {
        if (target->address % 64 != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
        if (bpp != 24) {
            if (target->stride % 64 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
        else {
            if (target->stride % 48 != 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }
        }
    }
#endif
    return error;
}


/* Convert VGLite source color format to HW values. */
vg_lite_uint32_t convert_source_format(vg_lite_buffer_format_t format)
{
    switch (format) {
        case VG_LITE_L4:
            return 0x80000;

        case VG_LITE_L8:
            return 0x0;

        case VG_LITE_A1:

            return 0x80003;

        case VG_LITE_A2:
            return 0x80001;

        case VG_LITE_A4:
            return 0x1;

        case VG_LITE_A8:
            return 0x2;

        case VG_LITE_A8L8:
            return 0x80002;

        case VG_LITE_RGBA4444:
            return 0x23;

        case VG_LITE_BGRA4444:
            return 0x3;

        case VG_LITE_ABGR4444:
            return 0x13;

        case VG_LITE_ARGB4444:
            return 0x33;

        case VG_LITE_RGB565:
            return 0x25;

        case VG_LITE_BGR565:
            return 0x5;

        case VG_LITE_RGBA8888:
            return 0x27;

        case VG_LITE_BGRA8888:
            return 0x7;

        case VG_LITE_ABGR8888:
            return 0x17;

        case VG_LITE_ARGB8888:
            return 0x37;

        case VG_LITE_RGBX8888:
            return 0x26;

        case VG_LITE_BGRX8888:
            return 0x6;

        case VG_LITE_XBGR8888:
            return 0x16;

        case VG_LITE_XRGB8888:
            return 0x36;

        case VG_LITE_BGRA5551:
            return 0x4;

        case VG_LITE_RGBA5551:
            return 0x24;

        case VG_LITE_ABGR1555:
            return 0x14;

        case VG_LITE_ARGB1555:
            return 0x34;

        case VG_LITE_YUYV:
            return 0x8;

        case VG_LITE_YUY2:
        case VG_LITE_YUY2_TILED:
            return 0x8;

        case VG_LITE_NV12:
        case VG_LITE_NV12_TILED:
            return 0xB;

        case VG_LITE_ANV12:
        case VG_LITE_ANV12_TILED:
            return 0xE;

        case VG_LITE_YV12:
            return 0x9;

        case VG_LITE_YV24:
            return 0xD;

        case VG_LITE_YV16:
            return 0xC;

        case VG_LITE_NV16:
            return 0xA;

        case VG_LITE_NV24:
        case VG_LITE_NV24_TILED:
            return 0xD | (1<<19);

        case VG_LITE_AYUY2:
        case VG_LITE_AYUY2_TILED:
            return 0xF;

        case VG_LITE_INDEX_1:
            return 0x200;

        case VG_LITE_INDEX_2:
            return 0x400;

        case VG_LITE_INDEX_4:
            return 0x600;

        case VG_LITE_INDEX_8:
            return 0x800;

        case VG_LITE_RGBA2222:
            return 0xA20;

        case VG_LITE_BGRA2222:
            return 0xA00;

        case VG_LITE_ABGR2222:
            return 0xA10;

        case VG_LITE_ARGB2222:
            return 0xA30;

        case VG_LITE_RGBA8888_ETC2_EAC:
#if !gcFEATURE_VG_RGB8_ETC2_EAC
            return 0xE00;
#else
            return 0xE07;
#endif

        case VG_LITE_RGB888_ETC2_EAC:
            return 0x20000E00;

        case VG_LITE_ARGB8565:
            return 0x40000030;

        case VG_LITE_RGBA5658:
            return 0x40000020;

        case VG_LITE_ABGR8565:
            return 0x40000010;

        case VG_LITE_BGRA5658:
            return 0x40000000;

        case VG_LITE_RGB888:
            return 0x20000020;

        case VG_LITE_BGR888:
            return 0x20000000;

        case VG_LITE_ARGB8565_PLANAR:
            return 0x60000030;

        case VG_LITE_RGBA5658_PLANAR:
            return 0x60000020;

        case VG_LITE_ABGR8565_PLANAR:
            return 0x60000010;

        case VG_LITE_BGRA5658_PLANAR:
            return 0x60000000;

        /* OpenVG VGImageFormat */
        case OPENVG_sRGBX_8888:
        case OPENVG_sRGBX_8888_PRE:
            return 0x16;
            break;

        case OPENVG_sRGBA_8888:
        case OPENVG_sRGBA_8888_PRE:
            return 0x17;
            break;

        case OPENVG_sRGB_565:
        case OPENVG_sRGB_565_PRE:
            return 0x5;
            break;

        case OPENVG_sRGBA_5551:
        case OPENVG_sRGBA_5551_PRE:
            return 0x14;
            break;

        case OPENVG_sRGBA_4444:
        case OPENVG_sRGBA_4444_PRE:
            return 0x13;
            break;

        case OPENVG_sL_8:
            return 0x0;
            break;

        case OPENVG_lRGBX_8888:
        case OPENVG_lRGBX_8888_PRE:
            return 0x16;
            break;

        case OPENVG_lRGBA_8888:
        case OPENVG_lRGBA_8888_PRE:
            return 0x17;
            break;

        case OPENVG_lRGB_565:
        case OPENVG_lRGB_565_PRE:
            return 0x5;
            break;

        case OPENVG_lRGBA_5551:
        case OPENVG_lRGBA_5551_PRE:
            return 0x14;
            break;

        case OPENVG_lRGBA_4444:
        case OPENVG_lRGBA_4444_PRE:
            return 0x13;
            break;

        case OPENVG_lL_8:
            return 0x0;
            break;

        case OPENVG_A_8:
            return 0x2;
            break;

        case OPENVG_BW_1:
            return 0x200;
            break;

        case OPENVG_A_1:
            return 0x1;
            break;

        case OPENVG_A_4:
            return 0x1;
            break;

        case OPENVG_sXRGB_8888:
            return 0x6;
            break;

        case OPENVG_sARGB_8888:
            return 0x7;
            break;

        case OPENVG_sARGB_8888_PRE:
            return 0x7;
            break;

        case OPENVG_sARGB_1555:
            return 0x4;
            break;

        case OPENVG_sARGB_4444:
            return 0x3;
            break;

        case OPENVG_lXRGB_8888:
            return 0x6;
            break;

        case OPENVG_lARGB_8888:
            return 0x7;
            break;
        case OPENVG_lARGB_8888_PRE:
            return 0x7;
            break;

        case OPENVG_sBGRX_8888:
            return 0x36;
            break;

        case OPENVG_sBGRA_8888:
            return 0x37;
            break;

        case OPENVG_sBGRA_8888_PRE:
            return 0x37;
            break;

        case OPENVG_sBGR_565:
            return 0x25;
            break;

        case OPENVG_sBGRA_5551:
            return 0x34;
            break;

        case OPENVG_sBGRA_4444:
            return 0x33;
            break;

        case OPENVG_lBGRX_8888:
            return 0x36;
            break;

        case OPENVG_lBGRA_8888:
            return 0x37;
            break;

        case OPENVG_lBGRA_8888_PRE:
            return 0x37;
            break;

        case OPENVG_sXBGR_8888:
            return 0x26;
            break;

        case OPENVG_sABGR_8888:
            return 0x27;
            break;

        case OPENVG_sABGR_8888_PRE:
            return 0x27;
            break;

        case OPENVG_sABGR_1555:
            return 0x24;
            break;

        case OPENVG_sABGR_4444:
            return 0x23;
            break;

        case OPENVG_lXBGR_8888:
            return 0x26;
            break;

        case OPENVG_lABGR_8888:
            return 0x27;
            break;

        case OPENVG_lABGR_8888_PRE:
            return 0x27;
            break;

        default:
            return 0;
            break;
    }
}

/* Convert VGLite blend modes to HW values. */
vg_lite_uint32_t convert_blend(vg_lite_blend_t blend)
{
    switch (blend) {
        case VG_LITE_BLEND_SRC_OVER:
        case VG_LITE_BLEND_NORMAL_LVGL:
        case OPENVG_BLEND_SRC_OVER:
            return 0x00000100;

        case VG_LITE_BLEND_DST_OVER:
        case OPENVG_BLEND_DST_OVER:
            return 0x00000200;

        case VG_LITE_BLEND_SRC_IN:
        case OPENVG_BLEND_SRC_IN:
            return 0x00000300;

        case VG_LITE_BLEND_DST_IN:
        case OPENVG_BLEND_DST_IN:
            return 0x00000400;

        case VG_LITE_BLEND_MULTIPLY:
        case VG_LITE_BLEND_MULTIPLY_LVGL:
        case OPENVG_BLEND_MULTIPLY:
            return 0x00000500;

        case VG_LITE_BLEND_SCREEN:
        case OPENVG_BLEND_SCREEN:
            return 0x00000600;

        case VG_LITE_BLEND_DARKEN:
        case OPENVG_BLEND_DARKEN:
            return 0x00000700;

        case VG_LITE_BLEND_LIGHTEN:
        case OPENVG_BLEND_LIGHTEN:
            return 0x00000800;

        case VG_LITE_BLEND_ADDITIVE:
        case VG_LITE_BLEND_ADDITIVE_LVGL:
        case OPENVG_BLEND_ADDITIVE:
            return 0x00000900;

        case VG_LITE_BLEND_SUBTRACT:
            return 0x00000A00;

        case VG_LITE_BLEND_SUBTRACT_LVGL:
#if gcFEATURE_VG_LVGL_SUPPORT
            return 0x00000C00;
#else
            return 0x00000A00;
#endif

        default:
            return 0;
    }
}

/* Check the input porter duff factor is conforms to one of VG_LITE_BLEND_NONE/VG_LITE_BLEND_SRC_IN/VG_LITE_BLEND_DST_IN. */
vg_lite_uint32_t check_porter_duff_factor_match(vg_lite_blend_t blend, vg_lite_porter_duff_config_t porter_duff_config)
{
#if gcFEATURE_VG_NEW_FACTOR
    if (blend != VG_LITE_PORTER_DUFF_BLEND)
        return 0;

    if (
        /* case1: porter duff factor configuration is same as VG_LITE_BLEND_NONE. */
        (porter_duff_config.factor_src_alpha == VG_LITE_BLEND_FACTOR_ALPHA_ZERO
            && porter_duff_config.factor_src_color == VG_LITE_BLEND_FACTOR_COLOR_ZERO
            && porter_duff_config.factor_dst_alpha == VG_LITE_BLEND_FACTOR_ALPHA_ZERO
            && porter_duff_config.factor_dst_color == VG_LITE_BLEND_FACTOR_COLOR_ZERO
            && porter_duff_config.final_equation_opcode == VG_LITE_BLEND_FUNC_ADD
            && porter_duff_config.srcchannelmode == VG_LITE_CHANNEL_MODE_PREMULTIPLY
            && porter_duff_config.dstchannelmode == VG_LITE_CHANNEL_MODE_PREMULTIPLY)
        ||
        /* case2: porter duff factor configuration is same as VG_LITE_BLEND_SRC_IN. */
        (porter_duff_config.factor_src_alpha == VG_LITE_BLEND_FACTOR_ALPHA_DEST_ALPHA
            && porter_duff_config.factor_src_color == VG_LITE_BLEND_FACTOR_COLOR_DEST_ALPHA
            && porter_duff_config.factor_dst_alpha == VG_LITE_BLEND_FACTOR_ALPHA_ZERO
            && porter_duff_config.factor_dst_color == VG_LITE_BLEND_FACTOR_COLOR_ZERO
            && porter_duff_config.final_equation_opcode == VG_LITE_BLEND_FUNC_ADD
            && porter_duff_config.srcchannelmode == VG_LITE_CHANNEL_MODE_PREMULTIPLY
            && porter_duff_config.dstchannelmode == VG_LITE_CHANNEL_MODE_PREMULTIPLY)
        ||
        /* case3: porter duff factor configuration is same as VG_LITE_BLEND_DST_IN. */
        (porter_duff_config.factor_src_alpha == VG_LITE_BLEND_FACTOR_ALPHA_ONE
            && porter_duff_config.factor_src_color == VG_LITE_BLEND_FACTOR_COLOR_ONE
            && porter_duff_config.factor_dst_alpha == VG_LITE_BLEND_FACTOR_ALPHA_SRC_ALPHA
            && porter_duff_config.factor_dst_color == VG_LITE_BLEND_FACTOR_COLOR_SRC_ALPHA
            && porter_duff_config.final_equation_opcode == VG_LITE_BLEND_FUNC_ADD
            && porter_duff_config.srcchannelmode == VG_LITE_CHANNEL_MODE_PREMULTIPLY
            && porter_duff_config.dstchannelmode == VG_LITE_CHANNEL_MODE_PREMULTIPLY)
        )
        return 1;
#endif
    return 0;
}

/* Config parameters according to the blend mode. */
vg_lite_void config_factor_parameter(vg_lite_blend_t blend, vg_lite_porter_duff_config_t porter_duff_config, vg_factor_config_t* factor_config)
{
    if (factor_config == NULL)
    {
        return;
    }

    switch (blend) 
    {
    case VG_LITE_BLEND_NONE:
    case OPENVG_BLEND_SRC:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_ZERO;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_ZERO;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        break;
    case VG_LITE_BLEND_SRC_OVER:
    case OPENVG_BLEND_SRC_OVER:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_ZERO;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_INV_SRC_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        break;
    case  VG_LITE_BLEND_DST_OVER:
    case OPENVG_BLEND_DST_OVER:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_DEST_ALPHA;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ONE;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_ONE;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        break;
    case  VG_LITE_BLEND_SRC_IN:
    case OPENVG_BLEND_SRC_IN:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_DEST_ALPHA;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_DEST_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_ZERO;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        break;
    case  VG_LITE_BLEND_DST_IN:
    case OPENVG_BLEND_DST_IN:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ONE;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_ONE;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_SRC_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        break;
    case  VG_LITE_BLEND_MULTIPLY:
    case OPENVG_BLEND_MULTIPLY:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA_PLUS;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_INV_SRC_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        break;
    case  VG_LITE_BLEND_SCREEN:
    case OPENVG_BLEND_SCREEN:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA_MUL_DEST_COLOR;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_ONE;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        break;
    case  VG_LITE_BLEND_DARKEN:
    case OPENVG_BLEND_DARKEN:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_INV_SRC_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_MIN;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        break;
    case  VG_LITE_BLEND_LIGHTEN:
    case OPENVG_BLEND_LIGHTEN:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_INV_SRC_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_MAX;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        break;
    case  VG_LITE_BLEND_ADDITIVE:
    case OPENVG_BLEND_ADDITIVE:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_ZERO;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ONE;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_ONE;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        break;
    case VG_LITE_BLEND_SUBTRACT:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ONE;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_ONE;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_INV_SRC_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_PREMULTIPLY;
        break;
    case VG_LITE_BLEND_NORMAL_LVGL:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_SRC_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_INV_SRC_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_SRC_COLOR;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_DEST_COLOR;
        break;
    case VG_LITE_BLEND_ADDITIVE_LVGL:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_SRC_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_INV_SRC_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_SRC_ADD_DEST;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_DEST_COLOR;
        break;
    case VG_LITE_BLEND_SUBTRACT_LVGL:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_SRC_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_INV_SRC_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_DEST_SUB_SRC;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_DEST_COLOR;
        break;
    case VG_LITE_BLEND_MULTIPLY_LVGL:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_SRC_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_INV_SRC_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_SRC_MUL_DEST;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_DEST_COLOR;
        break;
    case VG_LITE_BLEND_DIFFERENCE_LVGL:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_SRC_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_INV_SRC_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_DEST_SUB_SRC_ABS;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_DEST_COLOR;
        break;
    case SVG2_BLEND_NORMAL:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_DEST_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_SRC_COLOR;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_SRC_COLOR;
        break;
    case SVG2_BLEND_MULTIPLY:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_DEST_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_SRC_COLOR;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_SRC_MUL_DEST;
        break;
    case SVG2_BLEND_SCREEN:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_DEST_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_SRC_COLOR;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_SRC_ADD_DEST_MINUS;
        break;
    case SVG2_BLEND_LIGHTEN:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_DEST_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_SRC_COLOR;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_MAX_SRC_DEST;
        break;
    case SVG2_BLEND_DARKEN:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_DEST_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_SRC_COLOR;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_MIN_SRC_DEST;
        break;
    case SVG2_BLEND_DIFFERENCE:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_DEST_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_SRC_COLOR;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_DEST_SUB_SRC_ABS;
        break;
    case SVG2_BLEND_EXCLUSION:
        factor_config->factor_src_alpha = VG_LITE_BLEND_FACTOR_ALPHA_ZERO;
        factor_config->factor_src_color = VG_LITE_BLEND_FACTOR_COLOR_INV_DEST_ALPHA;
        factor_config->factor_dst_alpha = VG_LITE_BLEND_FACTOR_ALPHA_INV_SRC_ALPHA;
        factor_config->factor_dst_color = VG_LITE_BLEND_FACTOR_COLOR_DEST_ALPHA;
        factor_config->final_equation_opcode = VG_LITE_BLEND_FUNC_ADD;
        factor_config->srcchannelmode = VG_LITE_CHANNEL_MODE_SRC_COLOR;
        factor_config->dstchannelmode = VG_LITE_CHANNEL_MODE_SRC_ADD_DEST_MINUS2;
        break;
    case VG_LITE_PORTER_DUFF_BLEND:
        if (s_context.porter_duff_enable)
        {
            factor_config->factor_src_alpha = porter_duff_config.factor_src_alpha;
            factor_config->factor_src_color = porter_duff_config.factor_src_color;
            factor_config->factor_dst_alpha = porter_duff_config.factor_dst_alpha;
            factor_config->factor_dst_color = porter_duff_config.factor_dst_color;
            factor_config->final_equation_opcode = porter_duff_config.final_equation_opcode;
            factor_config->dstchannelmode = porter_duff_config.dstchannelmode;
            factor_config->srcchannelmode = porter_duff_config.srcchannelmode;
        }
        else
        {
            VGLITE_LOG("Porter duff factor is not configured, please configure it via vg_lite_blend_func() first.\n");
            return;
        }
        break;
    default:
        break;
    }
};

/* Convert VGLite uv swizzle enums to HW values. */
vg_lite_uint32_t convert_uv_swizzle(vg_lite_swizzle_t swizzle)
{
    switch (swizzle) {
        case VG_LITE_SWIZZLE_UV:
            return 0x00000040;
            break;
            
        case VG_LITE_SWIZZLE_VU:
            return 0x00000050;
            
        default:
            return 0;
            break;
    }
}

/* Convert VGLite yuv standard enums to HW values. */
vg_lite_uint32_t convert_yuv2rgb(vg_lite_yuv2rgb_t yuv)
{
    switch (yuv) {
        case VG_LITE_YUV601:
            return 0;
            break;
            
        case VG_LITE_YUV709:
            return 0x00008000;
            
        default:
            return 0;
            break;
    }
}

static vg_lite_error_t submit(vg_lite_context_t * context);
static vg_lite_error_t stall(vg_lite_context_t * context, vg_lite_uint32_t time_ms, vg_lite_uint32_t mask);
static vg_lite_error_t handle_cmd_overflow(vg_lite_uint32_t size, vg_lite_context_t* context);
#if gcFEATURE_VG_MESH_FOR_FRAME
static vg_lite_error_t mesh_rt_reset(vg_lite_buffer_t* target);
static vg_lite_error_t vg_lite_target_mesh_copy(vg_lite_buffer_t* target);
#endif

/* Push a state array into current command buffer. */
vg_lite_error_t push_clut(vg_lite_context_t * context, vg_lite_uint32_t address, vg_lite_uint32_t count, vg_lite_uint32_t *data)
{
    vg_lite_uint32_t i;
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    VG_LITE_RETURN_ERROR(handle_cmd_overflow(CMDBUF_OFFSET(*context) + 8 + VG_LITE_ALIGN(count + 1, 2) * 4, context));

    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_STATES(count, address);

    for (i = 0; i < count; i++) {
        ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1 + i] = data[i];
    }
    if (i % 2 == 0) {
        ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1 + i] = VG_LITE_NOP();
    }

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
    if (context->fb_command_offset + VG_LITE_ALIGN(count + 1, 2) * 4 > CACHE_COMMAND_BUFFER_SIZE) {
        printf("The size of cache is %d bytes, exceeding the capacity of cache buffer(%d bytes)!\n", context->fb_command_offset + VG_LITE_ALIGN(count + 1, 2) * 4, CACHE_COMMAND_BUFFER_SIZE);
        printf("Please clear cache or increase cache buffer size!\n");
        return VG_LITE_OUT_OF_MEMORY;
    }

    if (context->backup_fb_command_flag) {
        ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[0] = VG_LITE_STATES(count, address);

        for (i = 0; i < count; i++) {
            ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[1 + i] = data[i];
        }

        if (i % 2 == 0) {
            ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[1 + i] = VG_LITE_NOP();
        }
    }

    context->fb_command_offset += VG_LITE_ALIGN(count + 1, 2) * 4;
#endif

#if DUMP_COMMAND
    {
        vg_lite_uint32_t loops;
        if (strncmp(filename, "Commandbuffer", 13)) {
            sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
        }

        fp = fopen(filename, "a");

        if (fp == NULL)
            printf("error!\n");

        fprintf(fp, "Command buffer: 0x%08x, ",
                ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0]);

        for (loops = 0; loops < count / 2; loops++) {
            fprintf(fp, "0x%08x,\nCommand buffer: 0x%08x, ",
                    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(loops + 1) * 2 - 1],
                    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(loops + 1) * 2]);
        }

        fprintf(fp, "0x%08x,\n",
                ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(loops + 1) * 2 - 1]);

        fclose(fp);
        fp = NULL;
    }
#endif

    CMDBUF_OFFSET(*context) += VG_LITE_ALIGN(count + 1, 2) * 4;

    return VG_LITE_SUCCESS;
}

/* Push a single state command into the current command buffer. */
vg_lite_error_t push_state(vg_lite_context_t * context, vg_lite_uint32_t address, vg_lite_uint32_t data)
{
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    /* TODO wait for hw to complete development. */
    /* if (address == 0x0A1B || context->hw.hw_states[address & 0xff].state != data || !context->hw.hw_states[address & 0xff].init) */
    {
        VG_LITE_RETURN_ERROR(handle_cmd_overflow(CMDBUF_OFFSET(*context) + 16, context));

        /* TODO context->hw.hw_states[address & 0xff].state = data;
        context->hw.hw_states[address & 0xff].init = 1;*/

        ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_STATE(address);
        ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = data;

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
        if (context->fb_command_offset + 8 > CACHE_COMMAND_BUFFER_SIZE) {
            printf("The size of cache is %d bytes, exceeding the capacity of cache buffer(%d bytes)!\n", context->fb_command_offset + 8, CACHE_COMMAND_BUFFER_SIZE);
            printf("Please clear cache or increase cache buffer size!\n");
            return VG_LITE_OUT_OF_MEMORY;
        }

        if (context->backup_fb_command_flag) {
            ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[0] = VG_LITE_STATE(address);
            ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[1] = data;

            if (s_context.fb_command_buffer_end->special_register_address && address == s_context.fb_command_buffer_end->special_register_address)
                s_context.fb_command_buffer_end->special_register_offset_start = context->fb_command_offset;

            context->fb_command_offset += 8;
        }
#endif

#if DUMP_COMMAND
        if (strncmp(filename, "Commandbuffer", 13)) {
            sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
        }

        fp = fopen(filename, "a");

        if (fp == NULL)
            printf("error!\n");

        fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
                ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0],
                ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1]);

        fclose(fp);
        fp = NULL;
#endif

        CMDBUF_OFFSET(*context) += 8;
    }

    return VG_LITE_SUCCESS;
}

/* Push a single state command with given address. */
vg_lite_error_t push_state_ptr(vg_lite_context_t * context, vg_lite_uint32_t address, vg_lite_void * data_ptr)
{
    vg_lite_error_t error;
    vg_lite_uint32_t data = *(vg_lite_uint32_t *) data_ptr;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    /* TODO wait for hw to complete development. */
    /* if (address == 0x0A1B || context->hw.hw_states[address & 0xff].state != data || !context->hw.hw_states[address & 0xff].init) */
    {
        VG_LITE_RETURN_ERROR(handle_cmd_overflow(CMDBUF_OFFSET(*context) + 16, context));

        /* TODO context->hw.hw_states[address & 0xff].state = data;
        context->hw.hw_states[address & 0xff].init = 1;*/

        ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_STATE(address);
        ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = data;

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
        if (context->fb_command_offset + 8 > CACHE_COMMAND_BUFFER_SIZE) {
            printf("The size of cache is %d bytes, exceeding the capacity of cache buffer(%d bytes)!\n", context->fb_command_offset + 8, CACHE_COMMAND_BUFFER_SIZE);
            printf("Please clear cache or increase cache buffer size!\n");
            return VG_LITE_OUT_OF_MEMORY;
        }

        if (context->backup_fb_command_flag) {
            ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[0] = VG_LITE_STATE(address);
            ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[1] = data;

            if (s_context.fb_command_buffer_end->special_register_address && address == s_context.fb_command_buffer_end->special_register_address)
                s_context.fb_command_buffer_end->special_register_offset_start = context->fb_command_offset;

            context->fb_command_offset += 8;
        }
#endif

#if DUMP_COMMAND
        if (strncmp(filename, "Commandbuffer", 13)) {
            sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
        }

        fp = fopen(filename, "a");

        if (fp == NULL)
            printf("error!\n");
        fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
                ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0],
                ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1]);

        fclose(fp);
        fp = NULL;
#endif
        CMDBUF_OFFSET(*context) += 8;
    }

    return VG_LITE_SUCCESS;
}

/* Push a "call" command into the current command buffer. */
vg_lite_error_t push_call(vg_lite_context_t * context, vg_lite_uint32_t address, vg_lite_uint32_t bytes)
{
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    VG_LITE_RETURN_ERROR(handle_cmd_overflow(CMDBUF_OFFSET(*context) + 16, context));

    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_CALL((bytes + 7) / 8);
    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = address;

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
    if (context->fb_command_offset + 8 > CACHE_COMMAND_BUFFER_SIZE) {
        printf("The size of cache is %d bytes, exceeding the capacity of cache buffer(%d bytes)!\n", context->fb_command_offset + 8, CACHE_COMMAND_BUFFER_SIZE);
        printf("Please clear cache or increase cache buffer size!\n");
        return VG_LITE_OUT_OF_MEMORY;
    }

    if (context->backup_fb_command_flag) {
        ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[0] = VG_LITE_CALL((bytes + 7) / 8);
        ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[1] = address;

        context->fb_command_offset += 8;
    }
#endif

#if DUMP_COMMAND
    if (strncmp(filename, "Commandbuffer", 13)) {
        sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
    }

    fp = fopen(filename, "a");

    if (fp == NULL)
        printf("error!\n");
    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0],
            ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1]);

    fclose(fp);
    fp = NULL;
#endif

    CMDBUF_OFFSET(*context) += 8;

#if !gcFEATURE_VG_CMD_CALL_FIX_DISABLE
    VG_LITE_RETURN_ERROR(push_stall(&s_context, 0x10));
#endif

    return VG_LITE_SUCCESS;
}

#if gcFEATURE_VG_PE_CLEAR
static vg_lite_error_t push_pe_clear(vg_lite_context_t * context, vg_lite_uint32_t size)
{
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    VG_LITE_RETURN_ERROR(handle_cmd_overflow(CMDBUF_OFFSET(*context) + 16, context));

    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_DATA(1);
    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;
    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[2] = size;
    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[3] = 0;

    CMDBUF_OFFSET(*context) += 16;

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
    if (context->fb_command_offset + 16 > CACHE_COMMAND_BUFFER_SIZE) {
        printf("The size of cache is %d bytes, exceeding the capacity of cache buffer(%d bytes)!\n", context->fb_command_offset + 16, CACHE_COMMAND_BUFFER_SIZE);
        printf("Please clear cache or increase cache buffer size!\n");
        return VG_LITE_OUT_OF_MEMORY;
    }

    if (context->backup_fb_command_flag) {
        ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[0] = VG_LITE_DATA(1);
        ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[1] = 0;
        ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[2] = size;
        ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[3] = 0;

        context->fb_command_offset += 16;
    }
#endif

    return VG_LITE_SUCCESS;
}
#endif

/* Push a rectangle command into the current command buffer. */
static vg_lite_error_t push_rectangle(vg_lite_context_t * context, vg_lite_int32_t x, vg_lite_int32_t y, vg_lite_int32_t width, vg_lite_int32_t height)
{
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    VG_LITE_RETURN_ERROR(handle_cmd_overflow(CMDBUF_OFFSET(*context) + 16, context));

    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_DATA(1);
    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;
    ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[4] = (uint16_t)x;
    ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[5] = (uint16_t)y;
    ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[6] = (uint16_t)width;
    ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[7] = (uint16_t)height;

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
    if (context->fb_command_offset + 16 > CACHE_COMMAND_BUFFER_SIZE) {
        printf("The size of cache is %d bytes, exceeding the capacity of cache buffer(%d bytes)!\n", context->fb_command_offset + 16, CACHE_COMMAND_BUFFER_SIZE);
        printf("Please clear cache or increase cache buffer size!\n");
        return VG_LITE_OUT_OF_MEMORY;
    }

    if (context->backup_fb_command_flag) {
        ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[0] = VG_LITE_DATA(1);
        ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[1] = 0;
        ((uint16_t *) (context->fb_command_buffer + context->fb_command_offset))[4] = (uint16_t)x;
        ((uint16_t *) (context->fb_command_buffer + context->fb_command_offset))[5] = (uint16_t)y;
        ((uint16_t *) (context->fb_command_buffer + context->fb_command_offset))[6] = (uint16_t)width;
        ((uint16_t *) (context->fb_command_buffer + context->fb_command_offset))[7] = (uint16_t)height;

        context->fb_command_offset += 16;
    }
#endif

#if DUMP_COMMAND
    if (strncmp(filename, "Commandbuffer", 13)) {
        sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
    }

    fp = fopen(filename, "a");

    if (fp == NULL)
        printf("error!\n");

    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0], 0);

    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[5] << 16 |
            ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[4],
            ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[7] << 16 |
            ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[6]);

    fclose(fp);
    fp = NULL;
#endif

    CMDBUF_OFFSET(*context) += 16;

    return VG_LITE_SUCCESS;
}

/* Push a data array into the current command buffer. */
vg_lite_error_t push_data(vg_lite_context_t * context, vg_lite_uint32_t size, vg_lite_void * data)
{
    vg_lite_error_t error;
    vg_lite_uint32_t bytes = VG_LITE_ALIGN(size, 8);

    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    VG_LITE_RETURN_ERROR(handle_cmd_overflow(CMDBUF_OFFSET(*context) + 16 + bytes, context));

    /* Command buffer size must be at least data size "bytes" plus header and END command */
    if ((bytes + 16) > CMDBUF_SIZE(*context)) {
        printf("Command buffer size needs increase for data sized %d bytes!\n", (vg_lite_int32_t)(bytes + 16));
        return VG_LITE_OUT_OF_RESOURCES;
    }

    ((uint64_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(bytes >> 3)] = 0;
    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_DATA((bytes >> 3));
    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;
    memcpy(CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context) + 8, data, size);

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
    if (context->fb_command_offset + 8 + bytes > CACHE_COMMAND_BUFFER_SIZE) {
        printf("The size of cache is %d bytes, exceeding the capacity of cache buffer(%d bytes)!\n", context->fb_command_offset + 8 + bytes, CACHE_COMMAND_BUFFER_SIZE);
        printf("Please clear cache or increase cache buffer size!\n");
        return VG_LITE_OUT_OF_MEMORY;
    }

    if (context->backup_fb_command_flag) {
        ((uint64_t *) (context->fb_command_buffer + context->fb_command_offset))[(bytes >> 3)] = 0;
        ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[0] = VG_LITE_DATA((bytes >> 3));;
        ((vg_lite_uint32_t *) (context->fb_command_buffer + context->fb_command_offset))[1] = 0;
        memcpy(context->fb_command_buffer + context->fb_command_offset + 8, data, size);

        context->fb_command_offset += 8 + bytes;
    }
#endif

#if DUMP_COMMAND
    {
        vg_lite_int32_t loops;

        if (strncmp(filename, "Commandbuffer", 13)) {
            sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
        }

        fp = fopen(filename, "a");

        if (fp == NULL)
            printf("error!\n");

        fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
                ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0], 0);
        for (loops = 0; loops < (bytes >> 3); loops++) {
            fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
                   ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(loops + 1) * 2],
                   ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(loops + 1) * 2 + 1]);
        }

        fclose(fp);
        fp = NULL;
    }
#endif

    CMDBUF_OFFSET(*context) += 8 + bytes;

    return VG_LITE_SUCCESS;
}

/* Push a "stall" command into the current command buffer. */
vg_lite_error_t push_stall(vg_lite_context_t * context, vg_lite_uint32_t module)
{
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    VG_LITE_RETURN_ERROR(handle_cmd_overflow(CMDBUF_OFFSET(*context) + 16, context));

    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_SEMAPHORE(module);
    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;
    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[2] = VG_LITE_STALL(module);
    ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[3] = 0;

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
    if (context->fb_command_offset + 16 > CACHE_COMMAND_BUFFER_SIZE) {
        printf("The size of cache is %d bytes, exceeding the capacity of cache buffer(%d bytes)!\n", context->fb_command_offset + 16, CACHE_COMMAND_BUFFER_SIZE);
        printf("Please clear cache or increase cache buffer size!\n");
        return VG_LITE_OUT_OF_MEMORY;
    }

    if (context->backup_fb_command_flag) {
        ((vg_lite_uint32_t *)(context->fb_command_buffer + context->fb_command_offset))[0] = VG_LITE_SEMAPHORE(module);
        ((vg_lite_uint32_t *)(context->fb_command_buffer + context->fb_command_offset))[1] = 0;
        ((vg_lite_uint32_t *)(context->fb_command_buffer + context->fb_command_offset))[2] = VG_LITE_STALL(module);
        ((vg_lite_uint32_t *)(context->fb_command_buffer + context->fb_command_offset))[3] = 0;

        context->fb_command_offset += 16;
    }
#endif

#if DUMP_COMMAND
    if (strncmp(filename, "Commandbuffer", 13)) {
        sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
    }

    fp = fopen(filename, "a");

    if (fp == NULL)
        printf("error!\n");

    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0], 0);
    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[2], 0);

    fclose(fp);
    fp = NULL;
#endif

    CMDBUF_OFFSET(*context) += 16;

    return VG_LITE_SUCCESS;
}

/* Submit the current command buffer to HW and reset the current command buffer offset. */
static vg_lite_error_t submit(vg_lite_context_t *context)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_submit_t submit;

#if gcdVG_ENABLE_DELAY_RESUME
    vg_lite_kernel_delay_resume_t delay_resume;
    delay_resume.query_delay_resume = 1;
    vg_lite_int32_t resume_flag = vg_lite_kernel(VG_LITE_QUERY_DELAY_RESUME, &delay_resume);

    if (resume_flag == 1) {
        /* Reset GPU. */
        vg_lite_kernel_reset_t reset;
        reset.delay_resume_flag = 1;
        vg_lite_kernel(VG_LITE_RESET, &reset);
        printf("Delay resume success! \n");

#ifdef __ZEPHYR__
        /* If delay resume is enabled, power and clock would be turned on during the reset process. */
        /* Disable GPU clocking*/
        vg_lite_kernel_gpu_clock_state_t gpu_state;
        gpu_state.state = VG_LITE_GPU_STOP;
        vg_lite_kernel(VG_LITE_SET_GPU_CLOCK_STATE, &gpu_state);
#endif
}
#endif

    /* Check if there is a valid context and an allocated command buffer. */
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    /* Check if there is anything to submit. */
    if (CMDBUF_OFFSET(*context) == 0)
        return VG_LITE_INVALID_ARGUMENT;

#if 0
    /* This case is safe as command buffer is allocated with (command_buffer_size + 8) bytes */
    if (CMDBUF_OFFSET(*context) + 8 >= CMDBUF_SIZE(*context)) {
        /* Reset command buffer offset. */
        CMDBUF_OFFSET(*context) = 0;
        return VG_LITE_OUT_OF_RESOURCES;
    }
#endif

    /* Wait if GPU has not completed previous CMD buffer */
    if (submit_flag)
    {
        VG_LITE_RETURN_ERROR(stall(&s_context, 0, (vg_lite_uint32_t)~0));
    }

#if gcFEATURE_VG_MESH_FOR_FRAME
    vg_lite_kernel_mesh_info_t mesh_data;
    if (s_context.mesh_mode) {
        if (s_context.mesh_dirty == 0) {
            s_context.mesh_dirty = 1;
            mesh_data.width = s_context.rtbuffer->width;
            mesh_data.height = s_context.rtbuffer->height;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_SET_TARGET_MESH_W_H, &mesh_data));
#if DUMP_CAPTURE
            vg_lite_uint32_t mesh_w_h;
            mesh_w_h = mesh_data.width | (mesh_data.height << 16);
            vglitemDUMP_BUFFER("AHB", 0x530, &mesh_w_h, 0, 4);
#endif
        }
        else {
            printf("Excluding frame bound settings\n");
            return VG_LITE_INVALID_ARGUMENT;
        }   

#if gcFEATURE_VG_SIMPLE_BLT
        if (s_context.mesh_mode == VG_LITE_MESH_COPY_INTERNAL) {
            VG_LITE_RETURN_ERROR(vg_lite_target_mesh_copy(s_context.rtbuffer));
        }
#endif 
    }
#endif

#if gcFEATURE_VG_FLEXA
    vg_lite_kernel_flexa_info_t flexa_data;
    if (s_context.sync_mode) {
        if (s_context.flexa_dirty == 0) {
            s_context.flexa_dirty = 1;
            flexa_data.target_width = s_context.rtbuffer->width;
            flexa_data.target_height = s_context.rtbuffer->height;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FLEXA_SET_TARGET_W_H, &flexa_data));
        }
}
#endif

    /* Append END command into the command buffer. */
    if (s_context.frame_flag == VG_LITE_FRAME_END_FLAG) {
        /* A interrupt will be received to indicate that the GPU is idle. */
        ((vg_lite_uint32_t *)(CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_END(EVENT_FRAME_END);
        ((vg_lite_uint32_t *)(CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
        if (context->backup_fb_command_flag && context->fb_finish_flag) {
            ((vg_lite_uint32_t *)(context->fb_command_buffer + context->fb_command_offset))[0] = VG_LITE_END(EVENT_FRAME_END);
            ((vg_lite_uint32_t *)(context->fb_command_buffer + context->fb_command_offset))[1] = 0;

            context->fb_command_offset += 8;
            context->fb_finish_flag = 0;
        }
#endif
    }
    else {
        /* A interrupt will be received to indicate that the GPU has completed the current instruction. */
        ((vg_lite_uint32_t *)(CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_END(EVENT_END);
        ((vg_lite_uint32_t *)(CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
        if (context->backup_fb_command_flag && context->fb_finish_flag) {
            ((vg_lite_uint32_t *)(context->fb_command_buffer + context->fb_command_offset))[0] = VG_LITE_END(EVENT_END);
            ((vg_lite_uint32_t *)(context->fb_command_buffer + context->fb_command_offset))[1] = 0;

            context->fb_command_offset += 8;
            context->fb_finish_flag = 0;
        }
#endif
    }

    s_context.frame_flag = VG_LITE_END_FLAG;

#if DUMP_COMMAND
    if (strncmp(filename, "Commandbuffer", 13)) {
        sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
    }

    fp = fopen(filename, "a");

    if (fp == NULL)
        printf("error!\n");

    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((vg_lite_uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0], 0);

    fprintf(fp, "Command buffer addr is : %p,\n", CMDBUF_BUFFER(*context));
    fprintf(fp, "Command buffer offset is : %d,\n", CMDBUF_OFFSET(*context) + 8);

    fclose(fp);
    fp = NULL;
#endif

    CMDBUF_OFFSET(*context) += 8;

    /* Submit the command buffer. */
    submit.context = &context->context;
    submit.commands = CMDBUF_BUFFER(*context);
    submit.command_size = CMDBUF_OFFSET(*context);
    submit.command_id = CMDBUF_INDEX(*context);

#if DUMP_LAST_CAPTURE
    //backup command
    context->Physical = (size_t)CMDBUF_BUFFER(*context);
    context->last_command_buffer_logical = submit.context->command_buffer_logical[CMDBUF_INDEX(*context)];
    context->last_command_size = submit.command_size;
#endif

    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_SUBMIT, &submit));

    submit_flag = 1;

#if DUMP_CAPTURE
    vglitemDUMP_BUFFER("command", (size_t)CMDBUF_BUFFER(*context),
        submit.context->command_buffer_logical[CMDBUF_INDEX(*context)], 0, submit.command_size);
    vglitemDUMP("@[commit]");
#endif

    /* Reset command buffer. */
    CMDBUF_OFFSET(*context) = 0;

    return error;
}

/* Wait for the HW to finish the current execution. */
static vg_lite_error_t stall(vg_lite_context_t * context, vg_lite_uint32_t time_ms, vg_lite_uint32_t mask)
{
    vg_lite_error_t error;
    vg_lite_kernel_wait_t wait;

#if DUMP_CAPTURE
    vglitemDUMP("@[stall]");
#endif

    if (submit_flag == 0)
        return VG_LITE_SUCCESS;

    /* Wait until GPU is ready. */
    wait.context = &context->context;
    wait.timeout_ms = time_ms > 0 ? time_ms : VG_LITE_INFINITE;
    wait.event_mask = mask;
    wait.reset_type = RESTORE_ALL_COMMAND;
    wait.event_got = 0;
    error = vg_lite_kernel(VG_LITE_WAIT, &wait);

#if DUMP_LAST_CAPTURE
    if (error == VG_LITE_TIMEOUT)
    {
        for (vg_lite_int32_t i = 0; i < api_call_nums && i < LAST_CALL_API_NUMS; i++)
            vglitemDUMP_single("%d ", last_api_call);

        vglitemDUMP_BUFFER_single("command", context->Physical,
            context->last_command_buffer_logical, 0, context->last_command_size);
    }
    api_call_nums = 0;
#endif

    submit_flag = 0;
#if defined(_WINDLL)
    return VG_LITE_SUCCESS;
#else
    return error;
#endif
}

/* Handle cmd overflow accordingly */
static vg_lite_error_t handle_cmd_overflow(vg_lite_uint32_t size, vg_lite_context_t* context) {
    vg_lite_error_t error;
    if (size >= CMDBUF_SIZE(*context)) {
#if gcFEATURE_VG_SIMPLE_BLT || gcFEATURE_VG_EXTERNAL_DMA_MESH
        if ((s_context.mesh_mode == VG_LITE_MESH_COPY_INTERNAL) || (s_context.mesh_mode == VG_LITE_MESH_COPY_EXTERNAL)) {
            printf("Command buffer size needs increase for sized %d bytes!\n", size);
            return VG_LITE_OUT_OF_RESOURCES;
        }
#endif
        VG_LITE_RETURN_ERROR(submit(context));
        VG_LITE_RETURN_ERROR(stall(context, 0, (vg_lite_uint32_t)~0));
#if gcFEATURE_VG_MESH_FOR_FRAME
        VG_LITE_RETURN_ERROR(mesh_rt_reset(context->rtbuffer));
#endif
    }
    return VG_LITE_SUCCESS;
}

/* Get the inversion of a matrix. */
vg_lite_uint32_t inverse(vg_lite_matrix_t * result, vg_lite_matrix_t * matrix)
{
    vg_lite_float_t det00, det01, det02;
    vg_lite_float_t d;
    vg_lite_int32_t isAffine;

    /* Test for identity matrix. */
    if (matrix == NULL) {
        result->m[0][0] = 1.0f;
        result->m[0][1] = 0.0f;
        result->m[0][2] = 0.0f;
        result->m[1][0] = 0.0f;
        result->m[1][1] = 1.0f;
        result->m[1][2] = 0.0f;
        result->m[2][0] = 0.0f;
        result->m[2][1] = 0.0f;
        result->m[2][2] = 1.0f;

        /* Success. */
        return 1;
    }

    det00 = (matrix->m[1][1] * matrix->m[2][2]) - (matrix->m[2][1] * matrix->m[1][2]);
    det01 = (matrix->m[2][0] * matrix->m[1][2]) - (matrix->m[1][0] * matrix->m[2][2]);
    det02 = (matrix->m[1][0] * matrix->m[2][1]) - (matrix->m[2][0] * matrix->m[1][1]);
    
    /* Compute determinant. */
    d = (matrix->m[0][0] * det00) + (matrix->m[0][1] * det01) + (matrix->m[0][2] * det02);

    /* Return 0 if there is no inverse matrix. */
    if (d == 0.0f)
        return 0;

    /* Compute reciprocal. */
    d = 1.0f / d;

    /* Determine if the matrix is affine. */
    isAffine = (matrix->m[2][0] == 0.0f) && (matrix->m[2][1] == 0.0f) && (matrix->m[2][2] == 1.0f);

    result->m[0][0] = d * det00;
    result->m[0][1] = d * ((matrix->m[2][1] * matrix->m[0][2]) - (matrix->m[0][1] * matrix->m[2][2]));
    result->m[0][2] = d * ((matrix->m[0][1] * matrix->m[1][2]) - (matrix->m[1][1] * matrix->m[0][2]));
    result->m[1][0] = d * det01;
    result->m[1][1] = d * ((matrix->m[0][0] * matrix->m[2][2]) - (matrix->m[2][0] * matrix->m[0][2]));
    result->m[1][2] = d * ((matrix->m[1][0] * matrix->m[0][2]) - (matrix->m[0][0] * matrix->m[1][2]));
    result->m[2][0] = isAffine ? 0.0f : d * det02;
    result->m[2][1] = isAffine ? 0.0f : d * ((matrix->m[2][0] * matrix->m[0][1]) - (matrix->m[0][0] * matrix->m[2][1]));
    result->m[2][2] = isAffine ? 1.0f : d * ((matrix->m[0][0] * matrix->m[1][1]) - (matrix->m[1][0] * matrix->m[0][1]));

    /* Success. */
    return 1;
}

/* Transform a 2D point by a given matrix. */
vg_lite_uint32_t transform(vg_lite_point_t * result, vg_lite_float_t x, vg_lite_float_t y, vg_lite_matrix_t * matrix)
{
    vg_lite_float_t pt_x;
    vg_lite_float_t pt_y;
    vg_lite_float_t pt_w;
    
    /* Test for identity matrix. */
    if (matrix == NULL) {
        result->x = (vg_lite_int32_t)x;
        result->y = (vg_lite_int32_t)y;
        
        /* Success. */
        return 1;
    }

    if (((matrix->m[0][1] != 0.0f) || (matrix->m[1][0] != 0.0f) || (matrix->m[2][0] != 0.0f) || (matrix->m[2][1] != 0.0f) || (matrix->m[2][2] != 1.0f)) &&
        (s_context.filter == VG_LITE_FILTER_LINEAR || s_context.filter == VG_LITE_FILTER_BI_LINEAR)) {
        if (x != 0) {
            x = x + 0.5f;
        }

        if (y != 0 && s_context.filter == VG_LITE_FILTER_BI_LINEAR) {
            y = y + 0.5f;
        }
    }
    
    /* Transform x, y, and w. */
    pt_x = (x * matrix->m[0][0]) + (y * matrix->m[0][1]) + matrix->m[0][2];
    pt_y = (x * matrix->m[1][0]) + (y * matrix->m[1][1]) + matrix->m[1][2];
    pt_w = (x * matrix->m[2][0]) + (y * matrix->m[2][1]) + matrix->m[2][2];
    
    if (pt_w <= 0.0f) {
        s_context.filter = 0;
        return 0;
    }
    
    /* Compute projected x and y. */
    if (pt_x < 0)
    {
        result->x = (vg_lite_int32_t)((pt_x / pt_w) - 0.5f);
    }
    else
    {
        result->x = (vg_lite_int32_t)((pt_x / pt_w) + 0.5f);
    }
    if (pt_y < 0)
    {
        result->y = (vg_lite_int32_t)((pt_y / pt_w) - 0.5f);
    }
    else
    {
        result->y = (vg_lite_int32_t)((pt_y / pt_w) + 0.5f);
    }
    
    /* Success. */
    return 1;
}

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
/* Transform a 2D float point by a given matrix. */
vg_lite_uint32_t transform_float(vg_lite_float_point_t* result, vg_lite_float_t x, vg_lite_float_t y, vg_lite_matrix_t* matrix)
{
    vg_lite_float_t pt_x;
    vg_lite_float_t pt_y;
    vg_lite_float_t pt_w;

    /* Test for identity matrix. */
    if (matrix == NULL) {
        result->x = truncf(x);
        result->y = truncf(y);

        /* Success. */
        return 1;
    }

    /* Transform x, y, and w. */
    pt_x = (x * matrix->m[0][0]) + (y * matrix->m[0][1]) + matrix->m[0][2];
    pt_y = (x * matrix->m[1][0]) + (y * matrix->m[1][1]) + matrix->m[1][2];
    pt_w = (x * matrix->m[2][0]) + (y * matrix->m[2][1]) + matrix->m[2][2];

    if (pt_w <= 0.0f)
        return 0;

    /* Compute projected x and y. */
    result->x = (pt_x / pt_w);
    result->y = (pt_y / pt_w);

    /* Success. */
    return 1;
}
#endif

/* Flush specific VG module. */
static vg_lite_error_t flush_target(vg_lite_void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_context_t *context = GET_CONTEXT();
    
    do {
        VG_LITE_BREAK_ERROR(push_state(context, 0x0A1B, 0x00000011));
        VG_LITE_BREAK_ERROR(push_stall(context, 7));
    } while (VGL_FALSE);
    
    return error;
}

/* Allocate memory for image. */
static vg_lite_error_t image_buffer_update(vg_lite_buffer_t *image, vg_lite_uint32_t width)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    VG_LITE_CHECK_NULL_POINTER(image);

    /* Check if the memory is reusable. */
    if (image->handle != NULL)
    {
        if ((vg_lite_uint32_t)image->width >= width)
            return VG_LITE_SUCCESS;
        /* If the memory size is insufficient, release the old memory first. */
        VG_LITE_RETURN_ERROR(vg_lite_free(image));
    }

    /* Allocate the color ramp surface. */
    memset(image, 0, sizeof(vg_lite_buffer_t));
    image->width = width;
    image->height = 1;
    image->stride = 0;
    image->image_mode = VG_LITE_NONE_IMAGE_MODE;
    image->format = VG_LITE_ABGR8888;

    /* Allocate image buffer. */
    VG_LITE_RETURN_ERROR(vg_lite_allocate(image));

    return error;
}

/* Set the current render target. */
vg_lite_error_t set_render_target(vg_lite_buffer_t *target)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t yuv2rgb = 0;
    vg_lite_uint32_t uv_swiz = 0;
    vg_lite_uint32_t tile_setting = 0;
    vg_lite_uint32_t compress_mode = 0;
    vg_lite_uint32_t mirror_mode = 0;
    vg_lite_uint32_t premultiply_dst = 0;
    vg_lite_uint32_t rgb_alphadiv = 0;
    vg_lite_uint32_t read_dest = 0;
    vg_lite_uint32_t dst_format = 0;
    vg_lite_uint32_t rt_changed = 0;

    VG_LITE_CHECK_NULL_POINTER(target);

    /* Check if render target parameters are really changed. */
    if (memcmp(s_context.rtbuffer, target, sizeof(vg_lite_buffer_t))) {
        rt_changed = 1;
    }
    /* Simply return if render target, scissor, mirror, gamma, flexa states are not changed. */
    if (!rt_changed 
        && !s_context.scissor_dirty 
        && !s_context.mirror_dirty 
        && !s_context.gamma_dirty
#if gcFEATURE_VG_FLEXA
        && !s_context.flexa_dirty
#endif
#if gcFEATURE_VG_MESH_FOR_FRAME
        && !s_context.mesh_mode_dirty
        && !(s_context.mesh_mode && s_context.mesh_dirty)
#endif
        ) {
        return VG_LITE_SUCCESS;
    }

#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_RETURN_ERROR(feature_check_target_yuv_output_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_planar_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_render_target_mesh_for_frame(
        target, s_context.mesh_mode, s_context.mirror_dirty, s_context.mesh_height));
    VG_LITE_RETURN_ERROR(feature_check_mesh_render_target(
        s_context.rtbuffer, target, s_context.mesh_mode, s_context.mesh_dirty, s_context.mesh_height, s_context.mesh_count));
    VG_LITE_RETURN_ERROR(feature_check_flexa_render_target(
        s_context.rtbuffer, target, s_context.sync_mode, s_context.flexa_dirty));
    VG_LITE_RETURN_ERROR(dstbuf_align_check(target));
    VG_LITE_RETURN_ERROR(feature_check_compress(target->format, target->compress_mode, target->tiled, target->width, target->height));
#endif /* gcFEATURE_VG_ERROR_CHECK */

    /* Flush previous render target before setting the new render target. */
    VG_LITE_RETURN_ERROR(feature_check_hw_stall_scissor_target());

    /* Program render target states */
    {
        if (((target->format >= VG_LITE_YUY2) && (target->format <= VG_LITE_AYUY2)) ||
            ((target->format >= VG_LITE_YUY2_TILED) && (target->format <= VG_LITE_AYUY2_TILED)))
        {
            yuv2rgb = convert_yuv2rgb(target->yuv.yuv2rgb);
            uv_swiz = convert_uv_swizzle(target->yuv.swizzle);
        }

#if gcFEATURE_VG_NEW_FACTOR
        yuv2rgb = convert_yuv2rgb(target->yuv.yuv2rgb);
#endif

        mirror_mode = chip_get_mirror_mode(s_context.mirror_orient);
        compress_mode = ((vg_lite_uint32_t)target->compress_mode) << 25;

        if (target->premultiplied || target->apply_premult) {
            premultiply_dst = 0x00000100;
        }
        if (target->svg_blend_flag)
            premultiply_dst = 0x0000100;

#if gcFEATURE_VG_HW_PREMULTIPLY
        rgb_alphadiv = 0x00000200;
#endif
        read_dest = feature_set_reg_filed_for_read_destination(target);
        dst_format = convert_target_format(target->format);
        if (dst_format == 0xFF) {
            printf("Target format: 0x%x is not supported.\n", target->format);
            return VG_LITE_NOT_SUPPORT;
        }

        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A10,
            dst_format | read_dest | uv_swiz | yuv2rgb | compress_mode | mirror_mode | s_context.gamma_value | premultiply_dst | rgb_alphadiv));

        s_context.mirror_dirty = 0;
        s_context.gamma_dirty = 0;

        /* Set scissor rectangle on the render target */
        if (s_context.scissor_set) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A13, (MIN(s_context.scissor[2], target->width)) | (((MIN(s_context.scissor[3], target->height)) << 16))));
        }
        else {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A13, target->width | (target->height << 16)));
        }
        s_context.scissor_dirty = 0;
#if gcFEATURE_VG_MESH_FOR_FRAME
        if (s_context.mesh_mode) {
            s_context.mesh_dirty = 0;
        }
#endif

        tile_setting = (target->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0;

        /* 24bit format stride configured to 4bpp. */
        if (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658) {
            vg_lite_uint32_t stride = target->stride / 3 * 4;
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A12, stride | tile_setting));
        }
        else {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A12, target->stride | tile_setting));
        }

#if gcFEATURE_VG_MESH_FOR_FRAME
        s_context.mesh_mode_dirty = 0;
#if gcFEATURE_VG_SIMPLE_BLT || gcFEATURE_VG_EXTERNAL_DMA_MESH
        if ((s_context.mesh_mode == VG_LITE_MESH_COPY_INTERNAL) || (s_context.mesh_mode == VG_LITE_MESH_COPY_EXTERNAL)) {
            if (target->yuv.uv_planar)
            {   /* Program uv plane address if necessary. */
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A5C, target->mesh_buffer->yuv.uv_planar));
            }
            if (target->yuv.alpha_planar) {
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A5D, target->mesh_buffer->yuv.alpha_planar));
            }
#if _WIN32
            /* The simulation of DMA in CModel requires additional register configuration */
            if (s_context.mesh_mode == VG_LITE_MESH_COPY_EXTERNAL) {
#if CID == 0x456
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x169E, target->address));
#else
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x161E, target->address));
#endif
                if (target->mesh_buffer->yuv.alpha_planar) {
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x114B, target->yuv.alpha_planar));
                }
            }
#endif
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A11, target->mesh_buffer->address));
            /* Base_address == target_address */
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFF, target->mesh_buffer->address));
        }
        else
#endif
#endif
        {
            if (target->yuv.uv_planar)
            {   /* Program uv plane address if necessary. */
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A5C, target->yuv.uv_planar));
            }
            if (target->yuv.alpha_planar) {
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A5D, target->yuv.alpha_planar));
            }

            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A11, target->address));
            /* Base_address == target_address */
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFF, target->address));
        }
    }

#if gcFEATURE_VG_FLEXA
    s_context.flexa_dirty = 0;
#endif

    VG_LITE_TRACE_API("    set_render_target %p (%d, %d)\n", target, target->width, target->height);

    /* Copy the current render target parameters into s_context.rtbuffer */
    if (rt_changed) {
        memcpy(s_context.rtbuffer, target, sizeof(vg_lite_buffer_t));
    }

    return error;
}

vg_lite_void calculate_step_value(vg_lite_filter_t filter, vg_lite_matrix_t *inverse_matrix, vg_lite_int32_t width, vg_lite_int32_t height,
                                  vg_lite_float_t x_step[3], vg_lite_float_t y_step[3], vg_lite_float_t c_step[3])
{
#if gcFEATURE_VG_MATH_PRECISION_FIX_DISABLE
    if (filter == VG_LITE_FILTER_LINEAR)
    {
        /* Compute interpolation steps. */
        x_step[0] = (inverse_matrix->m[0][0] - 0.5f * inverse_matrix->m[2][0]);
        x_step[1] = inverse_matrix->m[1][0];
        x_step[2] = inverse_matrix->m[2][0];
        y_step[0] = (inverse_matrix->m[0][1] - 0.5f * inverse_matrix->m[2][1]);
        y_step[1] = inverse_matrix->m[1][1];
        y_step[2] = inverse_matrix->m[2][1];
        c_step[0] = (0.5f * (inverse_matrix->m[0][0] + inverse_matrix->m[0][1]) - 0.25f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[0][2] - 0.5f * inverse_matrix->m[2][2]);
        c_step[1] = (0.5f * (inverse_matrix->m[1][0] + inverse_matrix->m[1][1]) + inverse_matrix->m[1][2]);
        c_step[2] = 0.5f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[2][2];
    }
    else if (filter == VG_LITE_FILTER_BI_LINEAR)
    {
        /* Shift the linear sampling points to center of pixels to avoid pixel offset issue */
        x_step[0] = (inverse_matrix->m[0][0] - 0.5f * inverse_matrix->m[2][0]);
        x_step[1] = (inverse_matrix->m[1][0] - 0.5f * inverse_matrix->m[2][0]);
        x_step[2] = inverse_matrix->m[2][0];
        y_step[0] = (inverse_matrix->m[0][1] - 0.5f * inverse_matrix->m[2][1]);
        y_step[1] = (inverse_matrix->m[1][1] - 0.5f * inverse_matrix->m[2][1]);
        y_step[2] = inverse_matrix->m[2][1];
        c_step[0] = (0.5f * (inverse_matrix->m[0][0] + inverse_matrix->m[0][1]) - 0.25f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[0][2] - 0.5f * inverse_matrix->m[2][2]);
        c_step[1] = (0.5f * (inverse_matrix->m[1][0] + inverse_matrix->m[1][1]) - 0.25f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[1][2] - 0.5f * inverse_matrix->m[2][2]);
        c_step[2] = 0.5f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[2][2];
    }
    else
    {
        /* Compute interpolation steps. */
        x_step[0] = inverse_matrix->m[0][0];
        x_step[1] = inverse_matrix->m[1][0];
        x_step[2] = inverse_matrix->m[2][0];
        y_step[0] = inverse_matrix->m[0][1];
        y_step[1] = inverse_matrix->m[1][1];
        y_step[2] = inverse_matrix->m[2][1];
        c_step[0] = (0.5f * (inverse_matrix->m[0][0] + inverse_matrix->m[0][1]) + inverse_matrix->m[0][2]);
        c_step[1] = (0.5f * (inverse_matrix->m[1][0] + inverse_matrix->m[1][1]) + inverse_matrix->m[1][2]);
        c_step[2] = 0.5f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[2][2];

        // For FL32 rounding trick
        vg_lite_uint32_t datax[2], datay[2], datac[2];
        for (vg_lite_int32_t idx = 0; idx < 2; idx++)
        {
            datax[idx] = *(vg_lite_uint32_t*)((vg_lite_pointer)&x_step[idx]);
            datay[idx] = *(vg_lite_uint32_t*)((vg_lite_pointer)&y_step[idx]);
            datac[idx] = *(vg_lite_uint32_t*)((vg_lite_pointer)&c_step[idx]);
        }
        for (vg_lite_int32_t i = 0; i < 2; i++)
        {
            vg_lite_int32_t aSign = (datax[i] & 0x80000000) >> 31;
            vg_lite_int32_t bSign = (datay[i] & 0x80000000) >> 31;
            vg_lite_int32_t cSign = (datac[i] & 0x80000000) >> 31;
            vg_lite_int32_t aIn = (datax[i] & 0x20) >> 5;
            vg_lite_int32_t bIn = (datay[i] & 0x20) >> 5;
            if ((aSign == 0) && (bSign == 0) && (aIn == bIn))
            {
                vg_lite_int32_t cIn = (aSign ^ cSign) ^ ((~aIn) & 0x1);
                if (cIn == 0)
                {
                    datac[i] &= 0xFFFFFFDF;
                }
                else
                {
                    datac[i] |= 0x00000020;
                }
                c_step[i] = *(vg_lite_float_t*)((vg_lite_pointer)&datac[i]);
            }
        }
    }
#else
    if (filter == VG_LITE_FILTER_LINEAR)
    {
        /* Compute interpolation steps. */
        x_step[0] = (inverse_matrix->m[0][0] - 0.5f * inverse_matrix->m[2][0]) / width;
        x_step[1] = inverse_matrix->m[1][0] / height;
        x_step[2] = inverse_matrix->m[2][0];
        y_step[0] = (inverse_matrix->m[0][1] - 0.5f * inverse_matrix->m[2][1]) / width;
        y_step[1] = inverse_matrix->m[1][1] / height;
        y_step[2] = inverse_matrix->m[2][1];
        c_step[0] = (0.5f * (inverse_matrix->m[0][0] + inverse_matrix->m[0][1]) - 0.25f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[0][2] - 0.5f * inverse_matrix->m[2][2]) / width;
        c_step[1] = (0.5f * (inverse_matrix->m[1][0] + inverse_matrix->m[1][1]) + inverse_matrix->m[1][2]) / height;
        c_step[2] = 0.5f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[2][2];
    }
    else if (filter == VG_LITE_FILTER_BI_LINEAR)
    {
        /* Shift the linear sampling points to center of pixels to avoid pixel offset issue */
        x_step[0] = (inverse_matrix->m[0][0] - 0.5f * inverse_matrix->m[2][0]) / width;
        x_step[1] = (inverse_matrix->m[1][0] - 0.5f * inverse_matrix->m[2][0]) / height;
        x_step[2] = inverse_matrix->m[2][0];
        y_step[0] = (inverse_matrix->m[0][1] - 0.5f * inverse_matrix->m[2][1]) / width;
        y_step[1] = (inverse_matrix->m[1][1] - 0.5f * inverse_matrix->m[2][1]) / height;
        y_step[2] = inverse_matrix->m[2][1];
        c_step[0] = (0.5f * (inverse_matrix->m[0][0] + inverse_matrix->m[0][1]) - 0.25f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[0][2] - 0.5f * inverse_matrix->m[2][2]) / width;
        c_step[1] = (0.5f * (inverse_matrix->m[1][0] + inverse_matrix->m[1][1]) - 0.25f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[1][2] - 0.5f * inverse_matrix->m[2][2]) / height;
        c_step[2] = 0.5f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[2][2];
    }
    else
    {
        /* Compute interpolation steps. */
        x_step[0] = inverse_matrix->m[0][0] / width;
        x_step[1] = inverse_matrix->m[1][0] / height;
        x_step[2] = inverse_matrix->m[2][0];
        y_step[0] = inverse_matrix->m[0][1] / width;
        y_step[1] = inverse_matrix->m[1][1] / height;
        y_step[2] = inverse_matrix->m[2][1];
        c_step[0] = (0.5f * (inverse_matrix->m[0][0] + inverse_matrix->m[0][1]) + inverse_matrix->m[0][2]) / width;
        c_step[1] = (0.5f * (inverse_matrix->m[1][0] + inverse_matrix->m[1][1]) + inverse_matrix->m[1][2]) / height;
        c_step[2] = 0.5f * (inverse_matrix->m[2][0] + inverse_matrix->m[2][1]) + inverse_matrix->m[2][2];
    }
#endif
}

#if gcFEATURE_VG_MESH_FOR_FRAME
static vg_lite_error_t mesh_rt_reset(vg_lite_buffer_t* target) {
    vg_lite_error_t error = VG_LITE_SUCCESS;
    if (s_context.mesh_dirty && s_context.mesh_mode) {
        VG_LITE_RETURN_ERROR(set_render_target(target));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A39, s_context.tessbuf.tess_x_y));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3A, s_context.tessbuf.tess_w_h));
        s_context.mesh_dirty = 0;
    }

    return error;
}

static vg_lite_error_t check_mesh_params(vg_lite_mesh_mode_t mesh_mode,
                                         vg_lite_uint32_t mesh_height,
                                         vg_lite_uint32_t mesh_count)
{
#if !gcFEATURE_VG_SIMPLE_BLT
    if (mesh_mode == VG_LITE_MESH_COPY_INTERNAL)
        return VG_LITE_NOT_SUPPORT;
#endif
#if !gcFEATURE_VG_EXTERNAL_DMA_MESH
    if (mesh_mode == VG_LITE_MESH_COPY_EXTERNAL)
        return VG_LITE_NOT_SUPPORT;
#endif
    if (mesh_count <= 0 || mesh_count > 64) 
        return VG_LITE_INVALID_ARGUMENT;
    if (mesh_height <= 0)
        return VG_LITE_INVALID_ARGUMENT;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_set_target_mesh(vg_lite_mesh_mode_t mesh_mode,
                                        vg_lite_uint32_t mesh_height,
                                        vg_lite_uint32_t mesh_count)
{
    DUMP_API_CALL(vg_lite_set_target_mesh);
    VG_LITE_TRACE_API("vg_lite_set_target_mesh\n");

    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_mesh_info_t mesh_data;
    vg_lite_uint32_t n = 0;
    static vg_lite_mesh_mode_t last_mesh_mode = VG_LITE_MESH_DISABLE;

    VG_LITE_RETURN_ERROR(check_mesh_params(mesh_mode, mesh_height, mesh_count));
    VG_LITE_RETURN_ERROR(vg_lite_finish());

    mesh_data.mesh_control = 0;
    switch (mesh_mode)
    {
    case VG_LITE_MESH_DISABLE:
        /*disable*/
        s_context.mesh_mode = VG_LITE_MESH_DISABLE;
        s_context.mesh_height = 0;
        s_context.mesh_count = 0;
        break;

    case VG_LITE_MESH_COPY_EXTERNAL:
    case VG_LITE_MESH_COPY_INTERNAL:
        mesh_data.mesh_control |= mesh_count << 24;
    case VG_LITE_MESH_FRAME:
        n = mesh_height / 16 - 1;
        if (mesh_height % 16) {
            n++;
            printf("Mesh height not aligned for 16p. Set mesh height to %d\n", (n + 1) * 16);
        }
        s_context.mesh_mode = mesh_mode;
        s_context.mesh_height = (n + 1) * 16;
        s_context.mesh_count = mesh_count;
        mesh_data.mesh_control |= (n << 15);
        mesh_data.mesh_control |= (mesh_mode << 1);

        break;
    default:
        return VG_LITE_INVALID_ARGUMENT;
    }

    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_SET_TARGET_MESH_CONTROL, &mesh_data));
#if DUMP_CAPTURE
    vg_lite_uint32_t mesh_c;
    mesh_c = mesh_data.mesh_control;
    vglitemDUMP_BUFFER("AHB", 0x520, &mesh_c, 0, 4);
#endif

    if (last_mesh_mode != mesh_mode) {
        s_context.mesh_mode_dirty = 1;
        last_mesh_mode = mesh_mode;
    }

    return error;
}

#if gcFEATURE_VG_SIMPLE_BLT
/* Push a "stall" command into the current command buffer. */
static vg_lite_error_t push_mesh_stall(vg_lite_context_t* context, vg_lite_uint32_t module)
{
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x1583, 0x1F01));

    VG_LITE_RETURN_ERROR(handle_cmd_overflow(CMDBUF_OFFSET(*context) + 8, context));

    ((vg_lite_uint32_t*)(CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_STALL(module);
    ((vg_lite_uint32_t*)(CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
    if (context->fb_command_offset + 8 > CACHE_COMMAND_BUFFER_SIZE) {
        printf("The size of cache is %d bytes, exceeding the capacity of cache buffer(%d bytes)!\n", context->fb_command_offset + 8, CACHE_COMMAND_BUFFER_SIZE);
        printf("Please clear cache or increase cache buffer size!\n");
        return VG_LITE_OUT_OF_MEMORY;
    }

    if (context->backup_fb_command_flag) {
        ((vg_lite_uint32_t*)(context->fb_command_buffer + context->fb_command_offset))[0] = VG_LITE_STALL(module);
        ((vg_lite_uint32_t*)(context->fb_command_buffer + context->fb_command_offset))[1] = 0;

        context->fb_command_offset += 8;
    }
#endif

#if DUMP_COMMAND
    if (strncmp(filename, "Commandbuffer", 13)) {
        sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
    }

    fp = fopen(filename, "a");

    if (fp == NULL)
        printf("error!\n");

    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
        ((vg_lite_uint32_t*)(CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0], 0);

    fclose(fp);
    fp = NULL;
#endif

    CMDBUF_OFFSET(*context) += 8;

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t vg_lite_target_mesh_copy(vg_lite_buffer_t *target) {
    DUMP_API_CALL(vg_lite_target_mesh_copy);

    vg_lite_error_t error;
#if CID == 0x456
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x169D, target->mesh_buffer->address));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x169E, target->address));
    if (target->mesh_buffer->yuv.alpha_planar && target->yuv.alpha_planar) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x114A, target->mesh_buffer->yuv.alpha_planar));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x114B, target->yuv.alpha_planar));
    }
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x169F, target->height * target->stride));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x1680, 0x3));
    VG_LITE_RETURN_ERROR(push_mesh_stall(&s_context, (0x1F | (1 << 8))));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00011001));
#else
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x161D, target->mesh_buffer->address));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x161E, target->address));
    if (target->mesh_buffer->yuv.alpha_planar && target->yuv.alpha_planar) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x114A, target->mesh_buffer->yuv.alpha_planar));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x114B, target->yuv.alpha_planar));
    }
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x161F, target->height * target->stride));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x1600, 0x3));
    VG_LITE_RETURN_ERROR(push_mesh_stall(&s_context, (0x1F | (1 << 8))));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00011001));
#endif
    return VG_LITE_SUCCESS;
}
#endif
#else
vg_lite_error_t vg_lite_set_target_mesh(vg_lite_mesh_mode_t mesh_mode,
                                        vg_lite_uint32_t mesh_height,
                                        vg_lite_uint32_t mesh_count)
{
    return VG_LITE_NOT_SUPPORT;
}

#endif /* gcFEATURE_VG_MESH_FOR_FRAME */

/*************** VGLite API Functions ***********************************************/

vg_lite_error_t vg_lite_clear(vg_lite_buffer_t * target,
                              vg_lite_rectangle_t * rect,
                              vg_lite_color_t color)
{
    DUMP_API_CALL(vg_lite_clear, target, rect, color);
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_CLEAR_API);
    VG_LITE_TRACE_API("vg_lite_clear %p %p 0x%08X\n", target, rect, color);

    vg_lite_error_t error;
    vg_lite_point_t point_min, point_max;
    vg_lite_int32_t  left, top, right, bottom;
    vg_lite_uint32_t color32;
    vg_lite_uint32_t tile_setting = 0;
    vg_lite_uint32_t stripe_mode = 0;
    vg_lite_uint32_t in_premult = 0;
#if gcFEATURE_VG_NEW_FACTOR
    vg_factor_config_t factor_config;
    factor_config.factor_src_alpha = 0x0;
    factor_config.factor_src_color = 0x0;
    factor_config.factor_dst_alpha = 0x0;
    factor_config.factor_dst_color = 0x0;
    factor_config.final_equation_opcode = 0x0;
    factor_config.dstchannelmode = 0x0;
    factor_config.srcchannelmode = 0x0;
#endif
#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((target->format >= VG_LITE_ABGR8565_PLANAR) && (target->format <= VG_LITE_RGBA5658_PLANAR))
    {
        if (target->sw24bit_buffer)
        {
            target->sw24bit_buffer->format = convert_24bit_format(target->format);
            target = target->sw24bit_buffer;
        }
    }
#endif

    if (rect) VG_LITE_TRACE_API("    Rect(%d, %d, %d, %d)\n", rect->x, rect->y, rect->width, rect->height);

#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_RETURN_ERROR(chip_check_target_format(target->format));
#endif

#if gcFEATURE_VG_GAMMA
    set_gamma_dest_only(target, VGL_FALSE);
#endif

    if (target->premultiplied) {
        in_premult = 0x00000000;
        target->apply_premult = 0;
    }
    else {
        in_premult = 0x10000000;
        target->apply_premult = 1;
    }
    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }

    /* Get rectangle. */
    if (rect) {
        point_min.x = rect->x;
        point_min.y = rect->y;
        point_max.x = rect->x + rect->width;
        point_max.y = rect->y + rect->height;
    }
    else {
        point_min.x = 0;
        point_min.y = 0;
        point_max.x = s_context.rtbuffer->width;
        point_max.y = s_context.rtbuffer->height;
    }

    /* Clip to target. */
    left = 0;
    top = 0;
    right = target->width;
    bottom = target->height;

    if (s_context.scissor_set && !target->scissor_buffer) {
        left = MAX(s_context.scissor[0], left);
        top = MAX(s_context.scissor[1], top);
        right = MIN(s_context.scissor[2], right);
        bottom = MIN(s_context.scissor[3], bottom);
    }

    if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer) {
        left = MAX(s_context.scissor_layer_range[0], left);
        top = MAX(s_context.scissor_layer_range[1], top);
        right = MIN(s_context.scissor_layer_range[2], right);
        bottom = MIN(s_context.scissor_layer_range[3], bottom);
    }

    point_min.x = MAX(point_min.x, left);
    point_min.y = MAX(point_min.y, top);
    point_max.x = MIN(point_max.x, right);
    point_max.y = MIN(point_max.y, bottom);

    /* No need to draw. */
    if ((point_max.x <= point_min.x) || (point_max.y <= point_min.y)) {
        return VG_LITE_SUCCESS;
    }

    /* Get converted color when target is in L8 format. */
    color32 = (target->format == VG_LITE_L8 || target->format == VG_LITE_A8L8) ? rgb_to_l(color, target) : color;

#if gcFEATURE_VG_RECTANGLE_TILED_OUT
    if (target->tiled == VG_LITE_TILED) {
        tile_setting = 0x40;
        stripe_mode = 0x20000000;
    }
#endif

    {
        /* Setup the command buffer. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color32));

        /* Clear operation is not affected by color transformation and pixel matrix.
         * So PE clear and push_rectangle() clear have the same clear result color.
         */
#if gcFEATURE_VG_NEW_FACTOR
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF6, factor_config.srcchannelmode | (factor_config.dstchannelmode << 8)));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF8, factor_config.factor_src_alpha));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF9, factor_config.factor_src_color));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFA, factor_config.factor_dst_alpha));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFB, factor_config.factor_dst_color));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF7, factor_config.final_equation_opcode));
#endif

#if gcFEATURE_VG_PE_CLEAR
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A39, 0));
        s_context.tessbuf.tess_x_y = 0;
        if ((!rect && (point_min.x == 0 && point_min.y == 0 && (point_max.x - point_min.x) == target->width)) && !s_context.scissor_enable && 
             !s_context.scissor_set && !s_context.enable_mask && !peclear_align_check(target, point_max.y - point_min.y))
        {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, in_premult | 0x00000004 | tile_setting | s_context.scissor_enable | stripe_mode));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
            VG_LITE_RETURN_ERROR(push_pe_clear(&s_context, target->stride * (point_max.y - point_min.y)));
        }
        else
#endif
        {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, in_premult | 0x00000001 | tile_setting | s_context.scissor_enable | stripe_mode));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
            VG_LITE_RETURN_ERROR(push_rectangle(&s_context, point_min.x, point_min.y, point_max.x - point_min.x, point_max.y - point_min.y));
        }

        /* flush VGPE after clear */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000011));
    }
#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((target->format >= VG_LITE_ABGR8565) && (target->format <= VG_LITE_RGBA5658))
    {
        if (target->sw24bit_planar_buffer)
            target = target->sw24bit_planar_buffer;
    }
#endif

    /* Success. */
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_blit2(vg_lite_buffer_t* target,
                            vg_lite_buffer_t* source0,
                            vg_lite_buffer_t* source1,
                            vg_lite_matrix_t* matrix0,
                            vg_lite_matrix_t* matrix1,
                            vg_lite_blend_t blend,
                            vg_lite_filter_t filter)
{
    DUMP_API_CALL(vg_lite_blit2, target, source0, source1, matrix0, matrix1, blend, filter);
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_BLIT2_API);
    VG_LITE_TRACE_API("vg_lite_blit2 %p %p %p %p %p %d %d\n", target, source0, source1, matrix0, matrix1, blend, filter);

#if gcFEATURE_VG_DOUBLE_IMAGE && gcFEATURE_VG_IM_INPUT
    vg_lite_error_t error;
    vg_lite_point_t  point_min0, point_max0, point_min1, point_max1, temp;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_float_t x_step[2][3];
    vg_lite_float_t y_step[2][3];
    vg_lite_float_t c_step[2][3];
    vg_lite_float_t ratio0 = 1;
    vg_lite_float_t ratio1 = 1;
    vg_lite_uint32_t imageMode;
    vg_lite_uint32_t blend_mode;
    vg_lite_uint32_t filter_mode = 0;
    vg_lite_int32_t stride0;
    vg_lite_int32_t stride1;
    vg_lite_uint32_t rotation = 0;
    vg_lite_uint32_t conversion = 0;
    vg_lite_uint32_t tiled0, tiled1;
    vg_lite_int32_t left, right, bottom, top;

#if gcFEATURE_VG_FLEXA
    if (s_context.sync_mode)
    {
        printf("When Flexa is enabled vg_lite_blit2 is not support.\n");
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(source0->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(source1->format));
    VG_LITE_RETURN_ERROR(feature_check_source_packed_yuy_input(source0->format));
    VG_LITE_RETURN_ERROR(feature_check_source_packed_yuy_input(source1->format));
    VG_LITE_RETURN_ERROR(feature_check_source_planar_yuv_input(source0->format));
    VG_LITE_RETURN_ERROR(feature_check_source_planar_yuv_input(source1->format));
    VG_LITE_RETURN_ERROR(feature_check_source_planar_nv24_input(source0->format));
    VG_LITE_RETURN_ERROR(feature_check_source_planar_nv24_input(source1->format));
    VG_LITE_RETURN_ERROR(feature_check_source_ayuv_input(source0->format));
    VG_LITE_RETURN_ERROR(feature_check_source_ayuv_input(source1->format));
    VG_LITE_RETURN_ERROR(feature_check_source_yuv_tiled_input(source0->format));
    VG_LITE_RETURN_ERROR(feature_check_source_yuv_tiled_input(source1->format));
    VG_LITE_RETURN_ERROR(feature_check_new_blend_mode(blend));
    VG_LITE_RETURN_ERROR(feature_check_lvgl_recolor_image_mode(source0->image_mode));
    VG_LITE_RETURN_ERROR(feature_check_lvgl_recolor_image_mode(source1->image_mode));
    VG_LITE_RETURN_ERROR(feature_check_a124_a8l8_target_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_a124_a8l8_source_format(source0->format));
    VG_LITE_RETURN_ERROR(feature_check_a124_a8l8_source_format(source1->format));
#endif /* gcFEATURE_VG_ERROR_CHECK */

    if (!matrix0) {
        matrix0 = &identity_mtx;
    }
    if (!matrix1) {
        matrix1 = &identity_mtx;
    }

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }

    /* Check if the specified matrix has rotation or perspective. */
    if (  (matrix0->m[0][1] != 0.0f)
       || (matrix0->m[1][0] != 0.0f)
       || (matrix0->m[2][0] != 0.0f)
       || (matrix0->m[2][1] != 0.0f)
       || (matrix0->m[2][2] != 1.0f)
       ) {
        /* Mark that we have rotation. */
        rotation = 0x8000;
    }

    conversion = feature_a124_a8l8_l8_conversion(target->format, source0->format);

    /* Calculate transformation for Image0 (Paint) & Image1 (Image). */
    /* Image1. */
    /* Transform image (0,0) to screen. */
    if (!transform(&temp, 0.0f, 0.0f, matrix0))
        return VG_LITE_INVALID_ARGUMENT;

    /* Set initial point. */
    point_min0 = temp;
    point_max0 = temp;

    /* Transform image (0,height) to screen. */
    if (!transform(&temp, 0.0f, (vg_lite_float_t)source0->height, matrix0))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min0.x) point_min0.x = temp.x;
    if (temp.y < point_min0.y) point_min0.y = temp.y;
    if (temp.x > point_max0.x) point_max0.x = temp.x;
    if (temp.y > point_max0.y) point_max0.y = temp.y;

    /* Transform image (width,height) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source0->width, (vg_lite_float_t)source0->height, matrix0))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min0.x) point_min0.x = temp.x;
    if (temp.y < point_min0.y) point_min0.y = temp.y;
    if (temp.x > point_max0.x) point_max0.x = temp.x;
    if (temp.y > point_max0.y) point_max0.y = temp.y;

    /* Transform image (width,0) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source0->width, 0.0f, matrix0))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min0.x) point_min0.x = temp.x;
    if (temp.y < point_min0.y) point_min0.y = temp.y;
    if (temp.x > point_max0.x) point_max0.x = temp.x;
    if (temp.y > point_max0.y) point_max0.y = temp.y;

    /* Clip to target. */
    if (s_context.scissor_set) {
        left = s_context.scissor[0];
        top = s_context.scissor[1];
        right = s_context.scissor[2];
        bottom = s_context.scissor[3];
    }
    else {
        left = top = 0;
        right = target->width;
        bottom = target->height;
    }

    point_min0.x = MAX(point_min0.x, left);
    point_min0.y = MAX(point_min0.y, top);
    point_max0.x = MIN(point_max0.x, right);
    point_max0.y = MIN(point_max0.y, bottom);

    if ((point_max0.x - point_min0.x) <= 0 || (point_max0.y - point_min0.y) <= 0)
        return VG_LITE_SUCCESS;
    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix0))
        return VG_LITE_SUCCESS;

    /* Compute interpolation steps for image1 (Image). */
    calculate_step_value(filter, &inverse_matrix, source0->width, source0->height, &x_step[1][0], &y_step[1][0], &c_step[1][0]);

    /* Image0 (Paint, as background ). */
    /* Transform image (0,0) to screen. */
    if (!transform(&temp, 0.0f, 0.0f, matrix1))
        return VG_LITE_INVALID_ARGUMENT;

    /* Set initial point. */
    point_min1 = temp;
    point_max1 = temp;

    /* Transform image (0,height) to screen. */
    if (!transform(&temp, 0.0f, (vg_lite_float_t)source1->height, matrix1))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min1.x) point_min1.x = temp.x;
    if (temp.y < point_min1.y) point_min1.y = temp.y;
    if (temp.x > point_max1.x) point_max1.x = temp.x;
    if (temp.y > point_max1.y) point_max1.y = temp.y;

    /* Transform image (width,height) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source1->width, (vg_lite_float_t)source1->height, matrix1))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min1.x) point_min1.x = temp.x;
    if (temp.y < point_min1.y) point_min1.y = temp.y;
    if (temp.x > point_max1.x) point_max1.x = temp.x;
    if (temp.y > point_max1.y) point_max1.y = temp.y;

    /* Transform image (width,0) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source1->width, 0.0f, matrix1))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min1.x) point_min1.x = temp.x;
    if (temp.y < point_min1.y) point_min1.y = temp.y;
    if (temp.x > point_max1.x) point_max1.x = temp.x;
    if (temp.y > point_max1.y) point_max1.y = temp.y;

    /* Clip to target. */
    if (s_context.scissor_set) {
        left = s_context.scissor[0];
        top = s_context.scissor[1];
        right = s_context.scissor[2];
        bottom = s_context.scissor[3];
    }
    else {
        left = top = 0;
        right = target->width;
        bottom = target->height;
    }

    point_min1.x = MAX(point_min1.x, left);
    point_min1.y = MAX(point_min1.y, top);
    point_max1.x = MIN(point_max1.x, right);
    point_max1.y = MIN(point_max1.y, bottom);

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix1))
        return VG_LITE_SUCCESS;

    /* Compute interpolation steps for image1 (Image). */
    calculate_step_value(filter, &inverse_matrix, source1->width, source1->height, &x_step[0][0], &y_step[0][0], &c_step[0][0]);

    /* DOUBLE_IMAGE mode. */
    imageMode = 0x5000;
    blend_mode = convert_blend(blend);
    tiled0 = (source0->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0;
    tiled1 = (source1->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0;

    switch (filter) {
    case VG_LITE_FILTER_POINT:
        filter_mode = 0;
        break;

    case VG_LITE_FILTER_LINEAR:
        filter_mode = 0x10000;
        break;

    case VG_LITE_FILTER_BI_LINEAR:
        filter_mode = 0x20000;
        break;

    case VG_LITE_FILTER_GAUSSIAN:
        filter_mode = 0x30000;
        break;
    }

    /* Setup the command buffer. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x10000001 | imageMode | blend_mode | rotation | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));
    /* Program image1. */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A18, (vg_lite_void *) &c_step[1][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A19, (vg_lite_void *) &c_step[1][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1A, (vg_lite_void *) &c_step[1][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1C, (vg_lite_void *) &x_step[1][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1D, (vg_lite_void *) &x_step[1][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1E, (vg_lite_void *) &x_step[1][2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1F, 0x00000001));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A20, (vg_lite_void *) &y_step[1][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A21, (vg_lite_void *) &y_step[1][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A22, (vg_lite_void *) &y_step[1][2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A25, convert_source_format(source0->format) | filter_mode | conversion));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A27, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source0->address));

    if (source0->yuv.uv_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A51, source0->yuv.uv_planar));
    }
    if (source0->yuv.v_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source0->yuv.v_planar));
    }

    if (source0->yuv.alpha_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source0->yuv.alpha_planar));
    }
    /* 24bit format stride configured to 4bpp. */
    if (source0->format >= VG_LITE_RGB888 && source0->format <= VG_LITE_RGBA5658) {
        stride0 = source0->stride / 3 * 4;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, stride0 | tiled0));
    }
    else {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, source0->stride | tiled0));
    }
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2D, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2F, source0->width | (source0->height << 16)));

    VG_LITE_RETURN_ERROR(push_rectangle(&s_context, point_min0.x, point_min0.y, point_max0.x - point_min0.x, point_max0.y - point_min0.y));

    /* Program image0 (Paint, as background). */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A84, (vg_lite_void *) &c_step[0][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A85, (vg_lite_void *) &c_step[0][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A86, (vg_lite_void *) &c_step[0][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A7C, (vg_lite_void *) &x_step[0][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A7D, (vg_lite_void *) &x_step[0][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A7E, (vg_lite_void *) &x_step[0][2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1F, 0x00000001));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A80, (vg_lite_void *) &y_step[0][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A81, (vg_lite_void *) &y_step[0][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A82, (vg_lite_void *) &y_step[0][2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A24, convert_source_format(source1->format) | filter_mode | conversion));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A26, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A28, source1->address));
    if (source1->yuv.uv_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A50, source1->yuv.uv_planar));
    }
    if (source1->yuv.v_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A52, source1->yuv.v_planar));
    }
    if (source1->yuv.alpha_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A52, source1->yuv.alpha_planar));
    }
    /* 24bit format stride configured to 4bpp. */
    if (source1->format >= VG_LITE_RGB888 && source1->format <= VG_LITE_RGBA5658) {
        stride1 = source1->stride / 3 * 4;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2A, stride1 | tiled1));
    }
    else {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2A, source1->stride | tiled1));
    }
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2C, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2E, source1->width | (source1->height << 16)));

    VG_LITE_RETURN_ERROR(push_rectangle(&s_context, point_min1.x, point_min1.y, point_max1.x - point_min1.x, point_max1.y - point_min1.y));
    VG_LITE_RETURN_ERROR(flush_target());

#if DUMP_CAPTURE
    if (source0->compress_mode)
        ratio0 = _calc_decnano_compress_ratio(source0->format, source0->compress_mode);
    if (source1->compress_mode)
        ratio1 = _calc_decnano_compress_ratio(source1->format, source1->compress_mode);
    vglitemDUMP_BUFFER("image", (size_t)source0->address, source0->memory, 0, (source0->stride)*(source0->height)*ratio0);
    vglitemDUMP_BUFFER("image", (size_t)source1->address, source1->memory, 0, (source1->stride)*(source1->height)*ratio1);
#endif
#if DUMP_IMAGE
    dump_img(source0->memory, source0->width, source0->height, source0->format);
    dump_img(source1->memory, source1->width, source1->height, source1->format);
#endif

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_blend_func(
    vg_lite_uint32_t factor_src_alpha,
    vg_lite_uint32_t factor_src_color,
    vg_lite_uint32_t factor_dst_alpha,
    vg_lite_uint32_t factor_dst_color,
    vg_lite_uint32_t final_equation_opcode,
    vg_lite_uint32_t srcchannelmode,
    vg_lite_uint32_t dstchannelmode)
{
#if gcFEATURE_VG_NEW_FACTOR

        s_context.porter_duff_config.factor_src_alpha = factor_src_alpha;
        s_context.porter_duff_config.factor_src_color = factor_src_color;
        s_context.porter_duff_config.factor_dst_alpha = factor_dst_alpha;
        s_context.porter_duff_config.factor_dst_color = factor_dst_color;
        s_context.porter_duff_config.final_equation_opcode = final_equation_opcode;
        s_context.porter_duff_config.srcchannelmode = srcchannelmode;
        s_context.porter_duff_config.dstchannelmode = dstchannelmode;

        s_context.porter_duff_enable = 1;

#endif
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_blit(vg_lite_buffer_t* target,
                            vg_lite_buffer_t* source,
                            vg_lite_matrix_t* matrix,
                            vg_lite_blend_t blend,
                            vg_lite_color_t color,
                            vg_lite_filter_t filter)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_BLIT_API);
    DUMP_API_CALL(vg_lite_blit, target, source, matrix, blend, color, filter);
    VG_LITE_TRACE_API("vg_lite_blit %p %p %p %d 0x%08X %d\n", target, source, matrix, blend, color, filter);

#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((target->format >= VG_LITE_ABGR8565_PLANAR) && (target->format <= VG_LITE_RGBA5658_PLANAR))
    {
        if (target->sw24bit_buffer)
        {
            target->sw24bit_buffer->format = convert_24bit_format(target->format);
            target = target->sw24bit_buffer;
        }
    }
    if ((source->format >= VG_LITE_ABGR8565_PLANAR) && (source->format <= VG_LITE_RGBA5658_PLANAR))
    {
        if (source->sw24bit_buffer)
        {
            vg_lite_convert_24bitplanar_to_24bit(source, source->sw24bit_buffer);
            source = source->sw24bit_buffer;
        }
    }
#endif

#if gcFEATURE_VG_IM_INPUT
    vg_lite_error_t error;
    vg_lite_point_t point_min, point_max, temp;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_float_t x_step[3];
    vg_lite_float_t y_step[3];
    vg_lite_float_t c_step[3];
    vg_lite_uint32_t imageMode = 0;
    vg_lite_uint32_t paintType = 0;
    vg_lite_uint32_t in_premult = 0;
    vg_lite_uint32_t blend_mode;
    vg_lite_uint32_t filter_mode = 0;
    vg_lite_uint32_t transparency_mode = 0;
    vg_lite_uint32_t conversion = 0;
    vg_lite_uint32_t tiled_source;
    vg_lite_uint32_t yuv2rgb = 0;
    vg_lite_uint32_t uv_swiz = 0;
    vg_lite_uint32_t compress_mode = 0;
    vg_lite_uint32_t src_premultiply_enable = 0;
    vg_lite_uint32_t index_endian = 0;
    vg_lite_uint32_t eco_fifo = 0;
    vg_lite_uint32_t tile_setting = 0;
    vg_lite_uint32_t stripe_mode = 0;
    vg_lite_uint32_t premul_flag = 0;
    vg_lite_uint32_t prediv_flag = 0;
    vg_lite_int32_t  left, top, right, bottom;
    vg_lite_int32_t  stride;
    vg_lite_uint32_t pattern_tile = 0;
    vg_lite_porter_duff_config_t porter_duff_config;
#if gcFEATURE_VG_NEW_FACTOR
    vg_factor_config_t factor_config;
    factor_config.factor_src_alpha = 0x0;
    factor_config.factor_src_color = 0x0;
    factor_config.factor_dst_alpha = 0x3;
    factor_config.factor_dst_color = 0x5;
    factor_config.final_equation_opcode = 0x0;
    factor_config.dstchannelmode = 0x0;
    factor_config.srcchannelmode = 0x0;
    if (blend >= SVG2_BLEND_NORMAL)
        target->svg_blend_flag = 1;
    if ((blend >= VG_LITE_BLEND_NONE && blend <= VG_LITE_BLEND_SUBTRACT) || (blend >= OPENVG_BLEND_SRC && blend <= OPENVG_BLEND_ADDITIVE))
        target->yuv.yuv2rgb = VG_LITE_YUV709;
#endif
#if DUMP_CAPTURE
    vg_lite_float_t ratio = 1;
#endif
#if !gcFEATURE_VG_LVGL_SUPPORT
    vg_lite_uint8_t  lvgl_sw_blend = 0;
#endif

    vg_lite_uint8_t enable_sw_pre_opt = 0;

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    vg_lite_uint8_t* buffer_pointer;
    vg_lite_uint32_t buffer_address = 0,  buffer_scissor_address = 0, buffer_mask_address = 0, mul = 0, div = 0, required_align = 0;
    vg_lite_buffer_t new_target;
    vg_lite_point_t image_display_top_left = { 0 };
    vg_lite_point_t image_display_range = { 0 };
    vg_lite_float_point_t top_right_point, bottom_right_point, top_left_point, bottom_left_point;
    vg_lite_point_t new_target_top_left = { 0 };
    vg_lite_float_point4_t image_position_on_new_target = { 0 };
    vg_lite_float_point4_t image_display_position_on_new_target = { 0 };
    vg_lite_point_t image_display_top_left_on_new_target = { 0 };
    vg_lite_point_t image_display_range_on_new_target = { 0 };
    vg_lite_matrix_t matrix2_temp = { 0 };
    vg_lite_matrix_t temp_matrix;

    if (matrix != NULL && target->tiled != VG_LITE_TILED && target->compress_mode == VG_LITE_DEC_DISABLE && ((target->format >= VG_LITE_RGBA8888 && target->format <= VG_LITE_BGRA5551) || target->format == VG_LITE_A8
        || target->format == VG_LITE_L8 || (target->format >= VG_LITE_RGBA2222 && target->format <= VG_LITE_XRGB8888) || (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658_PLANAR))) {
        enable_sw_pre_opt = 1;
        memcpy(&temp_matrix, matrix, sizeof(vg_lite_matrix_t));
    }
#endif /* gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW */


#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_RETURN_ERROR(feature_check_source_index_endian(source->format, source->index_endian));
    VG_LITE_RETURN_ERROR(feature_check_source_rgba8888_etc2_eac(source->format, source->width, source->height));
    VG_LITE_RETURN_ERROR(feature_check_source_rgb888_etc2_eac(source->format, source->width, source->height));
    VG_LITE_RETURN_ERROR(feature_check_source_packed_yuy_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_planar_yuv_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_planar_nv24_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_ayuv_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_yuv_tiled_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(source->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_planar_format(source->format));
    VG_LITE_RETURN_ERROR(feature_check_im_dec_input_compress(source->compress_mode));
    VG_LITE_RETURN_ERROR(feature_check_stencil_image_mode(source->image_mode));
    VG_LITE_RETURN_ERROR(feature_check_dst_screen_copy_blend(target, &blend));
    VG_LITE_RETURN_ERROR(feature_check_new_blend_mode(blend));
    VG_LITE_RETURN_ERROR(blit_check_blend_on_yuy_target(blend, target->format));
    VG_LITE_RETURN_ERROR(feature_check_lvgl_recolor_image_mode(source->image_mode));
    VG_LITE_RETURN_ERROR(feature_check_mesh_blt_sw_lvgl_blend(blend, s_context.mesh_mode));
    VG_LITE_RETURN_ERROR(chip_check_target_format(target->format));
    VG_LITE_RETURN_ERROR(chip_check_source_format(source->format));
    VG_LITE_RETURN_ERROR(feature_check_a124_a8l8_target_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_a124_a8l8_source_format(source->format));
    VG_LITE_RETURN_ERROR(srcbuf_align_check(source));
    VG_LITE_RETURN_ERROR(feature_check_compress(source->format, source->compress_mode, source->tiled, source->width, source->height));
#endif /* gcFEATURE_VG_ERROR_CHECK */

#if !gcFEATURE_VG_LVGL_SUPPORT
    if ((blend >= VG_LITE_BLEND_ADDITIVE_LVGL && blend <= VG_LITE_BLEND_MULTIPLY_LVGL) || (blend == VG_LITE_BLEND_NORMAL_LVGL && gcFEATURE_VG_SRC_PREMULTIPLIED)) {
        if (!source->lvgl_buffer) {
            source->lvgl_buffer = (vg_lite_buffer_t *)vg_lite_os_malloc(sizeof(vg_lite_buffer_t));
            *source->lvgl_buffer = *source;
            source->lvgl_buffer->lvgl_buffer = NULL;
            vg_lite_allocate(source->lvgl_buffer);
        }
        /* Make sure render target is up to date before reading RT. */
        vg_lite_finish();
        setup_lvgl_image(target, source, source->lvgl_buffer, blend);
        blend = VG_LITE_BLEND_SRC_OVER;
        lvgl_sw_blend = 1;
    }
#endif

    if (!matrix) {
        matrix = &identity_mtx;
    }

    chip_get_source_index_endian_bits(source->format, source->index_endian, &index_endian);
#if !gcFEATURE_VG_STRIPE_MODE_DISABLE
    /* Enable fifo feature to share buffer between vg and ts to improve the rotation performance */
    eco_fifo = 1 << 7;
#endif

    transparency_mode = (source->transparency_mode == VG_LITE_IMAGE_TRANSPARENT ? 0x8000:0);

    porter_duff_config = s_context.porter_duff_config;
    /* Check if the specified matrix has rotation or perspective. */
    if (   (   (matrix->m[0][1] != 0.0f)
            || (matrix->m[1][0] != 0.0f)
            || (matrix->m[2][0] != 0.0f)
            || (matrix->m[2][1] != 0.0f)
            || (matrix->m[2][2] != 1.0f)
            )
        && ((   blend == VG_LITE_BLEND_NONE
            || blend == VG_LITE_BLEND_SRC_IN
            || blend == VG_LITE_BLEND_DST_IN
            )
            || check_porter_duff_factor_match(blend, porter_duff_config)
            )
        ) {
            feature_border_culling_special_process(&blend, &transparency_mode);
#if !gcFEATURE_VG_STRIPE_MODE_DISABLE
            stripe_mode = 1 << 29;
#endif
    }

    conversion = feature_a124_a8l8_l8_conversion(target->format, source->format);

#if gcFEATURE_VG_16PIXELS_ALIGNED
    /* Check if source specify bytes are aligned */
    error = _check_source_aligned(source->format, source->stride);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }
#endif

    s_context.filter = filter;

    /* Transform image (0,0) to screen. */
    if (!transform(&temp, 0.0f, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Set initial point. */
    point_min = temp;
    point_max = temp;
#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW    
    if(enable_sw_pre_opt) {
        if (!transform_float(&top_left_point, 0.0f, 0.0f, matrix))
            return VG_LITE_INVALID_ARGUMENT;
    }
#endif

    /* Transform image (0,height) to screen. */
    if (!transform(&temp, 0.0f, (vg_lite_float_t)source->height, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if(enable_sw_pre_opt) {
        if (!transform_float(&bottom_left_point, 0.0f, (vg_lite_float_t)source->height, matrix))
            return VG_LITE_INVALID_ARGUMENT;
    }
#endif

    /* Transform image (width,height) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source->width, (vg_lite_float_t)source->height, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if(enable_sw_pre_opt) {
        if (!transform_float(&bottom_right_point, (vg_lite_float_t)source->width, (vg_lite_float_t)source->height, matrix))
            return VG_LITE_INVALID_ARGUMENT;
    }
#endif

    /* Transform image (width,0) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source->width, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;

    s_context.filter = 0;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if(enable_sw_pre_opt) {
        if (!transform_float(&top_right_point, (vg_lite_float_t)source->width, 0.0f, matrix))
            return VG_LITE_INVALID_ARGUMENT;
    }
#endif

    /* Clip to target. */
    left = 0;
    top = 0;
    right = target->width;
    bottom = target->height;

    if (s_context.scissor_set && !target->scissor_buffer) {
        left = MAX(s_context.scissor[0], left);
        top = MAX(s_context.scissor[1], top);
        right = MIN(s_context.scissor[2], right);
        bottom = MIN(s_context.scissor[3], bottom);
    }

    if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer) {
        left = MAX(s_context.scissor_layer_range[0], left);
        top = MAX(s_context.scissor_layer_range[1], top);
        right = MIN(s_context.scissor_layer_range[2], right);
        bottom = MIN(s_context.scissor_layer_range[3], bottom);
    }

    point_min.x = MAX(point_min.x, left);
    point_min.y = MAX(point_min.y, top);
    point_max.x = MIN(point_max.x, right);
    point_max.y = MIN(point_max.y, bottom);

    /* No need to draw. */
    if ((point_max.x <= point_min.x) || (point_max.y <= point_min.y)) {
        return VG_LITE_SUCCESS;
    }

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if(enable_sw_pre_opt) {
        image_display_top_left.x = point_min.x;
        image_display_top_left.y = point_min.y;
        image_display_range.x = point_max.x - point_min.x;
        image_display_range.y = point_max.y - point_min.y;
        image_display_range_on_new_target = image_display_range;
    }
#endif

#if gcFEATURE_VG_GAMMA
    get_st_gamma_src_dest(source, target);
#endif

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_GLOBAL));
#endif
    /*blend input into context*/
    s_context.blend_mode = blend;
    in_premult = 0x00000000;

    /* Adjust premultiply setting according to openvg condition */
    src_premultiply_enable = 0x01000100;
#if gcFEATURE_VG_PIXEL_MATRIX
    if (s_context.matrix_enable == 0 && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
#else
    if (s_context.color_transform == 0 && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
#endif
    else {
        prediv_flag = 1;
    }
    if ((s_context.blend_mode >= OPENVG_BLEND_SRC && s_context.blend_mode <= OPENVG_BLEND_ADDITIVE) || source->image_mode == VG_LITE_STENCIL_MODE
        || is_lvgl_blend_mode(s_context.blend_mode)){
        premul_flag = 1;
    }
    else {
        premul_flag = 0;
    }

    if ((source->premultiplied == 0 && target->premultiplied == 0 && premul_flag == 0) ||
        (source->premultiplied == 1 && target->premultiplied == 0 && prediv_flag == 0)) {
        src_premultiply_enable = 0x01000100;
        in_premult = 0x10000000;
    }
    /* when src and dst all pre format, im pre_out set to 0 to perform data truncation to prevent data overflow */
    else if (source->premultiplied == 1 && target->premultiplied == 1 && prediv_flag == 0) {
        src_premultiply_enable = 0x00000100;
        in_premult = 0x00000000;
    }
    else if ((source->premultiplied == 0 && target->premultiplied == 1) ||
              (source->premultiplied == 0 && target->premultiplied == 0 && premul_flag == 1)) {
        src_premultiply_enable = 0x01000100;
        in_premult = 0x00000000;
    }
    else if ((source->premultiplied == 1 && target->premultiplied == 1 && prediv_flag == 1) ||
             (source->premultiplied == 1 && target->premultiplied == 0 && prediv_flag == 1)) {
        src_premultiply_enable = 0x00000100;
        in_premult = 0x00000000;
    }
    if ((source->format == VG_LITE_A1 || source->format == VG_LITE_A2 || source->format == VG_LITE_A4 || source->format == VG_LITE_A8)
        && blend >= VG_LITE_BLEND_SRC_OVER && blend <= VG_LITE_BLEND_SUBTRACT) {
        chip_adjust_src_premultiply_enable(&src_premultiply_enable);
#if gcFEATURE_VG_SRC_PREMULTIPLIED
        src_premultiply_enable = src_premultiply_enable & ~(1 << 8);
#endif
        in_premult = 0x00000000;
    }
    if (source->premultiplied == target->premultiplied && premul_flag == 0) {
        target->apply_premult = 1;
    }
    else {
        target->apply_premult = 0;
    }

#if !gcFEATURE_VG_NEW_FACTOR
#if (gcFEATURE_VG_SRC_PREMULTIPLIED == 0)
    if (blend == VG_LITE_BLEND_NORMAL_LVGL)
        in_premult = 0x00000000;
#endif
#endif

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if (enable_sw_pre_opt) {
        //update target memory address
        get_format_bytes(target->format, &mul, &div, &required_align);

        vg_lite_uint32_t temp_buffer_address = target->address;
        
        vg_lite_uint32_t temp_buffer_address1 = 0;
        if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer) 
            temp_buffer_address1 = s_context.scissor_layer->address;
        
        vg_lite_uint32_t temp_buffer_address2 = 0;
        if (s_context.enable_mask && s_context.mask_layer)
            temp_buffer_address2 = s_context.mask_layer->address;
        
        vg_lite_int32_t align_require = 4;
        vg_lite_int32_t align_require_temp = 4;
        vg_lite_int32_t align_flag = 0;

        if (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658_PLANAR)
            align_require = 64;

        vg_lite_int32_t dy = image_display_top_left.y;
        vg_lite_int32_t dx = image_display_top_left.x;

        for (; dy > 0; dy--)
        {
            for (; dx > 0; dx--)
            {
                if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer)
                    dx = dx & (~7); // dx need 8 align

                buffer_address = temp_buffer_address + dy * target->stride + dx * (mul / div);
                if ((buffer_address & (align_require - 1)) == 0) {
                    if (s_context.scissor_enable && s_context.enable_mask) {
                        if (s_context.scissor_layer->scissor_buffer && s_context.mask_layer) {
                            buffer_scissor_address = temp_buffer_address1 + dy * s_context.scissor_layer->stride + dx / 8;
                            buffer_mask_address = temp_buffer_address2 + dy * s_context.mask_layer->stride + dx;
                            
                            if (((buffer_scissor_address & (align_require_temp - 1)) == 0) && ((buffer_mask_address & (align_require_temp - 1)) == 0)) {
                                align_flag = 1;
                                break;
                            }
                        }
                    }
                    else if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer) { //then check scissor layer
                        buffer_scissor_address = temp_buffer_address1 + dy * s_context.scissor_layer->stride + dx /8;
                        
                        if ((buffer_scissor_address & (align_require_temp - 1)) == 0) {
                            align_flag = 1;
                            break;
                        }
                    }
                    else if (s_context.enable_mask && s_context.mask_layer) {
                        buffer_mask_address = temp_buffer_address2 + dy * s_context.mask_layer->stride + dx;

                        if ((buffer_mask_address & (align_require_temp - 1)) == 0) {
                            align_flag = 1;
                            break;
                        }
                    }
                    else {
                        align_flag = 1;
                        break;
                    }
                }
            }
            if (align_flag)
                break;
            dx = image_display_top_left.x;
        }

        if (!align_flag) {
            dx = dy = 0;

            if (s_context.scissor_enable)
                buffer_scissor_address = temp_buffer_address1;
            if (s_context.enable_mask)
                buffer_mask_address = temp_buffer_address2;
        }
            
        if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A16, buffer_scissor_address));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A17, s_context.scissor_layer->stride));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000100));
        }

        if (s_context.enable_mask && s_context.mask_layer) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A14, buffer_mask_address));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A15, s_context.mask_layer->stride));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000010));
        }

        //update new target coordinate
        new_target_top_left.x = dx;
        new_target_top_left.y = dy;
        image_display_top_left_on_new_target.x = image_display_top_left.x - new_target_top_left.x; image_display_top_left_on_new_target.y = image_display_top_left.y - new_target_top_left.y;

        if (align_flag) {
            //update buffer pointer address
            buffer_pointer = (vg_lite_uint8_t*)target->memory;
            buffer_pointer = buffer_pointer + (buffer_address - target->address);

            /* Calculate the transformation matrix */
            image_position_on_new_target[0].x = 0;                                  image_position_on_new_target[0].y = 0;
            image_position_on_new_target[1].x = 0;                                  image_position_on_new_target[1].y = (vg_lite_float_t)source->height;
            image_position_on_new_target[2].x = (vg_lite_float_t)source->width;     image_position_on_new_target[2].y = (vg_lite_float_t)source->height;
            image_position_on_new_target[3].x = (vg_lite_float_t)source->width;     image_position_on_new_target[3].y = 0;

            image_display_position_on_new_target[0].x = top_left_point.x - new_target_top_left.x;      image_display_position_on_new_target[0].y = top_left_point.y - new_target_top_left.y;
            image_display_position_on_new_target[1].x = bottom_left_point.x - new_target_top_left.x;   image_display_position_on_new_target[1].y = bottom_left_point.y - new_target_top_left.y;
            image_display_position_on_new_target[2].x = bottom_right_point.x - new_target_top_left.x;  image_display_position_on_new_target[2].y = bottom_right_point.y - new_target_top_left.y;
            image_display_position_on_new_target[3].x = top_right_point.x - new_target_top_left.x;     image_display_position_on_new_target[3].y = top_right_point.y - new_target_top_left.y;

            vg_lite_get_transform_matrix(image_position_on_new_target, image_display_position_on_new_target, &matrix2_temp);
            matrix2_temp.scaleX = matrix->scaleX;
            matrix2_temp.scaleY = matrix->scaleY;
            matrix2_temp.angle = matrix->angle;

            //update new_target and set  it as target
            memcpy(&new_target, target, sizeof(vg_lite_buffer_t));
            new_target.address = buffer_address;
            new_target.memory = buffer_pointer;
            target = &new_target;

            //update matrix
            memcpy(matrix, &matrix2_temp, sizeof(vg_lite_matrix_t));
        }
    }
#endif /* gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW */

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix))
        return VG_LITE_SUCCESS;

    VG_LITE_RETURN_ERROR(chip_calculate_blit_image_steps(source->width, source->height, filter, matrix,
        &inverse_matrix, x_step, y_step, c_step, enable_sw_pre_opt));

    /* Determine image mode (NORMAL, NONE , MULTIPLY or STENCIL) depending on the color. */
    switch (source->image_mode) {
        case VG_LITE_NONE_IMAGE_MODE:
            imageMode = 0x00000000;
            break;
        case VG_LITE_MULTIPLY_IMAGE_MODE:
            imageMode = 0x00002000;
            break;
        case VG_LITE_NORMAL_IMAGE_MODE:
        case VG_LITE_ZERO:
            imageMode = 0x00001000;
            break;
        case VG_LITE_STENCIL_MODE:
            imageMode = 0x00003000;
            break;
        case VG_LITE_RECOLOR_MODE:
            imageMode = 0x00006000;
            break;
    }

    switch (filter) {
    case VG_LITE_FILTER_POINT:
        filter_mode = 0;
        break;

    case VG_LITE_FILTER_LINEAR:
        filter_mode = 0x10000;
        break;

    case VG_LITE_FILTER_BI_LINEAR:
        filter_mode = 0x20000;
        break;

    case VG_LITE_FILTER_GAUSSIAN:
        filter_mode = 0x30000;
        break;
    }

    switch (source->paintType)
    {
    case VG_LITE_PAINT_COLOR:
        paintType = 0;
        break;

    case VG_LITE_PAINT_LINEAR_GRADIENT:
        paintType = 1 << 24;
        break;

    case VG_LITE_PAINT_RADIAL_GRADIENT:
        paintType = 1 << 25;
        break;

    case VG_LITE_PAINT_PATTERN:
        paintType = 1 << 24 | 1 << 25;
        break;

    default:
        break;
    }

    blend_mode = convert_blend(blend);
    tiled_source = (source->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0 ;
    VG_LITE_RETURN_ERROR(chip_set_rec_tile(target, source, matrix, &tile_setting, &stripe_mode));

    compress_mode = (vg_lite_uint32_t)source->compress_mode << 25;

#if gcFEATURE_VG_NEW_FACTOR
    config_factor_parameter(blend, porter_duff_config, &factor_config);
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF6, factor_config.srcchannelmode | (factor_config.dstchannelmode << 8)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF8, factor_config.factor_src_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF9, factor_config.factor_src_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFA, factor_config.factor_dst_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFB, factor_config.factor_dst_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF7, factor_config.final_equation_opcode));
#endif

    /* Setup the command buffer. */
    if (blend >= SVG2_BLEND_NORMAL && blend <= SVG2_BLEND_EXCLUSION)
    {
        in_premult = 0x00000000;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000001 | paintType | in_premult | imageMode | blend_mode | transparency_mode | tile_setting | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | eco_fifo | s_context.scissor_enable | stripe_mode));
    }
    else
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000001 | paintType | in_premult | imageMode | blend_mode | transparency_mode | tile_setting | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | eco_fifo | s_context.scissor_enable | stripe_mode));
    
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A18, (vg_lite_void *) &c_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A19, (vg_lite_void *) &c_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1A, (vg_lite_void *) &c_step[2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1C, (vg_lite_void *) &x_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1D, (vg_lite_void *) &x_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1E, (vg_lite_void *) &x_step[2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1F, 0x00000001));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A20, (vg_lite_void *) &y_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A21, (vg_lite_void *) &y_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A22, (vg_lite_void *) &y_step[2]));

    if (((source->format >= VG_LITE_YUY2) &&
         (source->format <= VG_LITE_AYUY2)) ||
        ((source->format >= VG_LITE_YUY2_TILED) &&
         (source->format <= VG_LITE_AYUY2_TILED))) {
            yuv2rgb = convert_yuv2rgb(source->yuv.yuv2rgb);
            uv_swiz = convert_uv_swizzle(source->yuv.swizzle);
    }

#if gcFEATURE_VG_GAUSSIAN_BLUR
    if (target->pattern_mode)
    {
        if (target->pattern_mode == VG_LITE_PATTERN_COLOR)
        {
            vg_lite_uint8_t a, r, g, b;
            pattern_tile = 0;
            a = target->bg_color >> 24;
            r = target->bg_color >> 16;
            g = target->bg_color >> 8;
            b = target->bg_color;
            target->bg_color = (a << 24) | (b << 16) | (g << 8) | r;
        }
        else if (target->pattern_mode == VG_LITE_PATTERN_PAD)
        {
            pattern_tile = 0x1000;
        }
#if gcFEATURE_VG_IM_REPEAT_REFLECT
        else if (target->pattern_mode == VG_LITE_PATTERN_REPEAT)
        {
            pattern_tile = 0x2000;
        }
        else if (target->pattern_mode == VG_LITE_PATTERN_REFLECT)
        {
            pattern_tile = 0x3000;
        }
#endif
        else
        {
            return VG_LITE_INVALID_ARGUMENT;
        }
    }
#endif

    if (blend >= SVG2_BLEND_NORMAL && blend <= SVG2_BLEND_EXCLUSION)
    {
        src_premultiply_enable = 0x01000100;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A25, convert_source_format(source->format) | filter_mode | pattern_tile | uv_swiz | yuv2rgb | conversion | compress_mode | src_premultiply_enable | index_endian));
    }
    else
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A25, convert_source_format(source->format) | filter_mode | pattern_tile | uv_swiz | yuv2rgb | conversion | compress_mode | src_premultiply_enable | index_endian));
    /* 24bit format stride configured to 4bpp. */
    if (source->format >= VG_LITE_RGB888 && source->format <= VG_LITE_RGBA5658) {
        stride = source->stride / 3 * 4;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, stride | tiled_source));
    }
    else {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, source->stride | tiled_source));
    }
    if (source->yuv.uv_planar) {
        /* Program u plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A51, source->yuv.uv_planar));
    }
    if (source->yuv.v_planar) {
        /* Program v plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source->yuv.v_planar));
    }
    if (source->yuv.alpha_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source->yuv.alpha_planar));
    }
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A27, target->bg_color));

#if !gcFEATURE_VG_LVGL_SUPPORT
    if (lvgl_sw_blend) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source->lvgl_buffer->address));
    }
    else
#endif
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source->address));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2D, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2F, source->width | (source->height << 16)));

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if (enable_sw_pre_opt) {
        VG_LITE_RETURN_ERROR(push_rectangle(&s_context, image_display_top_left_on_new_target.x, image_display_top_left_on_new_target.y, image_display_range_on_new_target.x, image_display_range_on_new_target.y));
    }
    else
#endif /* gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW */
    {
        VG_LITE_RETURN_ERROR(push_rectangle(&s_context, point_min.x, point_min.y, point_max.x - point_min.x, point_max.y - point_min.y));
    }

#if !gcFEATURE_VG_STRIPE_MODE_DISABLE
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0E02, 0x10 | (0x7 << 8)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0F00, 0x10 | (0x7 << 8)));
#endif

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_NORMAL));
#endif

     s_context.filter = 0;

     VG_LITE_BREAK_ERROR(push_state(&s_context, 0x0A1B, 0x00000011));

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if (enable_sw_pre_opt) {
        if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A17, s_context.scissor_layer->stride));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A16, s_context.scissor_layer->address));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000100));
        }

        if (s_context.enable_mask && s_context.mask_layer) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A15, s_context.mask_layer->stride));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A14, s_context.mask_layer->address));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000010));
        }

        memcpy(matrix, &temp_matrix, sizeof(vg_lite_matrix_t));
    }        
#endif /* gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW */

#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((target->format >= VG_LITE_ABGR8565) && (target->format <= VG_LITE_RGBA5658))
    {
        if (target->sw24bit_planar_buffer)
        {
            target = target->sw24bit_planar_buffer;
        }
    }
    if ((source->format >= VG_LITE_ABGR8565) && (source->format <= VG_LITE_RGBA5658))
    {
        if (source->sw24bit_planar_buffer)
        {
            source = source->sw24bit_planar_buffer;
        }
    }
#endif

#if DUMP_CAPTURE
    if (source->compress_mode)
        ratio = _calc_decnano_compress_ratio(source->format, source->compress_mode);
    vglitemDUMP_BUFFER("image", (size_t)source->address, source->memory, 0, (size_t)((source->stride)*(source->height)*ratio));
    if (source->yuv.uv_planar)
        vglitemDUMP_BUFFER(
            "uv_plane",
            (size_t)source->yuv.uv_planar, source->yuv.uv_memory,
            0,
            source->yuv.uv_stride * source->yuv.uv_height);
    if (source->yuv.v_planar)
        vglitemDUMP_BUFFER(
            "v_plane",
            (size_t)source->yuv.v_planar, source->yuv.v_memory,
            0,
            source->yuv.v_stride * source->yuv.v_height);
#endif
#if DUMP_IMAGE
    dump_img(source->memory, source->width, source->height, source->format);
#endif

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_blit_rect(vg_lite_buffer_t* target,
                                vg_lite_buffer_t* source,
                                vg_lite_rectangle_t* rect,
                                vg_lite_matrix_t* matrix,
                                vg_lite_blend_t blend,
                                vg_lite_color_t color,
                                vg_lite_filter_t filter)
{
    DUMP_API_CALL(vg_lite_blit_rect, target, source, rect, matrix, blend, color, filter);
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_BLIT_RECT_API);
    VG_LITE_TRACE_API("vg_lite_blit_rect %p %p %p %p %d 0x%08X %d\n", target, source, rect, matrix, blend, color, filter);

#if gcFEATURE_VG_IM_INPUT
    vg_lite_error_t error;
    vg_lite_point_t point_min, point_max, temp;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_float_t x_step[3];
    vg_lite_float_t y_step[3];
    vg_lite_float_t c_step[3];
    vg_lite_uint32_t imageMode = 0;
    vg_lite_uint32_t paintType = 0;
    vg_lite_uint32_t in_premult = 0;
    vg_lite_uint32_t blend_mode;
    vg_lite_uint32_t filter_mode = 0;
    vg_lite_uint32_t transparency_mode = 0;
    vg_lite_uint32_t conversion = 0;
    vg_lite_uint32_t rect_x = 0, rect_y = 0, rect_w = 0, rect_h = 0;
    vg_lite_uint32_t tiled_source;
    vg_lite_uint32_t yuv2rgb = 0;
    vg_lite_uint32_t uv_swiz = 0;
    vg_lite_uint32_t compress_mode = 0;
    vg_lite_uint32_t src_premultiply_enable = 0;
    vg_lite_uint32_t index_endian = 0;
    vg_lite_uint32_t eco_fifo = 0;
    vg_lite_uint32_t tile_setting = 0;
    vg_lite_uint32_t stripe_mode = 0;
    vg_lite_uint32_t premul_flag = 0;
    vg_lite_uint32_t prediv_flag = 0;
    vg_lite_int32_t  left, top, right, bottom;
    vg_lite_int32_t  stride;
    vg_lite_uint8_t enable_sw_pre_opt = 0;
    vg_lite_porter_duff_config_t porter_duff_config;

#if gcFEATURE_VG_NEW_FACTOR
    vg_factor_config_t factor_config;
    factor_config.factor_src_alpha = 0x0;
    factor_config.factor_src_color = 0x0;
    factor_config.factor_dst_alpha = 0x3;
    factor_config.factor_dst_color = 0x5;
    factor_config.final_equation_opcode = 0x0;
    factor_config.dstchannelmode = 0x0;
    factor_config.srcchannelmode = 0x0;
#endif
#if DUMP_CAPTURE
    vg_lite_float_t ratio = 1;
#endif
#if !gcFEATURE_VG_LVGL_SUPPORT
    vg_lite_uint8_t  lvgl_sw_blend = 0;
#endif
#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((target->format >= VG_LITE_ABGR8565_PLANAR) && (target->format <= VG_LITE_RGBA5658_PLANAR))
    {
        if (target->sw24bit_buffer)
        {
            target->sw24bit_buffer->format = convert_24bit_format(target->format);
            target = target->sw24bit_buffer;
        }
    }
    if ((source->format >= VG_LITE_ABGR8565_PLANAR) && (source->format <= VG_LITE_RGBA5658_PLANAR))
    {
        if (source->sw24bit_buffer)
        {
            vg_lite_convert_24bitplanar_to_24bit(source, source->sw24bit_buffer);
            source = source->sw24bit_buffer;
        }
    }
#endif
#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    vg_lite_uint8_t* buffer_pointer;
    vg_lite_uint32_t buffer_address = 0, buffer_scissor_address = 0, buffer_mask_address = 0, mul = 0, div = 0, required_align = 0;
    vg_lite_buffer_t new_target;
    vg_lite_float_point_t top_right_point, bottom_right_point, top_left_point, bottom_left_point;
    vg_lite_point_t image_display_top_left = { 0 };
    vg_lite_point_t image_display_range = { 0 };
    vg_lite_point_t new_target_top_left = { 0 };
    vg_lite_float_point4_t image_position_on_new_target = { 0 };
    vg_lite_float_point4_t image_display_position_on_new_target = { 0 };
    vg_lite_point_t image_display_top_left_on_new_target = { 0 };
    vg_lite_point_t image_display_range_on_new_target = { 0 };
    vg_lite_matrix_t matrix2_temp = { 0 };
    vg_lite_matrix_t temp_matrix;

    if (matrix != NULL && target->tiled != VG_LITE_TILED && target->compress_mode == VG_LITE_DEC_DISABLE && ((target->format >= VG_LITE_RGBA8888 && target->format <= VG_LITE_BGRA5551) || target->format == VG_LITE_A8
        || target->format == VG_LITE_L8 || (target->format >= VG_LITE_RGBA2222 && target->format <= VG_LITE_XRGB8888) || (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658_PLANAR))) {
        enable_sw_pre_opt = 1;
        memcpy(&temp_matrix, matrix, sizeof(vg_lite_matrix_t));
    }
#endif /* gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW */


#if gcFEATURE_VG_FLEXA
    if (s_context.sync_mode)
    {
        printf("When Flexa is enabled vg_lite_blit_rect is not support.\n");
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_RETURN_ERROR(feature_check_source_index_endian(source->format, source->index_endian));
    VG_LITE_RETURN_ERROR(feature_check_source_rgba8888_etc2_eac(source->format, source->width, source->height));
    VG_LITE_RETURN_ERROR(feature_check_source_rgb888_etc2_eac(source->format, source->width, source->height));
    VG_LITE_RETURN_ERROR(feature_check_source_packed_yuy_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_planar_yuv_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_planar_nv24_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_ayuv_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_yuv_tiled_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(source->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_planar_format(source->format));
    VG_LITE_RETURN_ERROR(feature_check_im_dec_input_compress(source->compress_mode));
    VG_LITE_RETURN_ERROR(feature_check_stencil_image_mode(source->image_mode));
    VG_LITE_RETURN_ERROR(feature_check_dst_screen_copy_blend(target, &blend));
    VG_LITE_RETURN_ERROR(feature_check_new_blend_mode(blend));
    VG_LITE_RETURN_ERROR(feature_check_lvgl_recolor_image_mode(source->image_mode));
    VG_LITE_RETURN_ERROR(feature_check_mesh_blt_sw_lvgl_blend(blend, s_context.mesh_mode));
    VG_LITE_RETURN_ERROR(blit_check_blend_on_yuy_target(blend, target->format));
    VG_LITE_RETURN_ERROR(chip_check_target_format(target->format));
    VG_LITE_RETURN_ERROR(chip_check_source_format(source->format));
    VG_LITE_RETURN_ERROR(feature_check_a124_a8l8_target_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_a124_a8l8_source_format(source->format));
    VG_LITE_RETURN_ERROR(srcbuf_align_check(source));
    VG_LITE_RETURN_ERROR(feature_check_compress(source->format, source->compress_mode, source->tiled, source->width, source->height));
#endif /* gcFEATURE_VG_ERROR_CHECK */

#if !gcFEATURE_VG_LVGL_SUPPORT
    if ((blend >= VG_LITE_BLEND_ADDITIVE_LVGL && blend <= VG_LITE_BLEND_MULTIPLY_LVGL) || (blend == VG_LITE_BLEND_NORMAL_LVGL && gcFEATURE_VG_SRC_PREMULTIPLIED)) {
        if (!source->lvgl_buffer) {
            source->lvgl_buffer = (vg_lite_buffer_t *)vg_lite_os_malloc(sizeof(vg_lite_buffer_t));
            *source->lvgl_buffer = *source;
            source->lvgl_buffer->lvgl_buffer = NULL;
            vg_lite_allocate(source->lvgl_buffer);
        }
        /* Make sure render target is up to date before reading RT. */
        vg_lite_finish();
        setup_lvgl_image(target, source, source->lvgl_buffer, blend);
        blend = VG_LITE_BLEND_SRC_OVER;
        lvgl_sw_blend = 1;
    }
#endif

    if (!matrix) {
        matrix = &identity_mtx;
    }

    chip_get_source_index_endian_bits(source->format, source->index_endian, &index_endian);
#if !gcFEATURE_VG_STRIPE_MODE_DISABLE
    /* Enable fifo feature to share buffer between vg and ts to improve the rotation performance */
    eco_fifo = 1 << 7;
#endif

    transparency_mode = (source->transparency_mode == VG_LITE_IMAGE_TRANSPARENT ? 0x8000:0);

    porter_duff_config = s_context.porter_duff_config;
    /* Check if the specified matrix has rotation or perspective. */
    if (   (   (matrix->m[0][1] != 0.0f)
            || (matrix->m[1][0] != 0.0f)
            || (matrix->m[2][0] != 0.0f)
            || (matrix->m[2][1] != 0.0f)
            || (matrix->m[2][2] != 1.0f)
            )
        && ((   blend == VG_LITE_BLEND_NONE
            || blend == VG_LITE_BLEND_SRC_IN
            || blend == VG_LITE_BLEND_DST_IN
            )
            || check_porter_duff_factor_match(blend, porter_duff_config)
            )
        ) {
            feature_border_culling_special_process(&blend, &transparency_mode);
#if !gcFEATURE_VG_STRIPE_MODE_DISABLE
            stripe_mode = 1 << 29;
#endif
    }

    conversion = feature_a124_a8l8_l8_conversion(target->format, source->format);

#if gcFEATURE_VG_16PIXELS_ALIGNED
    /* Check if source specify bytes are aligned */
    error = _check_source_aligned(source->format, source->stride);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }
#endif
    /* Set source region. */
    if (rect != NULL) {
        rect_x = (rect->x < 0) ? 0 : rect->x;
        rect_y = (rect->y < 0) ? 0 : rect->y;
        rect_w = rect->width;
        rect_h = rect->height;
        if ((rect_x > (vg_lite_uint32_t)source->width) || (rect_y > (vg_lite_uint32_t)source->height) ||
            (rect_w == 0) || (rect_h == 0))
        {
            printf("The width or height of the rect shouldn't be 0.\n");
            /*No intersection*/
            return VG_LITE_INVALID_ARGUMENT;
        }
        if (rect_x + rect_w > (vg_lite_uint32_t)source->width)
        {
            rect_w = source->width - rect_x;
        }
        if (rect_y + rect_h > (vg_lite_uint32_t)source->height)
        {
            rect_h = source->height - rect_y;
        }
    }
    else {
        rect_x = rect_y = 0;
        rect_w = source->width;
        rect_h = source->height;
    }

    s_context.filter = filter;

    /* Transform image (0,0) to screen. */
    if (!transform(&temp, 0.0f, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Set initial point. */
    point_min = temp;
    point_max = temp;
#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if (enable_sw_pre_opt) {
        if (!transform_float(&top_left_point, 0.0f, 0.0f, matrix))
            return VG_LITE_INVALID_ARGUMENT;
    }
#endif

    /* Transform image (0,height) to screen. */
    if (!transform(&temp, 0.0f, (vg_lite_float_t)rect_h, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if (enable_sw_pre_opt) {
        if (!transform_float(&bottom_left_point, 0.0f, (vg_lite_float_t)rect_h, matrix))
            return VG_LITE_INVALID_ARGUMENT;
    }
#endif

    /* Transform image (width,height) to screen. */
    if (!transform(&temp, (vg_lite_float_t)rect_w, (vg_lite_float_t)rect_h, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if (enable_sw_pre_opt) {
        if (!transform_float(&bottom_right_point, (vg_lite_float_t)rect_w, (vg_lite_float_t)rect_h, matrix))
            return VG_LITE_INVALID_ARGUMENT;
    }
#endif

    /* Transform image (width,0) to screen. */
    if (!transform(&temp, (vg_lite_float_t)rect_w, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;

    s_context.filter = 0;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if (enable_sw_pre_opt) {
        if (!transform_float(&top_right_point, (vg_lite_float_t)rect_w, 0.0f, matrix))
            return VG_LITE_INVALID_ARGUMENT;
    }
#endif

    /* Clip to target. */
    left = 0;
    top = 0;
    right = target->width;
    bottom = target->height;

    if (s_context.scissor_set && !target->scissor_buffer) {
        left = MAX(s_context.scissor[0], left);
        top = MAX(s_context.scissor[1], top);
        right = MIN(s_context.scissor[2], right);
        bottom = MIN(s_context.scissor[3], bottom);
    }

    if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer) {
        left = MAX(s_context.scissor_layer_range[0], left);
        top = MAX(s_context.scissor_layer_range[1], top);
        right = MIN(s_context.scissor_layer_range[2], right);
        bottom = MIN(s_context.scissor_layer_range[3], bottom);
    }

    point_min.x = MAX(point_min.x, left);
    point_min.y = MAX(point_min.y, top);
    point_max.x = MIN(point_max.x, right);
    point_max.y = MIN(point_max.y, bottom);
#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if (enable_sw_pre_opt) {
        image_display_top_left.x = point_min.x;
        image_display_top_left.y = point_min.y;
        image_display_range.x = point_max.x - point_min.x;
        image_display_range.y = point_max.y - point_min.y;
        image_display_range_on_new_target = image_display_range;
    }
#endif

    /* No need to draw. */
    if ((point_max.x <= point_min.x) || (point_max.y <= point_min.y)) {
        return VG_LITE_SUCCESS;
    }

#if gcFEATURE_VG_GAMMA
    get_st_gamma_src_dest(source, target);
#endif

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_GLOBAL));
#endif

    /*blend input into context*/
    s_context.blend_mode = blend;
    in_premult = 0x00000000;

    /* Adjust premultiply setting according to openvg condition */
    src_premultiply_enable = 0x01000100;
#if gcFEATURE_VG_PIXEL_MATRIX
    if (s_context.matrix_enable == 0 && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
#else
    if (s_context.color_transform == 0  && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
#endif
    else {
        prediv_flag = 1;
    }
    if ((s_context.blend_mode >= OPENVG_BLEND_SRC && s_context.blend_mode <= OPENVG_BLEND_ADDITIVE) || source->image_mode == VG_LITE_STENCIL_MODE
        || is_lvgl_blend_mode(s_context.blend_mode)){
        premul_flag = 1;
    }
    else {
        premul_flag = 0;
    }

    if ((source->premultiplied == 0 && target->premultiplied == 0 && premul_flag == 0) ||
        (source->premultiplied == 1 && target->premultiplied == 0 && prediv_flag == 0)) {
        src_premultiply_enable = 0x01000100;
        in_premult = 0x10000000;
    }
    /* when src and dst all pre format, im pre_out set to 0 to perform data truncation to prevent data overflow */
    else if (source->premultiplied == 1 && target->premultiplied == 1 && prediv_flag == 0) {
        src_premultiply_enable = 0x00000100;
        in_premult = 0x00000000;
    }
    else if ((source->premultiplied == 0 && target->premultiplied == 1) ||
              (source->premultiplied == 0 && target->premultiplied == 0 && premul_flag == 1)) {
        src_premultiply_enable = 0x01000100;
        in_premult = 0x00000000;
    }
    else if ((source->premultiplied == 1 && target->premultiplied == 1 && prediv_flag == 1) ||
             (source->premultiplied == 1 && target->premultiplied == 0 && prediv_flag == 1)) {
        src_premultiply_enable = 0x00000100;
        in_premult = 0x00000000;
    }
    if ((source->format == VG_LITE_A1 || source->format == VG_LITE_A2 || source->format == VG_LITE_A4 || source->format == VG_LITE_A8)
        && blend >= VG_LITE_BLEND_SRC_OVER && blend <= VG_LITE_BLEND_SUBTRACT) {
        chip_adjust_src_premultiply_enable(&src_premultiply_enable);
#if gcFEATURE_VG_SRC_PREMULTIPLIED
        src_premultiply_enable = src_premultiply_enable & ~(1 << 8);
#endif
        in_premult = 0x00000000;
    }
    if (source->premultiplied == target->premultiplied && premul_flag == 0) {
        target->apply_premult = 1;
    }
    else {
        target->apply_premult = 0;
    }

#if !gcFEATURE_VG_NEW_FACTOR
#if (gcFEATURE_VG_SRC_PREMULTIPLIED == 0)
    if (blend == VG_LITE_BLEND_NORMAL_LVGL)
        in_premult = 0x00000000;
#endif
#endif

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if (enable_sw_pre_opt) {
        //update target memory address
        get_format_bytes(target->format, &mul, &div, &required_align);

        vg_lite_uint32_t temp_buffer_address = target->address;

        vg_lite_uint32_t temp_buffer_address1 = 0;
        if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer)
            temp_buffer_address1 = s_context.scissor_layer->address;

        vg_lite_uint32_t temp_buffer_address2 = 0;
        if (s_context.enable_mask && s_context.mask_layer)
            temp_buffer_address2 = s_context.mask_layer->address;

        vg_lite_int32_t align_require = 4;
        vg_lite_int32_t align_require_temp = 4;
        vg_lite_int32_t align_flag = 0;

        if (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658_PLANAR)
            align_require = 64;

        vg_lite_int32_t dy = image_display_top_left.y;
        vg_lite_int32_t dx = image_display_top_left.x;

        for (; dy > 0; dy--)
        {
            for (; dx > 0; dx--)
            {
                if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer)
                    dx = dx & (~7); // dx need 8 align

                buffer_address = temp_buffer_address + dy * target->stride + dx * (mul / div);
                if ((buffer_address & (align_require - 1)) == 0) {
                    if (s_context.scissor_enable && s_context.enable_mask) {
                        if (s_context.scissor_layer->scissor_buffer && s_context.mask_layer) {
                            buffer_scissor_address = temp_buffer_address1 + dy * s_context.scissor_layer->stride + dx / 8;
                            buffer_mask_address = temp_buffer_address2 + dy * s_context.mask_layer->stride + dx;

                            if (((buffer_scissor_address & (align_require_temp - 1)) == 0) && ((buffer_mask_address & (align_require_temp - 1)) == 0)) {
                                align_flag = 1;
                                break;
                            }
                        }
                    }
                    else if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer) { //then check scissor layer
                        buffer_scissor_address = temp_buffer_address1 + dy * s_context.scissor_layer->stride + dx / 8;

                        if ((buffer_scissor_address & (align_require_temp - 1)) == 0) {
                            align_flag = 1;
                            break;
                        }
                    }
                    else if (s_context.enable_mask && s_context.mask_layer) {
                        buffer_mask_address = temp_buffer_address2 + dy * s_context.mask_layer->stride + dx;

                        if ((buffer_mask_address & (align_require_temp - 1)) == 0) {
                            align_flag = 1;
                            break;
                        }
                    }
                    else {
                        align_flag = 1;
                        break;
                    }
                }                    
            }
            if (align_flag)
                break;
            dx = image_display_top_left.x;
        }

        if (!align_flag) {
            dx = dy = 0;

            if (s_context.scissor_enable)
                buffer_scissor_address = temp_buffer_address1;
            if (s_context.enable_mask)
                buffer_mask_address = temp_buffer_address2;
        }

        if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A16, buffer_scissor_address));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A17, s_context.scissor_layer->stride));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000100));
        }

        if (s_context.enable_mask && s_context.mask_layer) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A14, buffer_mask_address));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A15, s_context.mask_layer->stride));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000010));
        }

        //update new target coordinate
        new_target_top_left.x = dx;
        new_target_top_left.y = dy;
        image_display_top_left_on_new_target.x = image_display_top_left.x - new_target_top_left.x; image_display_top_left_on_new_target.y = image_display_top_left.y - new_target_top_left.y;

        if (align_flag) {
            //update buffer pointer address
            buffer_pointer = (vg_lite_uint8_t*)target->memory;
            buffer_pointer = buffer_pointer + (buffer_address - target->address);

            /* Calculate the transformation matrix */
            image_position_on_new_target[0].x = 0;                          image_position_on_new_target[0].y = 0;
            image_position_on_new_target[1].x = 0;                          image_position_on_new_target[1].y = (vg_lite_float_t)rect_h;
            image_position_on_new_target[2].x = (vg_lite_float_t)rect_w;    image_position_on_new_target[2].y = (vg_lite_float_t)rect_h;
            image_position_on_new_target[3].x = (vg_lite_float_t)rect_w;    image_position_on_new_target[3].y = 0;

            image_display_position_on_new_target[0].x = top_left_point.x - new_target_top_left.x;      image_display_position_on_new_target[0].y = top_left_point.y - new_target_top_left.y;
            image_display_position_on_new_target[1].x = bottom_left_point.x - new_target_top_left.x;   image_display_position_on_new_target[1].y = bottom_left_point.y - new_target_top_left.y;
            image_display_position_on_new_target[2].x = bottom_right_point.x - new_target_top_left.x;  image_display_position_on_new_target[2].y = bottom_right_point.y - new_target_top_left.y;
            image_display_position_on_new_target[3].x = top_right_point.x - new_target_top_left.x;     image_display_position_on_new_target[3].y = top_right_point.y - new_target_top_left.y;
            vg_lite_get_transform_matrix(image_position_on_new_target, image_display_position_on_new_target, &matrix2_temp);
            matrix2_temp.scaleX = matrix->scaleX;
            matrix2_temp.scaleY = matrix->scaleY;
            matrix2_temp.angle = matrix->angle;

            //update new_target and set  it as target
            memcpy(&new_target, target, sizeof(vg_lite_buffer_t));
            new_target.address = buffer_address;
            new_target.memory = buffer_pointer;
            target = &new_target;

            //update matrix
            memcpy(matrix, &matrix2_temp, sizeof(vg_lite_matrix_t));
        }
    }
#endif /* gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW */
    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix))
        return VG_LITE_SUCCESS;

    VG_LITE_RETURN_ERROR(chip_calculate_blit_image_steps(chip_process_blit_boundary_point(rect_x, rect_y, rect_w),
        rect_h, filter, matrix, &inverse_matrix, x_step, y_step, c_step, enable_sw_pre_opt));

    /* Determine image mode (NORMAL, NONE , MULTIPLY or STENCIL) depending on the color. */
    switch (source->image_mode) {
        case VG_LITE_NONE_IMAGE_MODE:
            imageMode = 0x00000000;
            break;
        case VG_LITE_MULTIPLY_IMAGE_MODE:
            imageMode = 0x00002000;
            break;
        case VG_LITE_NORMAL_IMAGE_MODE:
        case VG_LITE_ZERO:
            imageMode = 0x00001000;
            break;
        case VG_LITE_STENCIL_MODE:
            imageMode = 0x00003000;
            break;
        case VG_LITE_RECOLOR_MODE:
            imageMode = 0x00006000;
            break;
    }

    switch (filter) {
    case VG_LITE_FILTER_POINT:
        filter_mode = 0;
        break;

    case VG_LITE_FILTER_LINEAR:
        filter_mode = 0x10000;
        break;

    case VG_LITE_FILTER_BI_LINEAR:
        filter_mode = 0x20000;
        break;

    case VG_LITE_FILTER_GAUSSIAN:
        filter_mode = 0x30000;
        break;
    }

    switch (source->paintType)
    {
    case VG_LITE_PAINT_COLOR:
        paintType = 0;
        break;

    case VG_LITE_PAINT_LINEAR_GRADIENT:
        paintType = 1 << 24;
        break;

    case VG_LITE_PAINT_RADIAL_GRADIENT:
        paintType = 1 << 25;
        break;

    case VG_LITE_PAINT_PATTERN:
        paintType = 1 << 24 | 1 << 25;
        break;

    default:
        break;
    }

    blend_mode = convert_blend(blend);
    tiled_source = (source->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0 ;
    VG_LITE_RETURN_ERROR(chip_set_rec_tile(target, source, matrix, &tile_setting, &stripe_mode));

    compress_mode = (vg_lite_uint32_t)source->compress_mode << 25;

#if gcFEATURE_VG_NEW_FACTOR
    config_factor_parameter(blend, porter_duff_config, &factor_config);
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF6, factor_config.srcchannelmode | (factor_config.dstchannelmode << 8)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF8, factor_config.factor_src_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF9, factor_config.factor_src_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFA, factor_config.factor_dst_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFB, factor_config.factor_dst_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF7, factor_config.final_equation_opcode));
#endif

    /* Setup the command buffer. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000001 | paintType | in_premult | imageMode | blend_mode | transparency_mode | tile_setting | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | eco_fifo | s_context.scissor_enable | stripe_mode));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A18, (vg_lite_void *) &c_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A19, (vg_lite_void *) &c_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1A, (vg_lite_void *) &c_step[2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1C, (vg_lite_void *) &x_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1D, (vg_lite_void *) &x_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1E, (vg_lite_void *) &x_step[2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1F, 0x00000001));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A20, (vg_lite_void *) &y_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A21, (vg_lite_void *) &y_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A22, (vg_lite_void *) &y_step[2]));

    if (((source->format >= VG_LITE_YUY2) &&
         (source->format <= VG_LITE_AYUY2)) ||
        ((source->format >= VG_LITE_YUY2_TILED) &&
         (source->format <= VG_LITE_AYUY2_TILED))) {
            yuv2rgb = convert_yuv2rgb(source->yuv.yuv2rgb);
            uv_swiz = convert_uv_swizzle(source->yuv.swizzle);
    }

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A25, convert_source_format(source->format) | filter_mode | uv_swiz | yuv2rgb | conversion | compress_mode | src_premultiply_enable | index_endian));
    /* 24bit format stride configured to 4bpp. */
    if (source->format >= VG_LITE_RGB888 && source->format <= VG_LITE_RGBA5658) {
        stride = source->stride / 3 * 4;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, stride | tiled_source));
    }
    else {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, source->stride | tiled_source));
    }

    if (source->yuv.uv_planar) {
        /* Program u plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A51, source->yuv.uv_planar));
    }
    if (source->yuv.v_planar) {
        /* Program v plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source->yuv.v_planar));
    }
    if (source->yuv.alpha_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source->yuv.alpha_planar));
    }
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A27, target->bg_color));

#if !gcFEATURE_VG_LVGL_SUPPORT
    if (lvgl_sw_blend) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source->lvgl_buffer->address));
    }
    else
#endif
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source->address));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2D, rect_x | (rect_y << 16)));
    
#if gcFEATURE_VG_BOUNDARY_FILTER_BYPASS && gcFEATURE_VG_16PIXELS_ALIGNED
    if (rect_x == 0 && rect_y == 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2F, ((rect_w + 15) & ~15) | (rect_h << 16)));
    }else
#endif  
    {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2F, rect_w | (rect_h << 16)));
    }
    
#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if (enable_sw_pre_opt) {
        VG_LITE_RETURN_ERROR(push_rectangle(&s_context, image_display_top_left_on_new_target.x, image_display_top_left_on_new_target.y, image_display_range_on_new_target.x, image_display_range_on_new_target.y));
    }
    else
#endif /* gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW */
    {
        VG_LITE_RETURN_ERROR(push_rectangle(&s_context, point_min.x, point_min.y, point_max.x - point_min.x, point_max.y - point_min.y));
    }

#if !gcFEATURE_VG_STRIPE_MODE_DISABLE
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0E02, 0x10 | (0x7 << 8)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0F00, 0x10 | (0x7 << 8)));
#endif

    error = flush_target();

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_NORMAL));
#endif

     s_context.filter = 0;

     VG_LITE_BREAK_ERROR(push_state(&s_context, 0x0A1B, 0x00000011));

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
    if (enable_sw_pre_opt) {
        if (s_context.scissor_enable && s_context.scissor_layer && s_context.scissor_layer->scissor_buffer) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A17, s_context.scissor_layer->stride));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A16, s_context.scissor_layer->address));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000100));
        }

        if (s_context.enable_mask && s_context.mask_layer) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A15, s_context.mask_layer->stride));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A14, s_context.mask_layer->address));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000010));
        }

        memcpy(matrix, &temp_matrix, sizeof(vg_lite_matrix_t));
    }
#endif /* gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW */

#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((target->format >= VG_LITE_ABGR8565) && (target->format <= VG_LITE_RGBA5658))
    {
        if (target->sw24bit_planar_buffer)
        {
            target = target->sw24bit_planar_buffer;
        }
    }
    if ((source->format >= VG_LITE_ABGR8565) && (source->format <= VG_LITE_RGBA5658))
    {
        if (source->sw24bit_planar_buffer)
        {
            source = source->sw24bit_planar_buffer;
        }
    }
#endif
#if DUMP_CAPTURE
    if (source->compress_mode)
        ratio = _calc_decnano_compress_ratio(source->format, source->compress_mode);
    vglitemDUMP_BUFFER("image", (size_t)source->address, source->memory, 0, (size_t)((source->stride)*(source->height)*ratio));
#endif
#if DUMP_IMAGE
    dump_img(source->memory, source->width, source->height, source->format);
#endif

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_init(vg_lite_uint32_t tess_width, vg_lite_uint32_t tess_height)
{
    DUMP_API_CALL(vg_lite_init, tess_width, tess_height);
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_INIT_API);
    VG_LITE_TRACE_API("vg_lite_init %d %d\n", tess_width, tess_height);

    vg_lite_error_t error;
    vg_lite_kernel_initialize_t initialize;
    vg_lite_uint8_t i;


    if (s_context.state != 0) {
        if (s_context.tess_width >= tess_width && s_context.tess_height >= tess_height) {
            /* VGLite is already initialized properly. Return */
            return VG_LITE_SUCCESS;
        }
        else {
            VG_LITE_RETURN_ERROR(vg_lite_close());
        }
    }

    s_context.rtbuffer = (vg_lite_buffer_t *)vg_lite_os_malloc(sizeof(vg_lite_buffer_t));
    if (!s_context.rtbuffer)
        return VG_LITE_OUT_OF_RESOURCES;
    memset(s_context.rtbuffer, 0, sizeof(vg_lite_buffer_t));

    if (tess_width <= 0) {
        tess_width = 0;
        tess_height = 0;
    }
    if (tess_height <= 0) {
        tess_height = 0;
        tess_width = 0;
    }
    tess_width  = VG_LITE_ALIGN(tess_width, 16);

    /* Allocate a command buffer and a tessellation buffer.
       Add extra 8 bytes in the allocated command buffer so there is space for a END command. */
    initialize.command_buffer_size = command_buffer_size + 8;
    initialize.tess_width = tess_width;
    initialize.tess_height = tess_height;
    initialize.command_buffer_pool = (vg_lite_vidmem_pool_t)s_context.command_buffer_pool;
    initialize.tess_buffer_pool = (vg_lite_vidmem_pool_t)s_context.tess_buffer_pool;
    initialize.context = &s_context.context;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_INITIALIZE, &initialize));

    /* Verify driver ChipId/ChipRevision/Cid match hardware chip information */
    VG_LITE_RETURN_ERROR(check_hardware_chip_info());

    /* Save draw context. */
    s_context.capabilities = initialize.capabilities;
    s_context.command_buffer[0] = (vg_lite_uint8_t *)initialize.command_buffer[0];
    s_context.command_buffer[1] = (vg_lite_uint8_t *)initialize.command_buffer[1];
    s_context.command_buffer_size = command_buffer_size;
    s_context.command_offset[0] = 0;
    s_context.command_offset[1] = 0;

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
    s_context.fb_command_buffer = (vg_lite_uint8_t*)initialize.fb_command_buffer;
    s_context.fb_command_buffer_size = initialize.fb_command_buffer_size;
    s_context.fb_command_buffer_physical = initialize.fb_command_buffer_gpu;
    s_context.fb_command_buffer_index = 0;
#endif

    if ((tess_width  > 0) && (tess_height > 0))
    {
        /* Set and Program Tessellation Buffer states. */
        s_context.tessbuf.physical_addr = initialize.physical_addr;
        s_context.tessbuf.logical_addr = initialize.logical_addr;
        s_context.tessbuf.tess_w_h = initialize.tess_w_h;
        s_context.tessbuf.tessbuf_size = initialize.tessbuf_size;
        s_context.tessbuf.countbuf_size = initialize.countbuf_size;

        VG_LITE_RETURN_ERROR(chip_program_tessellation(&s_context));
        /* Init register gcregVGPEColorKey. */
        for (i = 0; i < 8; i++) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A90 + i, 0));
        }
    }
    s_context.custom_tessbuf = 0;
    s_context.custom_cmdbuf = 0;
    s_context.tess_width = tess_width;
    s_context.tess_height = tess_height;

    /* Init scissor rect. */
    s_context.scissor[0] =
    s_context.scissor[1] =
    s_context.scissor[2] =
    s_context.scissor[3] = 0;

    s_context.path_counter = 0;

#if gcFEATURE_COMBO_VG_SPLIT_PATH_SUPPORT_BY_SW
    s_context.split_path = 1;
#endif

#if gcFEATURE_VG_MESH_FOR_FRAME
    s_context.mesh_dirty = 1;
#endif

#if gcFEATURE_VG_FLEXA
    s_context.flexa_dirty = 1;
#endif

    s_context.mirror_orient = VG_LITE_ORIENTATION_TOP_BOTTOM;

    /* backup init commands */
    memcpy(s_context.context.init_command_buffer_logical,
           s_context.context.command_buffer_logical[CMDBUF_INDEX(s_context)],
           CMDBUF_OFFSET(s_context));
    s_context.context.init_command_buffer_offset = CMDBUF_OFFSET(s_context);

#if DUMP_CAPTURE || DUMP_LAST_CAPTURE
    _SetDumpFileInfo();
#endif

    s_context.state = 1;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_close(vg_lite_void)
{
    DUMP_API_CALL(vg_lite_close);
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_CLOSE_API);
    VG_LITE_TRACE_API("vg_lite_close\n");

    vg_lite_error_t error;
    vg_lite_kernel_terminate_t terminate;


    if (s_context.state == 0) {
        /* VGLite is already closed properly. Return */
        return VG_LITE_SUCCESS;
    }

    /* Ensure the GPU finishes potential pending tasks. */
    VG_LITE_RETURN_ERROR(vg_lite_finish());

    if (s_context.scissor_layer)
    {
        vg_lite_free(s_context.scissor_layer);
        vg_lite_os_free(s_context.scissor_layer);
    }

    if (s_context.custom_cmdbuf)
    {
        vg_lite_kernel_unmap_memory_t unmap = {0};
        unmap.bytes = s_context.command_buffer_size * 2;
        unmap.logical = s_context.command_buffer[0];
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP_MEMORY, &unmap));
    }

    if (s_context.custom_tessbuf)
    {
        vg_lite_kernel_unmap_memory_t unmap = {0};
        unmap.bytes = s_context.tessbuf.tessbuf_size + s_context.tessbuf.countbuf_size;
        unmap.logical = s_context.tessbuf.logical_addr;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP_MEMORY, &unmap));
    }
    
    if (s_context.fb_command_buffer_start)
    {
        vg_lite_cache_cmd_info* buf_start = s_context.fb_command_buffer_start;
        vg_lite_cache_cmd_info* buf_next = NULL;

        while (buf_start != NULL)
        {
            buf_next = buf_start->next;
            vg_lite_os_free(buf_start);
            buf_start = buf_next;
        }

        s_context.fb_command_buffer_start = NULL;
    }

    /* Termnate the draw context. */
    terminate.context = &s_context.context;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_TERMINATE, &terminate));

    if (s_context.rtbuffer)
        vg_lite_os_free(s_context.rtbuffer);

    submit_flag = 0;

    /* Reset the draw context. */
    memset(&s_context, 0, sizeof(s_context));

#if DUMP_CAPTURE
    _SetDumpFileInfo();
#endif
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_set_command_buffer_size(vg_lite_uint32_t size)
{
    DUMP_API_CALL(vg_lite_set_command_buffer_size, size);
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_COMMAND_BUFFER_SIZE_API);
    VG_LITE_TRACE_API("vg_lite_set_command_buffer_size %d\n", size);

    if (command_buffer_size == 0)
        return VG_LITE_INVALID_ARGUMENT;

    command_buffer_size = size;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_set_command_buffer(vg_lite_uint32_t physical, vg_lite_uint32_t size)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_COMMAND_BUFFER_API);
    VG_LITE_TRACE_API("vg_lite_set_command_buffer 0x%08X %d\n", physical, size);

    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_map_memory_t map = { 0 };


    if ((physical == 0) || (size == 0) || (physical % 64) || (size % 128))
        return VG_LITE_INVALID_ARGUMENT;

    map.bytes = size;
    map.physical = physical;

    if (s_context.command_buffer[0])
    {
        
        if (submit_flag)
            VG_LITE_RETURN_ERROR(stall(&s_context, 0, (vg_lite_uint32_t)~0));

        if (!s_context.custom_cmdbuf)
        {
            vg_lite_kernel_free_t free;
            free.memory_handle = s_context.context.command_buffer[0];
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free));
            s_context.context.command_buffer[0] = 0;

            free.memory_handle = s_context.context.command_buffer[1];
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free));
            s_context.context.command_buffer[1] = 0;
        }
        else
        {
            vg_lite_kernel_unmap_memory_t unmap = { 0 };
            
            unmap.bytes = s_context.command_buffer_size + 8;
            unmap.logical = s_context.command_buffer[0];
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP_MEMORY, &unmap));
            unmap.bytes = s_context.command_buffer_size + 8;
            unmap.logical = s_context.command_buffer[1];
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP_MEMORY, &unmap));
        }
    }

    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP_MEMORY, &map));

    s_context.context.command_buffer_logical[0] = map.logical;
    s_context.context.command_buffer_physical[0] = map.physical;

    s_context.context.command_buffer_logical[1] = (vg_lite_pointer)((vg_lite_uint8_t*)map.logical + map.bytes / 2);
    s_context.context.command_buffer_physical[1] = map.physical + map.bytes / 2;

    s_context.command_buffer[0] = s_context.context.command_buffer_logical[0];
    s_context.command_buffer[1] = s_context.context.command_buffer_logical[1];
    s_context.command_offset[0] = 0;
    s_context.command_offset[1] = 0;
    s_context.command_buffer_current = 0;
    /* Reserve 8 bytes in mapped command buffer so there is space for a END command. */
    s_context.command_buffer_size = (map.bytes / 2) - 8;
    s_context.custom_cmdbuf = 1;

    return error;
}

vg_lite_error_t vg_lite_set_tess_buffer(vg_lite_uint32_t physical, vg_lite_uint32_t size)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_TESS_BUFFER_API);
    VG_LITE_TRACE_API("vg_lite_set_tess_buffer 0x%08X %d\n", physical, size);

    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_map_memory_t map = { 0 };


    if ((physical == 0) || (size == 0) || (physical % 64) || (size % 64) || (size < MIN_TS_SIZE))
        return VG_LITE_INVALID_ARGUMENT;

    map.bytes = size;
    map.physical = physical;

    if (s_context.tessbuf.logical_addr)
    {
        if (submit_flag)
            VG_LITE_RETURN_ERROR(stall(&s_context, 0, (vg_lite_uint32_t)~0));
        if (!s_context.custom_tessbuf)
        {
            vg_lite_kernel_free_t free;
            free.memory_handle = s_context.context.tess_buffer;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free));
            s_context.context.tess_buffer = 0;
        }
        else
        {
            vg_lite_kernel_unmap_memory_t unmap = { 0 };
        
            unmap.bytes = s_context.tessbuf.tessbuf_size + s_context.tessbuf.countbuf_size;
            unmap.logical = s_context.tessbuf.logical_addr;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP_MEMORY, &unmap));
        }
    }

    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP_MEMORY, &map));

    s_context.tessbuf.logical_addr = map.logical;
    s_context.tessbuf.physical_addr = map.physical;
    s_context.tessbuf.countbuf_size = size * 3 / 128;
    s_context.tessbuf.countbuf_size = VG_LITE_ALIGN(s_context.tessbuf.countbuf_size, 64);
    s_context.tessbuf.tessbuf_size = map.bytes - s_context.tessbuf.countbuf_size;

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A35, s_context.tessbuf.physical_addr));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AC8, s_context.tessbuf.tessbuf_size));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0ACB, s_context.tessbuf.physical_addr + s_context.tessbuf.tessbuf_size));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0ACC, s_context.tessbuf.countbuf_size));

    s_context.custom_tessbuf = 1;
    
    return error;
}

vg_lite_error_t vg_lite_get_mem_size(vg_lite_uint32_t* size)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_GET_MEM_SIZE_API);
    VG_LITE_TRACE_API("vg_lite_get_mem_size %p\n", size);

    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_mem_t mem;


    mem.pool = VG_LITE_POOL_RESERVED_MEMORY1;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_QUERY_MEM, &mem));
    *size = mem.bytes;

    return error;
}

static vg_lite_uint32_t _is_tiled_yuv_planer_format(vg_lite_buffer_format_t foramt)
{
    switch (foramt)
    {
    case VG_LITE_ANV12:
    case VG_LITE_NV12:
    case VG_LITE_YV12:
    case VG_LITE_YV24:
    case VG_LITE_YV16:
    case VG_LITE_NV16:
    case VG_LITE_NV12_TILED:
    case VG_LITE_ANV12_TILED:
    case VG_LITE_NV24:
    case VG_LITE_NV24_TILED:
        return 1;
    default:
        return 0;
    }
}

/* Handle tiled & yuv allocation. Currently including NV12, ANV12, YV12, YV16, NV16, YV24, NV24. */
static  vg_lite_error_t _allocate_tiled_yuv_planar(vg_lite_buffer_t *buffer)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t    yplane_size = 0;
    vg_lite_kernel_allocate_t allocate, uv_allocate, v_allocate;
    
    if (!_is_tiled_yuv_planer_format(buffer->format))
    {
        return error;
    }
    
    /* For NV12, there are 2 planes (Y, UV);
     For ANV12, there are 3 planes (Y, UV, Alpha).
     Each plane must be aligned by (4, 8).
     Then Y plane must be aligned by (8, 8).
     For YVxx, there are 3 planes (Y, U, V).
     YV12 is similar to NV12, both YUV420 format.
     YV16 and NV16 are YUV422 format.
     YV24 is YUV444 format.
     */
    buffer->width = VG_LITE_ALIGN(buffer->width, 8);
    buffer->height = VG_LITE_ALIGN(buffer->height, 8);
    buffer->stride = VG_LITE_ALIGN(buffer->width, 64);
    
    switch (buffer->format) {
        case VG_LITE_NV12:
        case VG_LITE_ANV12:
        case VG_LITE_NV12_TILED:
        case VG_LITE_ANV12_TILED:
            buffer->yuv.uv_stride = buffer->stride;
            buffer->yuv.alpha_stride = buffer->stride;
            buffer->yuv.uv_height = buffer->height / 2;
            break;
            
        case VG_LITE_NV16:
            buffer->yuv.uv_stride = buffer->stride;
            buffer->yuv.uv_height = buffer->height;
            break;

        case VG_LITE_NV24:
        case VG_LITE_NV24_TILED:
            buffer->yuv.uv_stride = buffer->stride * 2;
            buffer->yuv.uv_height = buffer->height;
            break;
            
        case VG_LITE_YV12:
            buffer->yuv.uv_stride =
            buffer->yuv.v_stride = buffer->stride / 2;
            buffer->yuv.uv_height =
            buffer->yuv.v_height = buffer->height / 2;
            break;
            
        case VG_LITE_YV16:
            buffer->yuv.uv_stride =
            buffer->yuv.v_stride = buffer->stride;
            buffer->yuv.uv_height =
            buffer->yuv.v_height = buffer->height / 2;
            break;
            
        case VG_LITE_YV24:
            buffer->yuv.uv_stride =
            buffer->yuv.v_stride = buffer->stride;
            buffer->yuv.uv_height =
            buffer->yuv.v_height = buffer->height;
            break;
            
        default:
            return error;
    }
    
    yplane_size = buffer->stride * buffer->height;
    
    /* Allocate buffer memory: Y. */
    allocate.bytes = yplane_size;
    allocate.contiguous = 1;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &allocate));
    
    /* Save the allocation. */
    buffer->handle  = allocate.memory_handle;
    buffer->memory  = allocate.memory;
    buffer->address = allocate.memory_gpu;
    
    if ((buffer->format == VG_LITE_NV12) || (buffer->format == VG_LITE_ANV12)
        || (buffer->format == VG_LITE_NV16) || (buffer->format == VG_LITE_NV24) || (buffer->format == VG_LITE_NV24_TILED)
        || (buffer->format == VG_LITE_NV12_TILED) || (buffer->format == VG_LITE_ANV12_TILED)) {
        /* Allocate buffer memory: UV. */
        uv_allocate.bytes = buffer->yuv.uv_stride * buffer->yuv.uv_height;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &uv_allocate));
        buffer->yuv.uv_handle = uv_allocate.memory_handle;
        buffer->yuv.uv_memory = uv_allocate.memory;
        buffer->yuv.uv_planar = uv_allocate.memory_gpu;
        
        if ((buffer->format == VG_LITE_ANV12) || (buffer->format == VG_LITE_ANV12_TILED)) {
            uv_allocate.bytes = yplane_size;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &uv_allocate));
            buffer->yuv.alpha_planar = uv_allocate.memory_gpu;
        }
    } else {
        /* Allocate buffer memory: U, V. */
        uv_allocate.bytes = buffer->yuv.uv_stride * buffer->yuv.uv_height;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &uv_allocate));
        buffer->yuv.uv_handle = uv_allocate.memory_handle;
        buffer->yuv.uv_memory = uv_allocate.memory;
        buffer->yuv.uv_planar = uv_allocate.memory_gpu;
        
        v_allocate.bytes = buffer->yuv.v_stride * buffer->yuv.v_height;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &v_allocate));
        buffer->yuv.v_handle = v_allocate.memory_handle;
        buffer->yuv.v_memory = v_allocate.memory;
        buffer->yuv.v_planar = v_allocate.memory_gpu;
    }
    
    return error;
}

/* Handle tiled & yuv map. Currently including NV12, ANV12, YV12, YV16, NV16, YV24, NV24. */
static  vg_lite_error_t _map_yuv_planar(vg_lite_buffer_t* buffer, vg_lite_map_flag_t flag, vg_lite_int32_t fd)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_map_t map;
    vg_lite_uint32_t yplane_size = 0;

    /* For NV12, there are 2 planes (Y, UV);
    For ANV12, there are 3 planes (Y, UV, Alpha).
    Each plane must be aligned by (4, 8).
    Then Y plane must be aligned by (8, 8).
    For YVxx, there are 3 planes (Y, U, V).
    YV12 is similar to NV12, both YUV420 format.
    YV16 and NV16 are YUV422 format.
    YV24 is YUV444 format.
    */
    if (((buffer->width % 8) != 0) || ((buffer->height % 8) != 0) || ((buffer->stride % 64) != 0)) 
        return VG_LITE_INVALID_ARGUMENT;
    buffer->stride = VG_LITE_ALIGN(buffer->width, 64);
    switch (buffer->format) {
    case VG_LITE_NV12:
    case VG_LITE_ANV12:
    case VG_LITE_NV12_TILED:
    case VG_LITE_ANV12_TILED:
        buffer->yuv.uv_stride = buffer->stride;
        buffer->yuv.alpha_stride = buffer->stride;
        buffer->yuv.uv_height = buffer->height / 2;
        break;

    case VG_LITE_NV16:
        buffer->yuv.uv_stride = buffer->stride;
        buffer->yuv.uv_height = buffer->height;
        break;

    case VG_LITE_NV24:
    case VG_LITE_NV24_TILED:
        buffer->yuv.uv_stride = buffer->stride * 2;
        buffer->yuv.uv_height = buffer->height;
        break;

    case VG_LITE_YV12:
        buffer->yuv.uv_stride =
            buffer->yuv.v_stride = buffer->stride / 2;
        buffer->yuv.uv_height =
            buffer->yuv.v_height = buffer->height / 2;
        break;

    case VG_LITE_YV16:
        buffer->yuv.uv_stride =
            buffer->yuv.v_stride = buffer->stride;
        buffer->yuv.uv_height =
            buffer->yuv.v_height = buffer->height / 2;
        break;

    case VG_LITE_YV24:
        buffer->yuv.uv_stride =
            buffer->yuv.v_stride = buffer->stride;
        buffer->yuv.uv_height =
            buffer->yuv.v_height = buffer->height;
        break;

    default:
        return error;
    }

    yplane_size = buffer->stride * buffer->height;
    /* Map the buffer. */
    map.bytes = yplane_size;
    map.logical = buffer->memory;
    map.physical = buffer->address;

    if (flag == VG_LITE_MAP_USER_MEMORY) {
        map.flags = VG_LITE_HAL_MAP_USER_MEMORY;
    }
    else if (flag == VG_LITE_MAP_DMABUF) {
        map.flags = VG_LITE_HAL_MAP_DMABUF;
    }
    else {
        return VG_LITE_INVALID_ARGUMENT;
    }

    map.dma_buf_fd = fd;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP, &map));
    /* Save the buffer allocation. */
    buffer->handle = map.memory_handle;
    buffer->address = map.memory_gpu;

    if ((buffer->format == VG_LITE_NV12) || (buffer->format == VG_LITE_ANV12)
        || (buffer->format == VG_LITE_NV16) || (buffer->format == VG_LITE_NV24) || (buffer->format == VG_LITE_NV24_TILED)
        || (buffer->format == VG_LITE_NV12_TILED) || (buffer->format == VG_LITE_ANV12_TILED)) {
        /* Map buffer memory: UV. */
        map.bytes = buffer->yuv.uv_stride * buffer->yuv.uv_height;
        map.logical = buffer->yuv.uv_memory;
        map.physical = buffer->yuv.uv_planar;
        map.memory_gpu = 0;
        map.memory_handle = NULL;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP, &map));
        buffer->yuv.uv_handle = map.memory_handle;
        buffer->yuv.uv_planar = map.memory_gpu;

        if ((buffer->format == VG_LITE_ANV12) || (buffer->format == VG_LITE_ANV12_TILED)) {
            map.bytes = yplane_size / 2;
            map.logical = buffer->yuv.alpha_memory;
            map.physical = 0;
            map.memory_gpu = 0;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP, &map));
            buffer->yuv.alpha_handle = map.memory_handle;
            buffer->yuv.alpha_planar = map.memory_gpu;
        }
    }
    else {
        /* Map buffer memory: U, V. */
        map.bytes = buffer->yuv.uv_stride * buffer->yuv.uv_height;
        map.logical = buffer->yuv.uv_memory;
        map.physical = 0;
        map.memory_gpu = 0;
        map.memory_handle = NULL;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP, &map));
        buffer->yuv.uv_handle = map.memory_handle;
        buffer->yuv.uv_planar = map.memory_gpu;

        map.bytes = buffer->yuv.v_stride * buffer->yuv.v_height;
        map.logical = buffer->yuv.v_memory;
        map.physical = 0;
        map.memory_gpu = 0;
        map.memory_handle = NULL;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP, &map));
        buffer->yuv.v_handle = map.memory_handle;
        buffer->yuv.v_planar = map.memory_gpu;
    }

    return error;
   
}

vg_lite_error_t vg_lite_allocate(vg_lite_buffer_t * buffer)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_allocate_t allocate;

    DUMP_API_CALL(vg_lite_allocate, buffer);
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_ALLOCATE_API);
    VG_LITE_TRACE_API("vg_lite_allocate %p  (w: %d, h: %d, fmt: %d)\n", buffer, buffer->width, buffer->height, buffer->format);

    if ((buffer->format == VG_LITE_RGBA8888_ETC2_EAC || buffer->format == VG_LITE_RGB888_ETC2_EAC) && (buffer->width % 4 || buffer->height % 4))
        return VG_LITE_INVALID_ARGUMENT;

    /* Default to tiled mode for ETC2 format */
    if (buffer->format == VG_LITE_RGBA8888_ETC2_EAC || buffer->format == VG_LITE_RGB888_ETC2_EAC) {
        buffer->tiled = VG_LITE_TILED;
    }

    /* Set buffer->premultiplied properly according to buffer->format */
    if (buffer->format < VG_LITE_RGBA8888)
    {   /* For all OpenVG VG_* formats */
#if gcFEATURE_VG_HW_PREMULTIPLY
        switch (buffer->format) {
            case OPENVG_sRGBA_8888_PRE:
            case OPENVG_lRGBA_8888_PRE:
            case OPENVG_sARGB_8888_PRE:
            case OPENVG_lARGB_8888_PRE:
            case OPENVG_sBGRA_8888_PRE:
            case OPENVG_lBGRA_8888_PRE:
            case OPENVG_sABGR_8888_PRE:
            case OPENVG_lABGR_8888_PRE:
            case OPENVG_sRGBX_8888_PRE:
            case OPENVG_lRGBX_8888_PRE:
            case OPENVG_sRGB_565_PRE:
            case OPENVG_lRGB_565_PRE:
            case OPENVG_sRGBA_5551_PRE:
            case OPENVG_lRGBA_5551_PRE:
            case OPENVG_sRGBA_4444_PRE:
            case OPENVG_lRGBA_4444_PRE:
                buffer->premultiplied = 1;
                break;
            default:
                buffer->premultiplied = 0;
                break;
        };
#else
        /* Cannot support OpenVG VG_* format if HW does not support premultiply */
        return VG_LITE_INVALID_ARGUMENT;
#endif
    }
    else {
        /* All VG_LITE_* formats are not premultiplied */
        buffer->premultiplied = 0;
    }

    /* Reset planar. */
    buffer->yuv.uv_planar =
    buffer->yuv.v_planar =
    buffer->yuv.alpha_planar = 0;

    /* Align height in case format is tiled. */
    if ((buffer->format >= VG_LITE_YUY2 && buffer->format <= VG_LITE_NV16) || buffer->format == VG_LITE_NV24) {
        buffer->height = VG_LITE_ALIGN(buffer->height, 4);
        buffer->yuv.swizzle = VG_LITE_SWIZZLE_UV;
    }

    if ((buffer->format >= VG_LITE_YUY2_TILED && buffer->format <= VG_LITE_AYUY2_TILED) || buffer->format == VG_LITE_NV24_TILED) {
        buffer->height = VG_LITE_ALIGN(buffer->height, 4);
        buffer->tiled = VG_LITE_TILED;
        buffer->yuv.swizzle = VG_LITE_SWIZZLE_UV;
    }

    if ((buffer->format >= VG_LITE_ANV12 && buffer->format <= VG_LITE_ANV12_TILED
         && buffer->format != VG_LITE_AYUY2 && buffer->format != VG_LITE_YUY2_TILED) 
        || (buffer->format >= VG_LITE_NV24 && buffer->format <= VG_LITE_NV24_TILED)) {
        _allocate_tiled_yuv_planar(buffer);
    }
    else {
        /* Driver need compute the stride always with RT500 project. */

        vg_lite_float_t ratio = 1.0f;
        vg_lite_uint32_t mul, div, align;
        get_format_bytes(buffer->format, &mul, &div, &align);
        buffer->stride = buffer->width * mul / div;

#if gcFEATURE_VG_16PIXELS_ALIGNED
        vg_lite_int32_t tmp_align = 16 * mul / div;
        if ((mul / div) % 2 != 0) {
            if (buffer->stride % tmp_align != 0) {
                buffer->stride = (buffer->stride + tmp_align) / tmp_align * tmp_align;
            }
        }
        else {
            buffer->stride = VG_LITE_ALIGN(buffer->stride, tmp_align);
        }
#endif
        /* Allocate the buffer. */
        if (buffer->compress_mode)
            ratio = _calc_decnano_compress_ratio(buffer->format, buffer->compress_mode);
        if (ratio < 0)
            return VG_LITE_NOT_SUPPORT;
        allocate.bytes = (vg_lite_uint32_t)(buffer->stride * buffer->height * ratio);
        allocate.contiguous = 1;
        allocate.pool = (vg_lite_vidmem_pool_t)s_context.render_buffer_pool;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &allocate));

        /* Save the buffer allocation. */
        buffer->handle  = allocate.memory_handle;
        buffer->memory  = allocate.memory;
        buffer->address = allocate.memory_gpu;
        buffer->pool    = (vg_lite_memory_pool_t)allocate.pool;

        if ((buffer->format == VG_LITE_AYUY2) || (buffer->format == VG_LITE_AYUY2_TILED))
        {
            allocate.bytes = (buffer->stride / 2) * buffer->height;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &allocate));
            buffer->yuv.alpha_stride = buffer->stride;
            buffer->yuv.alpha_height = buffer->height;
            buffer->yuv.alpha_handle = allocate.memory_handle;
            buffer->yuv.alpha_memory = allocate.memory;
            buffer->yuv.alpha_planar = allocate.memory_gpu;
        }

        else if ((buffer->format >= VG_LITE_ABGR8565_PLANAR) && (buffer->format <= VG_LITE_RGBA5658_PLANAR))
        {
            allocate.bytes = (buffer->stride / 2) * buffer->height;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &allocate));
            buffer->yuv.alpha_stride = buffer->stride;
            buffer->yuv.alpha_height = buffer->height;
            buffer->yuv.alpha_handle = allocate.memory_handle;
            buffer->yuv.alpha_memory = allocate.memory;
            buffer->yuv.alpha_planar = allocate.memory_gpu;
#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
            if (buffer->sw24bit_buffer)
            {
                vg_lite_free(buffer->sw24bit_buffer);
                buffer->sw24bit_buffer = NULL;
            }

            buffer->sw24bit_buffer = (vg_lite_buffer_t*)vg_lite_os_malloc(sizeof(vg_lite_buffer_t));
            memcpy(buffer->sw24bit_buffer, buffer, sizeof(vg_lite_buffer_t));
            buffer->sw24bit_buffer->format = convert_24bit_format(buffer->format);
            memset(&(buffer->sw24bit_buffer->yuv), 0, sizeof(vg_lite_yuvinfo_t));
            buffer->sw24bit_buffer->sw24bit_buffer = NULL;
            vg_lite_allocate(buffer->sw24bit_buffer);
            buffer->sw24bit_buffer->sw24bit_planar_buffer = buffer;
#endif
        }
    }

    VG_LITE_TRACE_API("=>buffer: width=%d, height=%d, stride=%d, bytes=%d, format=%d\n",
        buffer->width, buffer->height, buffer->stride, allocate.bytes, buffer->format);

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_free(vg_lite_buffer_t * buffer)
{
    vg_lite_error_t error;
    vg_lite_kernel_free_t free, uv_free, v_free;
#if DUMP_CAPTURE
    vg_lite_float_t ratio = 1;
#endif

    DUMP_API_CALL(vg_lite_free, buffer);
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_FREE_API);
    VG_LITE_TRACE_API("vg_lite_free %p\n", buffer);

    VG_LITE_CHECK_NULL_POINTER(buffer);
    if (!(memcmp(s_context.rtbuffer,buffer,sizeof(vg_lite_buffer_t))) ) {
        VG_LITE_RETURN_ERROR(vg_lite_finish());

#if DUMP_CAPTURE
        vglitemDUMP("@[swap 0x%08X %dx%d +%u]",
            s_context.rtbuffer->address,
            s_context.rtbuffer->width, s_context.rtbuffer->height,
            s_context.rtbuffer->stride);

        if (buffer->compress_mode)
            ratio = _calc_decnano_compress_ratio(buffer->format, buffer->compress_mode);
        vglitemDUMP_BUFFER(
            "framebuffer",
            (size_t)s_context.rtbuffer->address,s_context.rtbuffer->memory,
            0,
            (size_t)(s_context.rtbuffer->stride*(s_context.rtbuffer->height)*ratio));
#endif

        memset(s_context.rtbuffer, 0, sizeof(vg_lite_buffer_t));
    }

#if !gcFEATURE_VG_LVGL_SUPPORT
    if (buffer->lvgl_buffer != NULL) {
        free.memory_handle = buffer->lvgl_buffer->handle;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free));
        vg_lite_os_free(buffer->lvgl_buffer);
        buffer->lvgl_buffer = NULL;
    }
#endif

#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((buffer->format >= VG_LITE_ABGR8565_PLANAR) && (buffer->format <= VG_LITE_RGBA5658_PLANAR))
    {
        if (buffer->sw24bit_buffer != NULL) {
            free.memory_handle = buffer->sw24bit_buffer->handle;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free));
            vg_lite_os_free(buffer->sw24bit_buffer);
            buffer->sw24bit_buffer = NULL;
        }
    }
#endif

#if gcFEATURE_VG_SIMPLE_BLT || gcFEATURE_VG_EXTERNAL_DMA_MESH
    if (buffer->mesh_buffer != NULL && buffer->mesh_buffer->handle != NULL) {
        VG_LITE_RETURN_ERROR(vg_lite_free(buffer->mesh_buffer));
        vg_lite_os_free(buffer->mesh_buffer);
        buffer->mesh_buffer = NULL;
    }
#endif

    if (buffer->yuv.uv_planar) {
        /* Free UV(U) planar buffer. */
#if DUMP_CAPTURE
        vglitemDUMP_BUFFER(
            "uv_plane",
            (size_t)buffer->yuv.uv_planar,buffer->yuv.uv_memory,
            0,
            buffer->yuv.uv_stride*buffer->yuv.uv_height);
#endif
        uv_free.memory_handle = buffer->yuv.uv_handle;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &uv_free));

        /* Mark the buffer as freed. */
        buffer->yuv.uv_handle = NULL;
        buffer->yuv.uv_memory = NULL;
    }

    if (buffer->yuv.v_planar) {
        /* Free V planar buffer. */
#if DUMP_CAPTURE
        vglitemDUMP_BUFFER(
            "v_plane",
            (size_t)buffer->yuv.v_planar,buffer->yuv.v_memory,
            0,
            buffer->yuv.v_stride*buffer->yuv.v_height);
#endif
        /* Free V planar buffer. */
        v_free.memory_handle = buffer->yuv.v_handle;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &v_free));

        /* Mark the buffer as freed. */
        buffer->yuv.v_handle = NULL;
        buffer->yuv.v_memory = NULL;
    }

    /* Make sure we have a valid memory handle. */
    if (buffer->handle == NULL) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    /* Free the buffer. */
    free.memory_handle = buffer->handle;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free));

    /* Mark the buffer as freed. */
    buffer->handle = NULL;
    buffer->memory = NULL;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_map(vg_lite_buffer_t* buffer, vg_lite_map_flag_t flag, vg_lite_int32_t fd)
{
    vg_lite_error_t error;
    vg_lite_kernel_map_t map, map1;

    DUMP_API_CALL(vg_lite_map, buffer, flag, fd);
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_MAP_API);
    VG_LITE_TRACE_API("vg_lite_map %p\n", buffer);

    /* We either need a logical or physical address. */
    if (buffer->memory == NULL && buffer->address == 0) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    if ((buffer->format >= VG_LITE_YUY2 && buffer->format <= VG_LITE_NV16) || buffer->format == VG_LITE_NV24) {
        if ((buffer->height % 4) != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
        buffer->yuv.swizzle = VG_LITE_SWIZZLE_UV;
    }
    if ((buffer->format >= VG_LITE_YUY2_TILED && buffer->format <= VG_LITE_AYUY2_TILED) || buffer->format == VG_LITE_NV24_TILED) {
        if ((buffer->height % 4) != 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
        buffer->tiled = VG_LITE_TILED;
        buffer->yuv.swizzle = VG_LITE_SWIZZLE_UV;
    }

    if ((buffer->format >= VG_LITE_ANV12 && buffer->format <= VG_LITE_ANV12_TILED
        && buffer->format != VG_LITE_AYUY2 && buffer->format != VG_LITE_YUY2_TILED)
        || (buffer->format >= VG_LITE_NV24 && buffer->format <= VG_LITE_NV24_TILED)) {

        if (buffer->yuv.uv_memory == NULL && buffer->yuv.uv_planar == 0 && buffer->yuv.v_memory == NULL && buffer->yuv.v_planar == 0
            && buffer->yuv.alpha_memory == NULL && buffer->yuv.alpha_planar == 0) {
            return VG_LITE_INVALID_ARGUMENT;
        }
        _map_yuv_planar(buffer, flag, fd);

    }
    else
    {
        /* Compute the stride. Align if necessary. */
        if (buffer->stride == 0) {
            vg_lite_uint32_t mul, div, align;
            get_format_bytes(buffer->format, &mul, &div, &align);
            buffer->stride = buffer->width * mul / div;
        }
        /* Map the buffer. */
        map.bytes = buffer->stride * buffer->height;
        map.logical = buffer->memory;
        map.physical = buffer->address;

        if (flag == VG_LITE_MAP_USER_MEMORY) {
            map.flags = VG_LITE_HAL_MAP_USER_MEMORY;
        }
        else if (flag == VG_LITE_MAP_DMABUF) {
            map.flags = VG_LITE_HAL_MAP_DMABUF;
        }
        else {
            return VG_LITE_INVALID_ARGUMENT;
        }

        map.dma_buf_fd = fd;

        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP, &map));

        /* Save the buffer allocation. */
        buffer->handle = map.memory_handle;
        buffer->address = map.memory_gpu;

        /* Map the alpha plane. */
        if ((buffer->format == VG_LITE_AYUY2) || (buffer->format == VG_LITE_AYUY2_TILED) || ((buffer->format >= VG_LITE_ABGR8565_PLANAR)
            && (buffer->format <= VG_LITE_RGBA5658_PLANAR))) {

            if (buffer->yuv.alpha_memory == NULL && buffer->yuv.alpha_planar == 0) {
                return VG_LITE_INVALID_ARGUMENT;
            }

            /* Compute the stride.*/
            if (buffer->yuv.alpha_stride == 0) {
                buffer->yuv.alpha_stride = buffer->stride / 2;
            }

            map1.bytes = buffer->yuv.alpha_stride * buffer->height;
            map1.logical = buffer->yuv.alpha_memory;
            map1.physical = buffer->yuv.alpha_planar;

            if (flag == VG_LITE_MAP_USER_MEMORY) {
                map1.flags = VG_LITE_HAL_MAP_USER_MEMORY;
            }
            else if (flag == VG_LITE_MAP_DMABUF) {
                map1.flags = VG_LITE_HAL_MAP_DMABUF;
            }
            else {
                return VG_LITE_INVALID_ARGUMENT;
            }

            map1.dma_buf_fd = fd;

            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP, &map1));

            /* Save the buffer allocation. */
            buffer->yuv.alpha_handle = map1.memory_handle;
            buffer->yuv.alpha_planar = map1.memory_gpu;
        }
    }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_unmap(vg_lite_buffer_t * buffer)
{
    DUMP_API_CALL(vg_lite_unmap, buffer);
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_UNMAP_API);
    VG_LITE_TRACE_API("vg_lite_unmap %p\n", buffer);

    vg_lite_error_t error;
    vg_lite_kernel_unmap_t unmap, uv_unmap, v_unmap, alpha_unmap;


    /* Make sure we have a valid memory handle. */
    if (buffer->handle == NULL) {
        return VG_LITE_INVALID_ARGUMENT;
    }
    
    /* Unmap the buffer. */
    unmap.memory_handle = buffer->handle;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP, &unmap));
    
    /* Mark the buffer as freed. */
    buffer->handle = NULL;

    /* Unmap the UV(U) planar buffer. */
    if (buffer->yuv.uv_handle)
    {
        uv_unmap.memory_handle = buffer->yuv.uv_handle;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP, &uv_unmap));
        buffer->yuv.uv_handle = NULL;
    }
    /* Unmap the V planar buffer. */
    if (buffer->yuv.v_handle)
    {
        v_unmap.memory_handle = buffer->yuv.v_handle;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP, &v_unmap));
        buffer->yuv.v_handle = NULL;
    }

    /* Unmap the alpha planar buffer. */
    if (buffer->yuv.alpha_handle)
    {
        alpha_unmap.memory_handle = buffer->yuv.alpha_handle;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP, &alpha_unmap));
        buffer->yuv.alpha_handle = NULL;
    }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_flush_mapped_buffer(vg_lite_buffer_t * buffer)
{
    DUMP_API_CALL(vg_lite_flush_mapped_buffer, buffer);
    VG_LITE_TRACE_API("vg_lite_flush_mapped_buffer %p\n", buffer);

    vg_lite_error_t error;
    vg_lite_kernel_cache_t cache;


    /* Make sure we have a valid memory handle. */
    if (buffer->handle == NULL) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    cache.memory_handle = buffer->handle;
    cache.cache_op = VG_LITE_CACHE_FLUSH;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_CACHE, &cache));

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_get_register(vg_lite_uint32_t address, vg_lite_uint32_t* result)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_GET_REGISTER_API);
    VG_LITE_TRACE_API("vg_lite_get_register 0x%08X %p\n", address, result);

    vg_lite_error_t error;
    vg_lite_kernel_info_t data;

    /* Get input register address. */
    data.addr = address;

    /* Get register info. */
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_CHECK, &data));

    /* Return register info. */
    *result = data.reg;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_get_info(vg_lite_info_t *info)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_GET_INFO_API);
    VG_LITE_TRACE_API("vg_lite_get_info %p\n", info);

    if (info != NULL)
    {
        info->api_version = VGLITE_API_VERSION_3_0;
        info->header_version = VGLITE_HEADER_VERSION;
        info->release_version = VGLITE_RELEASE_VERSION;
        info->reserved = 0;
    }

    return VG_LITE_SUCCESS;
}

vg_lite_uint32_t vg_lite_get_product_info(vg_lite_char* name, vg_lite_uint32_t* chip_id, vg_lite_uint32_t* chip_rev)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_GET_PRODUCT_INFO_API);
    VG_LITE_TRACE_API("vg_lite_get_product_info %p %p %p\n", name, chip_id, chip_rev);

    const vg_lite_char *product_name;
    vg_lite_uint32_t name_len;
    vg_lite_uint32_t rev = 0, id = 0;


    vg_lite_get_register(0x24, &rev);
    vg_lite_get_register(0x20, &id);

    if (id == 0x265 || id == 0x555)
        product_name = "GCNanoUltraV";
    else if (id == 0x255)
        product_name = "GCNanoLiteV";
    else if (id == 0x355)
        product_name = "GC355";
    else
        product_name = "Unknown";

    name_len = strlen(product_name) + 1;
    if (name != NULL)
    {
        memcpy(name, product_name, name_len);
    }
    
    if (chip_id != NULL)
    {
        *chip_id = id;
    }
    
    if (chip_rev != NULL)
    {
        *chip_rev = rev;
    }

    return name_len;
}

vg_lite_uint32_t vg_lite_query_feature(vg_lite_feature_t feature)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_QUERY_FEATURE_API);
    VG_LITE_TRACE_API("vg_lite_query_feature %d\n", feature);

    vg_lite_uint32_t result;


    if (feature < gcFEATURE_COUNT)
        result = s_ftable.ftable[feature];
    else
        result = 0;

    return result;
}

vg_lite_error_t vg_lite_finish()
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_FINISH_API);
    DUMP_API_CALL(vg_lite_finish);
    VG_LITE_TRACE_API("vg_lite_finish\n");

    vg_lite_error_t  error;


#if gcFEATURE_VG_SIMPLE_BLT || gcFEATURE_VG_EXTERNAL_DMA_MESH
    if (((s_context.mesh_mode == VG_LITE_MESH_COPY_INTERNAL) || (s_context.mesh_mode == VG_LITE_MESH_COPY_EXTERNAL))
        && s_context.frame_flag != VG_LITE_FRAME_END_FLAG) {
        return VG_LITE_SUCCESS;
    }
#endif

#if gcFEATURE_VG_FLEXA
    if (s_context.sync_mode == VG_LITE_MESH_COPY_INTERNAL && s_context.frame_flag != VG_LITE_FRAME_END_FLAG) {
        return VG_LITE_SUCCESS;
    }
#endif

    /* Return if there is nothing to submit. */
    if (CMDBUF_OFFSET(s_context) == 0)
    {
        if (submit_flag)
            VG_LITE_RETURN_ERROR(stall(&s_context, 0, (vg_lite_uint32_t)~0));
        s_context.frame_flag = VG_LITE_END_FLAG;
        return VG_LITE_SUCCESS;
    }

#if gcFEATURE_VG_MESH_FOR_FRAME
    if (s_context.mesh_mode && s_context.mesh_dirty) {
        printf("Excluding frame bound settings, merge to next submission\n");
        return VG_LITE_SUCCESS;
    }
#else
    /* Flush is moved from each draw to here. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000001));
#endif 

    VG_LITE_RETURN_ERROR(flush_target());

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
    s_context.fb_finish_flag = 1;
#endif

    VG_LITE_RETURN_ERROR(submit(&s_context));

#if gcFEATURE_VG_POWER_MANAGEMENT
    s_context.context.end_of_frame = 1;
#endif

#if defined(_WINDLL)
    VG_LITE_RETURN_ERROR(stall(&s_context, 0, (vg_lite_uint32_t)~0));
#elif defined(__linux__)
    VG_LITE_RETURN_ERROR(stall(&s_context, 20000, (vg_lite_uint32_t)~0));
#else
    VG_LITE_RETURN_ERROR(stall(&s_context, 5000, (vg_lite_uint32_t)~0));
#endif

#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((s_context.rtbuffer->format >= VG_LITE_ABGR8565) && (s_context.rtbuffer->format <= VG_LITE_RGBA5658))
    {
        if (s_context.rtbuffer->sw24bit_planar_buffer)
        {
            vg_lite_convert_planar(s_context.rtbuffer, s_context.rtbuffer->sw24bit_planar_buffer);
        }
    }
#endif

#if gcFEATURE_VG_SINGLE_COMMAND_BUFFER
    CMDBUF_OFFSET(s_context) = 0;
#else
    CMDBUF_SWAP(s_context);
    /* Reset command buffer. */
    CMDBUF_OFFSET(s_context) = 0;
#endif

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_flush(vg_lite_void)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_FLUSH_API);
    DUMP_API_CALL(vg_lite_flush);
    VG_LITE_TRACE_API("vg_lite_flush\n");

#if !gcFEATURE_VG_SINGLE_COMMAND_BUFFER
    vg_lite_error_t error;


#if gcFEATURE_VG_SIMPLE_BLT || gcFEATURE_VG_EXTERNAL_DMA_MESH
    if (((s_context.mesh_mode == VG_LITE_MESH_COPY_INTERNAL) || (s_context.mesh_mode == VG_LITE_MESH_COPY_EXTERNAL))
        && s_context.frame_flag != VG_LITE_FRAME_END_FLAG) {
        return VG_LITE_SUCCESS;
    }
#endif
#if gcFEATURE_VG_FLEXA
    if (s_context.sync_mode == VG_LITE_MESH_COPY_INTERNAL && s_context.frame_flag != VG_LITE_FRAME_END_FLAG) {
        return VG_LITE_SUCCESS;
    }
#endif

    /* Return if there is nothing to submit. */
    if (CMDBUF_OFFSET(s_context) == 0)
    {
        s_context.frame_flag = VG_LITE_END_FLAG;
        return VG_LITE_SUCCESS;
    }

    /* Wait if GPU has not completed previous CMD buffer */
    if (submit_flag)
    {
        VG_LITE_RETURN_ERROR(stall(&s_context, 0, (vg_lite_uint32_t)~0));
    }

#if gcFEATURE_VG_MESH_FOR_FRAME
    if (s_context.mesh_mode && s_context.mesh_dirty) {
        printf("Excluding frame bound settings, merge to next submission\n");
        return VG_LITE_SUCCESS;
    }
#else
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000001));
#endif

    /* Submit the current command buffer. */
    VG_LITE_RETURN_ERROR(flush_target());
    VG_LITE_RETURN_ERROR(submit(&s_context));
#if gcFEATURE_VG_POWER_MANAGEMENT
    s_context.context.end_of_frame = 1;
#endif

    CMDBUF_SWAP(s_context);

    /* Reset command buffer. */
    CMDBUF_OFFSET(s_context) = 0;

    return VG_LITE_SUCCESS;

#else
    printf("vg_lite_flush is not support when enable single command buffer!\n");
    return VG_LITE_NOT_SUPPORT;
#endif

}

vg_lite_error_t vg_lite_init_grad(vg_lite_linear_gradient_t *grad)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_INIT_GRAD_API);
    DUMP_API_CALL(vg_lite_init_grad, grad);
    VG_LITE_TRACE_API("vg_lite_init_grad %p\n", grad);

    vg_lite_error_t error = VG_LITE_SUCCESS;

    if (grad->memory != grad)
    {
        memset(grad, 0, sizeof(*grad));
        grad->memory = grad;
        vg_lite_identity(&grad->matrix);
    }
    grad->count = 0;

    /* Set the member values according to driver defaults. */
    grad->image.width = VLC_GRADIENT_BUFFER_WIDTH;
    grad->image.height = 1;
    grad->image.stride = 0;
    grad->image.format = VG_LITE_BGRA8888;
    
    /* Allocate the image for gradient. */
    VG_LITE_RETURN_ERROR(vg_lite_allocate(&grad->image));

    return error;
}

vg_lite_error_t vg_lite_set_linear_grad(vg_lite_ext_linear_gradient_t *grad,
                                 vg_lite_uint32_t count,
                                 vg_lite_color_ramp_t *color_ramp,
                                 vg_lite_linear_gradient_parameter_t linear_gradient,
                                 vg_lite_gradient_spreadmode_t spread_mode,
                                 vg_lite_uint8_t pre_multiplied)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_LINEAR_GRAD_API);
    VG_LITE_TRACE_API("vg_lite_set_linear_grad %p %d %p (%f %f %f %f) %d %d\n", grad, count, color_ramp,
        linear_gradient.X0, linear_gradient.X1, linear_gradient.Y0, linear_gradient.Y1, spread_mode, pre_multiplied);

    static vg_lite_color_ramp_t default_ramp[] =
    {
        {
            0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        },
        {
            1.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        }
    };

    vg_lite_uint32_t i, trg_count;
    vg_lite_float_t prev_stop;
    vg_lite_color_ramp_t *src_ramp = NULL;
    vg_lite_color_ramp_t *src_ramp_last = NULL;
    vg_lite_color_ramp_t *trg_ramp = NULL;

    /* Reset the count. */
    trg_count = 0;

    if ((linear_gradient.X0 == linear_gradient.X1) && (linear_gradient.Y0 == linear_gradient.Y1))
        return VG_LITE_INVALID_ARGUMENT;
    if (grad->memory != grad)
    {
        memset(grad, 0, sizeof(*grad));
        grad->memory = grad;
    }
    vg_lite_identity(&grad->matrix);
    grad->linear_grad = linear_gradient;
    grad->pre_multiplied = pre_multiplied;
    grad->spread_mode = spread_mode;

    if (count > 0 && count <= VLC_MAX_COLOR_RAMP_STOPS && color_ramp != NULL)
    {
        if (count > grad->m_count)
        {
            if (grad->color_ramp != NULL)
            {
                vg_lite_os_free(grad->color_ramp);
                grad->color_ramp = NULL;
            }
            grad->color_ramp = vg_lite_os_malloc(sizeof(vg_lite_color_ramp_t) * count);
            if (grad->color_ramp == NULL)
            {
                return VG_LITE_OUT_OF_MEMORY;
            }
            if (grad->converted_ramp != NULL)
            {
                vg_lite_os_free(grad->converted_ramp);
                grad->converted_ramp = NULL;
            }
            grad->converted_ramp = vg_lite_os_malloc(sizeof(vg_lite_color_ramp_t) * (count + 2));
            if (grad->converted_ramp == NULL)
            {
                return VG_LITE_OUT_OF_MEMORY;
            }
            grad->m_count = count;
        }
        else
        {
            if (grad->color_ramp != NULL)
                memset(grad->color_ramp, 0, sizeof(vg_lite_color_ramp_t) * grad->m_count);
            if (grad->converted_ramp != NULL)
                memset(grad->converted_ramp, 0, sizeof(vg_lite_color_ramp_t) * (grad->m_count + 2));
        }
    }
    else if (!count || count > VLC_MAX_COLOR_RAMP_STOPS || color_ramp == NULL)
    {

        if (grad->converted_ramp != NULL)
        {
            vg_lite_os_free(grad->converted_ramp);
            grad->converted_ramp = NULL;
        }
        grad->converted_ramp = vg_lite_os_malloc(sizeof(vg_lite_color_ramp_t) * 2);
        if (grad->converted_ramp == NULL)
        {
            return VG_LITE_OUT_OF_MEMORY;
        }
        goto Empty_sequence_handler;
    }

    grad->ramp_length = count;

    if (grad->color_ramp != NULL){
        for (i = 0; i < count; i++)
            grad->color_ramp[i] = color_ramp[i];

        /* Determine the last source ramp. */
        src_ramp_last = grad->color_ramp + grad->ramp_length;
    }

    /* Set the initial previous stop. */
    prev_stop = -1;

    /* Reset the count. */
    trg_count = 0;

    /* Walk through the source ramp. */
    for (
        src_ramp = grad->color_ramp, trg_ramp = grad->converted_ramp;
        src_ramp && trg_ramp && (src_ramp < src_ramp_last) && (trg_count < count + 2);
        src_ramp += 1
        )
    {
        /* Must be in increasing order. */
        if (src_ramp->stop < prev_stop)
        {
            /* Ignore the entire sequence. */
            trg_count = 0;
            break;
        }

        /* Update the previous stop value. */
        prev_stop = src_ramp->stop;

        /* Must be within [0..1] range. */
        if ((src_ramp->stop < 0.0f) || (src_ramp->stop > 1.0f))
        {
            /* Ignore. */
            continue;
        }

        /* Clamp color. */
        ClampColor(COLOR_FROM_RAMP(src_ramp),COLOR_FROM_RAMP(trg_ramp),0);

        /* First stop greater then zero? */
        if ((trg_count == 0) && (src_ramp->stop > 0.0f))
        {
            /* Force the first stop to 0.0f. */
            trg_ramp->stop = 0.0f;

            /* Replicate the entry. */
            trg_ramp[1] = *trg_ramp;
            trg_ramp[1].stop = src_ramp->stop;

            /* Advance. */
            trg_ramp  += 2;
            trg_count += 2;
        }
        else
        {
            /* Set the stop value. */
            trg_ramp->stop = src_ramp->stop;

            /* Advance. */
            trg_ramp  += 1;
            trg_count += 1;
        }
    }

    /* Empty sequence? */
    if (trg_count == 0 && grad->converted_ramp)
    {
        memcpy(grad->converted_ramp, default_ramp, sizeof(default_ramp));
        grad->converted_length = sizeof(default_ramp) / sizeof(vg_lite_color_ramp_t);
    }
    else
    {
        /* The last stop must be at 1.0. */
        if (trg_ramp && trg_ramp[-1].stop != 1.0f)
        {
            /* Replicate the last entry. */
            *trg_ramp = trg_ramp[-1];

            /* Force the last stop to 1.0f. */
            trg_ramp->stop = 1.0f;

            /* Update the final entry count. */
            trg_count += 1;
        }

        /* Set new length. */
        grad->converted_length = trg_count;
    }
    return VG_LITE_SUCCESS;

Empty_sequence_handler:
    memcpy(grad->converted_ramp, default_ramp, sizeof(default_ramp));
    grad->converted_length = sizeof(default_ramp) / sizeof(vg_lite_color_ramp_t);

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_update_linear_grad(vg_lite_ext_linear_gradient_t *grad)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_UPDATE_LINEAR_GRAD_API);
    DUMP_API_CALL(vg_lite_update_linear_grad, grad);
    VG_LITE_TRACE_API("vg_lite_update_linear_grad %p\n", grad);

    vg_lite_uint32_t ramp_length;
    vg_lite_color_ramp_t *color_ramp;
    vg_lite_uint32_t stop;
    vg_lite_uint32_t i, width;
    vg_lite_uint8_t* bits;
    vg_lite_float_t x0,y0,x1,y1,length,dx,dy;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t align, mul, div;


    /* Get shortcuts to the color ramp. */
    ramp_length = grad->converted_length;
    color_ramp       = grad->converted_ramp;

    x0 = grad->matrix.m[0][0] * grad->linear_grad.X0 + grad->matrix.m[0][1] * grad->linear_grad.Y0 + grad->matrix.m[0][2];
    y0 = grad->matrix.m[1][0] * grad->linear_grad.X0 + grad->matrix.m[1][1] * grad->linear_grad.Y0 + grad->matrix.m[1][2];
    x1 = grad->matrix.m[0][0] * grad->linear_grad.X1 + grad->matrix.m[0][1] * grad->linear_grad.Y1 + grad->matrix.m[0][2];
    y1 = grad->matrix.m[1][0] * grad->linear_grad.X1 + grad->matrix.m[1][1] * grad->linear_grad.Y1 + grad->matrix.m[1][2];
    dx = x1 - x0;
    dy = y1 - y0;
    length = (vg_lite_float_t)sqrt((vg_lite_double_t)dx * (vg_lite_double_t)dx + (vg_lite_double_t)dy * (vg_lite_double_t)dy);
    width = ramp_length * 128;

    if (length <= 0)
        return VG_LITE_INVALID_ARGUMENT;
    /* Find the common denominator of the color ramp stops. */

    /* Compute transform matrix from ramp surface to grad.*/
    vg_lite_identity(&(grad->matrix));
    vg_lite_translate(x0, y0, &(grad->matrix));
    vg_lite_rotate(
        ((dy >= 0) ? acosf(dx / length) : (2 * PI - acosf(dx / length))) * 180.f / PI,
        &(grad->matrix)
    );
    vg_lite_scale(length / width, 1.f, &(grad->matrix));

    /* Set grad to ramp surface. */
    grad->linear_grad.X0 = 0.f;
    grad->linear_grad.Y0 = 0.f;
    grad->linear_grad.X1 = (vg_lite_float_t)width;
    grad->linear_grad.Y1 = 0.f;

    /* Allocate the image for gradient. */
    VG_LITE_RETURN_ERROR(image_buffer_update(&grad->image, width));

    memset(grad->image.memory, 0, grad->image.stride * grad->image.height);
    /* Set pointer to color array. */
    bits = (vg_lite_uint8_t*)grad->image.memory;

    get_format_bytes(VG_LITE_ABGR8888, &mul, &div, &align);
    width = grad->image.stride * div / mul;

    /* Start filling the color array. */
    stop = 0;
    for (i = 0; i < width; ++i)
    {
        vg_lite_float_t gradient;
        vg_lite_float_t color[4];
        vg_lite_float_t color1[4];
        vg_lite_float_t color2[4];
        vg_lite_float_t weight;

        /* Compute gradient for current color array entry. */
        gradient = (vg_lite_float_t) i / (vg_lite_float_t) (width - 1);

        /* Find the entry in the color ramp that matches or exceeds this
        ** gradient. */
        while (gradient > color_ramp[stop].stop)
        {
            ++stop;
        }

        if (gradient == color_ramp[stop].stop)
        {
            /* Perfect match weight 1.0. */
            weight = 1.0f;

            /* Use color ramp color. */
            color1[3] = color_ramp[stop].alpha;
            color1[2] = color_ramp[stop].blue;
            color1[1] = color_ramp[stop].green;
            color1[0] = color_ramp[stop].red;

            color2[3] =
            color2[2] =
            color2[1] =
            color2[0] = 0.0f;
        }
        else
        {
            if(stop == 0){
                return VG_LITE_INVALID_ARGUMENT;
            }
            /* Compute weight. */
            weight = (color_ramp[stop].stop - gradient)
                    / (color_ramp[stop].stop - color_ramp[stop - 1].stop);

            /* Grab color ramp color of previous stop. */
            color1[3] = color_ramp[stop - 1].alpha;
            color1[2] = color_ramp[stop - 1].blue;
            color1[1] = color_ramp[stop - 1].green;
            color1[0] = color_ramp[stop - 1].red;

            /* Grab color ramp color of current stop. */
            color2[3] = color_ramp[stop].alpha;
            color2[2] = color_ramp[stop].blue;
            color2[1] = color_ramp[stop].green;
            color2[0] = color_ramp[stop].red;
        }

        if (grad->pre_multiplied)
        {
            /* Pre-multiply the first color. */
            color1[2] *= color1[3];
            color1[1] *= color1[3];
            color1[0] *= color1[3];

            /* Pre-multiply the second color. */
            color2[2] *= color2[3];
            color2[1] *= color2[3];
            color2[0] *= color2[3];
        }

        /* Filter the colors per channel. */
        color[3] = LERP(color1[3], color2[3], weight);
        color[2] = LERP(color1[2], color2[2], weight);
        color[1] = LERP(color1[1], color2[1], weight);
        color[0] = LERP(color1[0], color2[0], weight);

        /* Pack the final color. */
        *bits++ = PackColorComponent(color[3]);
        *bits++ = PackColorComponent(color[2]);
        *bits++ = PackColorComponent(color[1]);
        *bits++ = PackColorComponent(color[0]);
    }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_set_radial_grad(vg_lite_radial_gradient_t *grad,
                                 vg_lite_uint32_t count,
                                 vg_lite_color_ramp_t *color_ramp,
                                 vg_lite_radial_gradient_parameter_t radial_grad,
                                 vg_lite_gradient_spreadmode_t spread_mode,
                                 vg_lite_uint8_t pre_multiplied)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_RADIAL_GRAD_API);
    VG_LITE_TRACE_API("vg_lite_set_radial_grad %p %d %p (%f %f %f %f %f) %d %d\n", grad, count, color_ramp,
        radial_grad.cx, radial_grad.cy, radial_grad.fx, radial_grad.fy, radial_grad.r, spread_mode, pre_multiplied);

    static vg_lite_color_ramp_t defaultRamp[] =
    {
        {
            0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        },
        {
            1.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        }
    };

    vg_lite_uint32_t i, trgCount;
    vg_lite_float_t prevStop;
    vg_lite_color_ramp_t *srcRamp = NULL;
    vg_lite_color_ramp_t *srcRampLast = NULL;
    vg_lite_color_ramp_t *trgRamp = NULL;

    /* Reset the count. */
    trgCount = 0;

    if (radial_grad.r <= 0)
        return VG_LITE_INVALID_ARGUMENT;
    if (grad->memory != grad)
    {
        memset(grad, 0, sizeof(*grad));
        grad->memory = grad;
        vg_lite_identity(&grad->matrix);
    }

    grad->radial_grad = radial_grad;
    grad->pre_multiplied = pre_multiplied;
    grad->spread_mode = spread_mode;

    if (count > 0 && count <= VLC_MAX_COLOR_RAMP_STOPS && color_ramp != NULL)
    {
        if (count > grad->m_count)
        {
            if (grad->color_ramp != NULL)
            {
                vg_lite_os_free(grad->color_ramp);
                grad->color_ramp = NULL;
            }
            grad->color_ramp = vg_lite_os_malloc(sizeof(vg_lite_color_ramp_t) * count);
            if (grad->color_ramp == NULL)
            {
                return VG_LITE_OUT_OF_MEMORY;
            }
            if (grad->converted_ramp != NULL)
            {
                vg_lite_os_free(grad->converted_ramp);
                grad->converted_ramp = NULL;
            }
            grad->converted_ramp = vg_lite_os_malloc(sizeof(vg_lite_color_ramp_t) * (count + 2));
            if (grad->converted_ramp == NULL)
            {
                return VG_LITE_OUT_OF_MEMORY;
            }
            grad->m_count = count;
        }
        else
        {
            if (grad->color_ramp != NULL)
                memset(grad->color_ramp, 0, sizeof(vg_lite_color_ramp_t) * grad->m_count);
            if (grad->converted_ramp != NULL)
                memset(grad->converted_ramp, 0, sizeof(vg_lite_color_ramp_t) * (grad->m_count + 2));
        }
    }
    else if (!count || count > VLC_MAX_COLOR_RAMP_STOPS || color_ramp == NULL)
    {
        if (grad->converted_ramp != NULL)
        {
            vg_lite_os_free(grad->converted_ramp);
            grad->converted_ramp = NULL;
        }
        grad->converted_ramp = vg_lite_os_malloc(sizeof(vg_lite_color_ramp_t) * 2);
        if (grad->converted_ramp == NULL)
        {
            return VG_LITE_OUT_OF_MEMORY;
        }
        goto Empty_sequence_handler;
    }

    grad->ramp_length = count;

    if (grad->color_ramp){
        for (i = 0; i < count; i++)
            grad->color_ramp[i] = color_ramp[i];

        /* Determine the last source ramp. */
        srcRampLast = grad->color_ramp + grad->ramp_length;
    }

    /* Set the initial previous stop. */
    prevStop = -1;

    /* Reset the count. */
    trgCount = 0;

    /* Walk through the source ramp. */
    for (
        srcRamp = grad->color_ramp, trgRamp = grad->converted_ramp;
        srcRamp && trgRamp && (srcRamp < srcRampLast) && (trgCount < count + 2);
        srcRamp += 1
        )
    {
        /* Must be in increasing order. */
        if (srcRamp->stop < prevStop)
        {
            /* Ignore the entire sequence. */
            trgCount = 0;
            break;
        }

        /* Update the previous stop value. */
        prevStop = srcRamp->stop;

        /* Must be within [0..1] range. */
        if ((srcRamp->stop < 0.0f) || (srcRamp->stop > 1.0f))
        {
            /* Ignore. */
            continue;
        }

        /* Clamp color. */
        ClampColor(COLOR_FROM_RAMP(srcRamp),COLOR_FROM_RAMP(trgRamp),0);

        /* First stop greater then zero? */
        if ((trgCount == 0) && (srcRamp->stop > 0.0f))
        {
            /* Force the first stop to 0.0f. */
            trgRamp->stop = 0.0f;

            /* Replicate the entry. */
            trgRamp[1] = *trgRamp;
            trgRamp[1].stop = srcRamp->stop;

            /* Advance. */
            trgRamp  += 2;
            trgCount += 2;
        }
        else
        {
            /* Set the stop value. */
            trgRamp->stop = srcRamp->stop;

            /* Advance. */
            trgRamp  += 1;
            trgCount += 1;
        }
    }

    /* Empty sequence? */
    if (trgCount == 0 && grad->converted_ramp)
    {
        memcpy(grad->converted_ramp,defaultRamp,sizeof(defaultRamp));
        grad->converted_length = sizeof(defaultRamp) / sizeof(vg_lite_color_ramp_t);
    }
    else
    {
        /* The last stop must be at 1.0. */
        if (trgRamp && trgRamp[-1].stop != 1.0f)
        {
            /* Replicate the last entry. */
            *trgRamp = trgRamp[-1];

            /* Force the last stop to 1.0f. */
            trgRamp->stop = 1.0f;

            /* Update the final entry count. */
            trgCount += 1;
        }

        /* Set new length. */
        grad->converted_length = trgCount;
    }
    return VG_LITE_SUCCESS;

Empty_sequence_handler:
    memcpy(grad->converted_ramp,defaultRamp,sizeof(defaultRamp));
    grad->converted_length = sizeof(defaultRamp) / sizeof(vg_lite_color_ramp_t);

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_update_radial_grad(vg_lite_radial_gradient_t *grad)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_UPDATE_RADIAL_GRAD_API);
    DUMP_API_CALL(vg_lite_update_radial_grad, grad);
    VG_LITE_TRACE_API("vg_lite_update_radial_grad %p\n", grad);

    vg_lite_uint32_t ramp_length;
    vg_lite_color_ramp_t *colorRamp;
    vg_lite_uint32_t common, stop;
    vg_lite_uint32_t i, width;
    vg_lite_uint8_t* bits;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t align, mul, div;


    /* Get shortcuts to the color ramp. */
    ramp_length = grad->converted_length;
    colorRamp   = grad->converted_ramp;

    if (grad->radial_grad.r <= 0)
        return VG_LITE_INVALID_ARGUMENT;

    if (grad->radial_grad.r < 1)
    {
         common = 1;
         for (i = 0; i < ramp_length; ++i)
         {
             if (colorRamp[i].stop != 0.0f)
             {
                 vg_lite_float_t mul2 = common * colorRamp[i].stop;
                 vg_lite_float_t frac = mul2 - (vg_lite_float_t)floor(mul2);
                 if (frac > 0.00013f)    /* Suppose error for zero is 0.00013 */
                 {
                     common = MAX(common, (vg_lite_uint32_t)(1.0f / frac + 0.5f));
                 }
             }
         }

        /* Compute the width of the required color array. */
        width = common + 1;
        width = (width + 15) & (~0xf);
    }
    else
    {
        width = ramp_length * 128;
    }

    /* Allocate the image for gradient. */
    VG_LITE_RETURN_ERROR(image_buffer_update(&grad->image, width));

    get_format_bytes(VG_LITE_ABGR8888, &mul, &div, &align);
    width = grad->image.stride * div / mul;

    memset(grad->image.memory, 0, grad->image.stride * grad->image.height);
    /* Set pointer to color array. */
    bits = (vg_lite_uint8_t*)grad->image.memory;

    /* Start filling the color array. */
    stop = 0;
    for (i = 0; i < width; ++i)
    {
        vg_lite_float_t gradient;
        vg_lite_float_t color[4];
        vg_lite_float_t color1[4];
        vg_lite_float_t color2[4];
        vg_lite_float_t weight;

        /* Compute gradient for current color array entry. */
        gradient = (vg_lite_float_t) i / (vg_lite_float_t) (width - 1);

        /* Find the entry in the color ramp that matches or exceeds this
        ** gradient. */
        while (gradient > colorRamp[stop].stop)
        {
            ++stop;
        }

        if (gradient == colorRamp[stop].stop)
        {
            /* Perfect match weight 1.0. */
            weight = 1.0f;

            /* Use color ramp color. */
            color1[3] = colorRamp[stop].alpha;
            color1[2] = colorRamp[stop].blue;
            color1[1] = colorRamp[stop].green;
            color1[0] = colorRamp[stop].red;

            color2[3] =
            color2[2] =
            color2[1] =
            color2[0] = 0.0f;
        }
        else
        {
            /* Make sure stop stays within range */
            if (stop < 1 || stop >= ramp_length)
                return VG_LITE_INVALID_ARGUMENT;

            /* Compute weight. */
            weight = (colorRamp[stop].stop - gradient)
                    / (colorRamp[stop].stop - colorRamp[stop - 1].stop);

            /* Grab color ramp color of previous stop. */
            color1[3] = colorRamp[stop - 1].alpha;
            color1[2] = colorRamp[stop - 1].blue;
            color1[1] = colorRamp[stop - 1].green;
            color1[0] = colorRamp[stop - 1].red;

            /* Grab color ramp color of current stop. */
            color2[3] = colorRamp[stop].alpha;
            color2[2] = colorRamp[stop].blue;
            color2[1] = colorRamp[stop].green;
            color2[0] = colorRamp[stop].red;
        }

        if (grad->pre_multiplied)
        {
            /* Pre-multiply the first color. */
            color1[2] *= color1[3];
            color1[1] *= color1[3];
            color1[0] *= color1[3];

            /* Pre-multiply the second color. */
            color2[2] *= color2[3];
            color2[1] *= color2[3];
            color2[0] *= color2[3];
        }

        /* Filter the colors per channel. */
        color[3] = LERP(color1[3], color2[3], weight);
        color[2] = LERP(color1[2], color2[2], weight);
        color[1] = LERP(color1[1], color2[1], weight);
        color[0] = LERP(color1[0], color2[0], weight);

        /* Pack the final color. */
        *bits++ = PackColorComponent(color[3]);
        *bits++ = PackColorComponent(color[2]);
        *bits++ = PackColorComponent(color[1]);
        *bits++ = PackColorComponent(color[0]);
    }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_set_grad(vg_lite_linear_gradient_t *grad,
                                 vg_lite_uint32_t count,
                                 vg_lite_uint32_t *colors,
                                 vg_lite_uint32_t *stops)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_GRAD_API);
    VG_LITE_TRACE_API("vg_lite_set_grad %p %d %p %p\n", grad, count, colors, stops);

    vg_lite_uint32_t i;


    grad->count = 0;    /* Opaque B&W gradient */
    if (count > 0 && count <= VLC_MAX_GRADIENT_STOPS && colors != NULL && stops != NULL)
    {
        if (count > grad->m_count)
        {
            if (grad->colors != NULL)
            {
                vg_lite_os_free(grad->colors);
                grad->colors = NULL;
            }
            grad->colors = vg_lite_os_malloc(sizeof(vg_lite_uint32_t) * count);
            if (grad->colors == NULL)
            {
                return VG_LITE_OUT_OF_MEMORY;
            }
            if (grad->stops != NULL)
            {
                vg_lite_os_free(grad->stops);
                grad->stops = NULL;
            }
            grad->stops = vg_lite_os_malloc(sizeof(vg_lite_uint32_t) * count);
            if (grad->stops == NULL)
            {
                return VG_LITE_OUT_OF_MEMORY;
            }
            memset(grad->stops, 0, sizeof(vg_lite_uint32_t) * count);
            grad->m_count = count;
        }
        else
        {
            if (grad->colors != NULL)
                memset(grad->colors, 0, sizeof(vg_lite_uint32_t) * grad->m_count);

            if (grad->stops != NULL)
                memset(grad->stops, 0, sizeof(vg_lite_uint32_t) * grad->m_count);
        }
    }
    else if (!count || count > VLC_MAX_GRADIENT_STOPS || colors == NULL || stops == NULL)
    {
        grad->colors = NULL;
        grad->colors = vg_lite_os_malloc(sizeof(vg_lite_uint32_t) * 2);
        if (grad->colors == NULL)
        {
            return VG_LITE_OUT_OF_MEMORY;
        }
        grad->stops = NULL;
        grad->stops = vg_lite_os_malloc(sizeof(vg_lite_uint32_t) * 2);
        if (grad->stops == NULL)
        {
            return VG_LITE_OUT_OF_MEMORY;
        }
        return VG_LITE_SUCCESS;
    }

    /* Check stops validity */
    for (i = 0; i < count; i++)
        if (grad->colors && grad->stops && stops[i] < VLC_GRADIENT_BUFFER_WIDTH) {
            if (!grad->count || stops[i] > grad->stops[grad->count - 1])
            {
                grad->stops[grad->count] = stops[i];
                grad->colors[grad->count] = colors[i];
                grad->count++;
            } else if (stops[i] == grad->stops[grad->count - 1]) {
                /* Equal stops : use the color corresponding to the last stop
                in the sequence */
                grad->colors[grad->count - 1] = colors[i];
            }
        }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_update_grad(vg_lite_linear_gradient_t *grad)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_UPDATE_GRAD_API);
    DUMP_API_CALL(vg_lite_update_grad, grad);
    VG_LITE_TRACE_API("vg_lite_update_grad %p\n", grad);

    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_int32_t r0, g0, b0, a0;
    vg_lite_int32_t r1, g1, b1, a1;
    vg_lite_int32_t lr, lg, lb, la;
    vg_lite_uint32_t i;
    vg_lite_int32_t j;
    vg_lite_int32_t ds, dr, dg, db, da;
    vg_lite_uint32_t *buffer = (vg_lite_uint32_t *)grad->image.memory;


    if (grad->count == 0) {
        /* If no valid stops have been specified (e.g., due to an empty input
        * array, out-of-range, or out-of-order stops), a stop at 0 with color
        * 0xFF000000 (opaque black) and a stop at 255 with color 0xFFFFFFFF
        * (opaque white) are implicitly defined. */
        grad->stops[0] = 0;
        grad->colors[0] = 0xFF000000;   /* Opaque black */
        grad->stops[1] = 255;
        grad->colors[1] = 0xFFFFFFFF;   /* Opaque white */
        grad->count = 2;
    } else if (grad->stops[0] != 0) {
        /* If at least one valid stop has been specified, but none has been
        * defined with an offset of 0, an implicit stop is added with an
        * offset of 0 and the same color as the first user-defined stop. */
        for (i = 0; i < grad->stops[0]; i++)
            buffer[i] = grad->colors[0];
    }
    a0 = A(grad->colors[0]);
    r0 = R(grad->colors[0]);
    g0 = G(grad->colors[0]);
    b0 = B(grad->colors[0]);

    /* Calculate the colors for each pixel of the image. */
    for (i = 0; i < grad->count - 1; i++) {
        buffer[grad->stops[i]] = grad->colors[i];
        ds = grad->stops[i + 1] - grad->stops[i];
        a1 = A(grad->colors[i + 1]);
        r1 = R(grad->colors[i + 1]);
        g1 = G(grad->colors[i + 1]);
        b1 = B(grad->colors[i + 1]);

        da = a1 - a0;
        dr = r1 - r0;
        dg = g1 - g0;
        db = b1 - b0;

        for (j = 1; j < ds; j++) {
            la = a0 + da * j / ds;
            lr = r0 + dr * j / ds;
            lg = g0 + dg * j / ds;
            lb = b0 + db * j / ds;

            buffer[grad->stops[i] + j] = ARGB(la, lr, lg, lb);
        }

        a0 = a1;
        r0 = r1;
        g0 = g1;
        b0 = b1;
    }

    /* If at least one valid stop has been specified, but none has been defined
    * with an offset of 255, an implicit stop is added with an offset of 255
    * and the same color as the last user-defined stop. */
    for (i = grad->stops[grad->count - 1]; i < VLC_GRADIENT_BUFFER_WIDTH; i++)
        buffer[i] = grad->colors[grad->count - 1];

    return error;
}

vg_lite_error_t vg_lite_clear_linear_grad(vg_lite_ext_linear_gradient_t *grad)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_CLEAR_LINEAR_GRAD_API);
    DUMP_API_CALL(vg_lite_clear_linear_grad, grad);
    VG_LITE_TRACE_API("vg_lite_clear_linear_grad %p\n", grad);

    vg_lite_error_t error = VG_LITE_SUCCESS;


    grad->count = 0;
    grad->m_count = 0;
    grad->memory = NULL;
    /* Release the image resource. */
    if (grad->image.handle != NULL)
    {
        error = vg_lite_free(&grad->image);
    }
    if (grad->color_ramp != NULL)
    {
        vg_lite_os_free(grad->color_ramp);
        grad->color_ramp = NULL;
    }
    if (grad->converted_ramp != NULL)
    {
        vg_lite_os_free(grad->converted_ramp);
        grad->converted_ramp = NULL;
    }

    return error;
}

vg_lite_error_t vg_lite_clear_grad(vg_lite_linear_gradient_t *grad)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_CLEAR_GRAD_API);
    DUMP_API_CALL(vg_lite_clear_grad, grad);
    VG_LITE_TRACE_API("vg_lite_clear_grad %p\n", grad);

    vg_lite_error_t error = VG_LITE_SUCCESS;


    grad->count = 0;
    grad->m_count = 0;
    grad->memory = NULL;
    /* Release the image resource. */
    if (grad->image.handle != NULL)
    {
        error = vg_lite_free(&grad->image);
    }

    if (grad->colors != NULL)
    {
        vg_lite_os_free(grad->colors);
        grad->colors = NULL;
    }
    if (grad->stops != NULL)
    {
        vg_lite_os_free(grad->stops);
        grad->stops = NULL;
    }
    return error;
}

vg_lite_error_t vg_lite_clear_radial_grad(vg_lite_radial_gradient_t *grad)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_CLEAR_RADIAL_GRAD_API);
    DUMP_API_CALL(vg_lite_clear_radial_grad, grad);
    VG_LITE_TRACE_API("vg_lite_clear_radial_grad %p\n", grad);

    vg_lite_error_t error = VG_LITE_SUCCESS;


    grad->count = 0;
    grad->m_count = 0;
    grad->memory = NULL;
    /* Release the image resource. */
    if (grad->image.handle != NULL)
    {
        error = vg_lite_free(&grad->image);
    }
    if (grad->color_ramp != NULL)
    {
        vg_lite_os_free(grad->color_ramp);
        grad->color_ramp = NULL;
    }
    if (grad->converted_ramp != NULL)
    {
        vg_lite_os_free(grad->converted_ramp);
        grad->converted_ramp = NULL;
    }

    return error;
}

vg_lite_matrix_t * vg_lite_get_linear_grad_matrix(vg_lite_ext_linear_gradient_t *grad)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_GET_LINEAR_GRAD_MATRIX_API);
    VG_LITE_TRACE_API("vg_lite_get_linear_grad_matrix %p\n", grad);

    return &grad->matrix;
}

vg_lite_matrix_t * vg_lite_get_grad_matrix(vg_lite_linear_gradient_t *grad)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_GET_GRAD_MATRIX_API);
    VG_LITE_TRACE_API("vg_lite_get_grad_matrix %p\n", grad);

    return &grad->matrix;
}

vg_lite_matrix_t * vg_lite_get_radial_grad_matrix(vg_lite_radial_gradient_t *grad)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_GET_RADIAL_GRAD_MATRIX_API);
    VG_LITE_TRACE_API("vg_lite_get_radial_grad_matrix %p\n", grad);

    return &grad->matrix;
}

vg_lite_error_t vg_lite_dump_command_buffer()
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DUMP_COMMAND_BUFFER_API);
    DUMP_API_CALL(vg_lite_dump_command_buffer);
    vg_lite_error_t error = VG_LITE_SUCCESS;
#if DUMP_CAPTURE
    vg_lite_kernel_submit_t submit;
    vg_lite_context_t* context = &s_context;

    /* Submit the command buffer. */
    submit.context = &context->context;
    submit.commands = CMDBUF_BUFFER(*context);
    submit.command_size = CMDBUF_OFFSET(*context);
    submit.command_id = CMDBUF_INDEX(*context);

    vglitemDUMP_BUFFER("command", (size_t)CMDBUF_BUFFER(*context),
        submit.context->command_buffer_logical[CMDBUF_INDEX(*context)], 0, submit.command_size);
    vglitemDUMP("@[commit]");
#else
    printf("Please enable DUMP_CAPTURE to use vg_lite_dump_command_buffer().\n");
#endif

    return error;
}

vg_lite_error_t vg_lite_get_parameter(vg_lite_param_type_t type,
                                      vg_lite_int32_t count,
                                      vg_lite_pointer params)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_GET_PARAMETER_API);
    VG_LITE_TRACE_API("vg_lite_get_parameter %d %p\n", count, params);

    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t gpu_idle = 0;
    vg_lite_uint32_t chip_id = 0;
    vg_lite_int32_t *iparams;
    vg_lite_uint32_t *uiparams;
    vg_lite_buffer_t* buf;
    vg_lite_kernel_hardware_running_time_t time;


    switch (type)
    {
    case VG_LITE_GPU_IDLE_STATE:
        if (count != 1)
            return VG_LITE_INVALID_ARGUMENT;
        
        if(CMDBUF_OFFSET(s_context) > ((CMDBUF_SIZE(s_context)) >> 1)){
            error = vg_lite_flush();
            if (error != VG_LITE_SUCCESS) {
                return error;
            }
        }
        vg_lite_get_register(0x04, &gpu_idle);
        uiparams = (vg_lite_uint32_t *)params;
        *uiparams = ((gpu_idle & 0x0B05) == 0x0B05);
        break;
    case VG_LITE_HW_ID:
        if (count != 1)
            return VG_LITE_INVALID_ARGUMENT;

        vg_lite_get_product_info(NULL, &chip_id, NULL);
        uiparams = (vg_lite_uint32_t*)params;
        *uiparams = chip_id;
        break;

    case VG_LITE_SCISSOR_RECT:
        if ((count % 4) != 0)
            return VG_LITE_INVALID_ARGUMENT;

        iparams = (vg_lite_int32_t *)params;

        for (vg_lite_int32_t i = 0; i < count; i++)
            *(iparams + i) = (vg_lite_int32_t)s_context.scissor[i];

        break;

    case VG_LITE_HARDWARE_RUNNING_TIME:
        vg_lite_kernel(VG_LITE_RECORD_RUNNING_TIME, &time);
        *((vg_lite_float_t*)params) = (vg_lite_float_t)time.run_time / 1000.0f;
        break;

    case VG_LITE_SRC_BUF_ALIGNED_CHECK:
        buf = ((vg_lite_buffer_t*)params);
#if gcFEATURE_VG_16PIXELS_ALIGNED       
        if (_check_source_aligned(buf->format, buf->stride))
            return VG_LITE_NOT_ALIGNED;
#endif
        if (srcbuf_align_check(buf))
            return VG_LITE_NOT_ALIGNED;
        if (feature_check_compress(buf->format, buf->compress_mode, buf->tiled, buf->width, buf->height))
            return VG_LITE_NOT_ALIGNED;
        if ((buf->format == VG_LITE_RGBA8888_ETC2_EAC || buf->format == VG_LITE_RGB888_ETC2_EAC) && (buf->width % 4 || buf->height % 4)) {
            return VG_LITE_NOT_ALIGNED;
        }
        break;

    case VG_LITE_DST_BUF_ALIGNED_CHECK:
        buf = ((vg_lite_buffer_t*)params);
        if (dstbuf_align_check(buf))
            return VG_LITE_NOT_ALIGNED;
        if (feature_check_compress(buf->format, buf->compress_mode, buf->tiled, buf->width, buf->height))
            return VG_LITE_NOT_ALIGNED;
        if (peclear_align_check(buf, buf->height)) {
            printf("Not aligned for peclear.\n");
            return VG_LITE_NOT_ALIGNED;
        }
            
        break;

    default:
        error = VG_LITE_INVALID_ARGUMENT;
        break;
    }

    return error;
}

vg_lite_error_t vg_lite_copy_image(vg_lite_buffer_t *target, vg_lite_buffer_t *source,
                                vg_lite_int32_t dx, vg_lite_int32_t dy,
                                vg_lite_int32_t sx, vg_lite_int32_t sy,
                                vg_lite_uint32_t width, vg_lite_uint32_t height)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_COPY_IMAGE_API);
    VG_LITE_TRACE_API("vg_lite_copy_image %p %p %d %d %d %d %d %d\n", target, source, sx, sy, dx, dy, width, height);

#if gcFEATURE_VG_IM_INPUT
    vg_lite_error_t error;
    vg_lite_point_t point_min, point_max, temp;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_matrix_t n;
    vg_lite_float_t x_step[3];
    vg_lite_float_t y_step[3];
    vg_lite_float_t c_step[3];
    vg_lite_uint32_t imageMode = 0;
    vg_lite_uint32_t in_premult = 0;
    vg_lite_int32_t stride;
    vg_lite_uint32_t transparency_mode = 0;
    vg_lite_uint32_t filter_mode = 0;
    vg_lite_uint32_t conversion = 0;
    vg_lite_uint32_t tiled_source;
    vg_lite_int32_t left, top, right, bottom;
    vg_lite_uint32_t rect_x = 0, rect_y = 0, rect_w = 0, rect_h = 0;
    vg_lite_rectangle_t rectangle = { sx, sy, width, height };
    vg_lite_uint32_t yuv2rgb = 0;
    vg_lite_uint32_t uv_swiz = 0;
    vg_lite_uint32_t compress_mode;
    vg_lite_uint32_t src_premultiply_enable = 0;
    vg_lite_uint32_t index_endian = 0;
    vg_lite_uint32_t eco_fifo = 0;
    vg_lite_uint32_t tile_setting = 0;
    vg_lite_uint32_t stripe_mode = 0;
    vg_lite_uint32_t prediv_flag = 0;
    vg_lite_color_t color = 0;
#if gcFEATURE_VG_NEW_FACTOR
    vg_factor_config_t factor_config;
    factor_config.factor_src_alpha = 0x0;
    factor_config.factor_src_color = 0x0;
    factor_config.factor_dst_alpha = 0x0;
    factor_config.factor_dst_color = 0x0;
    factor_config.final_equation_opcode = 0x0;
    factor_config.dstchannelmode = 0x0;
    factor_config.srcchannelmode = 0x0;
#endif
#if DUMP_CAPTURE
    vg_lite_float_t ratio = 1;
#endif
#if gcFEATURE_VG_FLEXA
    if (s_context.sync_mode)
    {
        printf("When Flexa is enabled vg_lite_copy_image is not support.\n");
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_RETURN_ERROR(feature_check_source_index_endian(source->format, source->index_endian));
    VG_LITE_RETURN_ERROR(feature_check_target_rectangle_tiled_out(target->tiled));
    VG_LITE_RETURN_ERROR(feature_check_source_rgba8888_etc2_eac(source->format, source->width, source->height));
    VG_LITE_RETURN_ERROR(feature_check_source_rgb888_etc2_eac(source->format, source->width, source->height));
    VG_LITE_RETURN_ERROR(feature_check_source_packed_yuy_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_planar_yuv_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_planar_nv24_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_ayuv_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_source_yuv_tiled_input(source->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(source->format));
    VG_LITE_RETURN_ERROR(feature_check_24bit_planar_format(source->format));
    VG_LITE_RETURN_ERROR(feature_check_im_dec_input_compress(source->compress_mode));
    VG_LITE_RETURN_ERROR(feature_check_stencil_image_mode(source->image_mode));
    VG_LITE_RETURN_ERROR(feature_check_lvgl_recolor_image_mode(source->image_mode));
    VG_LITE_RETURN_ERROR(chip_check_target_format(target->format));
    VG_LITE_RETURN_ERROR(chip_check_source_format(source->format));
    VG_LITE_RETURN_ERROR(feature_check_a124_a8l8_target_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_a124_a8l8_source_format(source->format));
    VG_LITE_RETURN_ERROR(srcbuf_align_check(source));
    VG_LITE_RETURN_ERROR(feature_check_compress(source->format, source->compress_mode, source->tiled, source->width, source->height));
#endif /* gcFEATURE_VG_ERROR_CHECK */

    chip_get_source_index_endian_bits(source->format, source->index_endian, &index_endian);
#if !gcFEATURE_VG_STRIPE_MODE_DISABLE
    /* Enable fifo feature to share buffer between vg and ts to improve the rotation performance */
    eco_fifo = 1 << 7;
#endif

    transparency_mode = (source->transparency_mode == VG_LITE_IMAGE_TRANSPARENT ? 0x8000 : 0);

    vg_lite_matrix_t* matrix = &n;
    vg_lite_identity(matrix);
    vg_lite_translate((vg_lite_float_t)dx, (vg_lite_float_t)dy, matrix);

    conversion = feature_a124_a8l8_l8_conversion(target->format, source->format);

#if gcFEATURE_VG_16PIXELS_ALIGNED
    /* Check if source specify bytes are aligned */
    error = _check_source_aligned(source->format, source->stride);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }
#endif

    /* Set source region. */
    vg_lite_rectangle_t* rect = &rectangle;
    rect_x = (rect->x < 0) ? 0 : rect->x;
    rect_y = (rect->y < 0) ? 0 : rect->y;
    rect_w = rect->width;
    rect_h = rect->height;
    if ((rect_x > (vg_lite_uint32_t)source->width) || (rect_y > (vg_lite_uint32_t)source->height) ||
        (rect_w == 0) || (rect_h == 0))
    {
        /*No intersection*/
        return VG_LITE_INVALID_ARGUMENT;
    }
    if (rect_x + rect_w > (vg_lite_uint32_t)source->width)
    {
        rect_w = source->width - rect_x;
    }
    if (rect_y + rect_h > (vg_lite_uint32_t)source->height)
    {
        rect_h = source->height - rect_y;
    }

    /* Transform image (0,0) to screen. */
    if (!transform(&temp, 0.0f, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;

    /* Set initial point. */
    point_min = temp;
    point_max = temp;

    /* Transform image (0,height) to screen. */
    if (!transform(&temp, 0.0f, (vg_lite_float_t)rect_h, matrix))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

    /* Transform image (width,height) to screen. */
    if (!transform(&temp, (vg_lite_float_t)rect_w, (vg_lite_float_t)rect_h, matrix))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

    /* Transform image (width,0) to screen. */
    if (!transform(&temp, (vg_lite_float_t)rect_w, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

    /* Clip to target. */
    if (s_context.scissor_set && !target->scissor_buffer) {
        left   = s_context.scissor[0];
        top    = s_context.scissor[1];
        right  = s_context.scissor[2];
        bottom = s_context.scissor[3];
    }
    else {
        left   = 0;
        top    = 0;
        right  = target->width;
        bottom = target->height;
    }

    point_min.x = MAX(point_min.x, left);
    point_min.y = MAX(point_min.y, top);
    point_max.x = MIN(point_max.x, right);
    point_max.y = MIN(point_max.y, bottom);

    /* No need to draw. */
    if ((point_max.x <= point_min.x) || (point_max.y <= point_min.y)) {
        return VG_LITE_SUCCESS;
    }

#if gcFEATURE_VG_GAMMA
    get_st_gamma_src_dest(source, target);
#endif

    /*blend input into context*/
    in_premult = 0x00000000;

    /* Adjust premultiply setting according to openvg condition */
    src_premultiply_enable = 0x01000100;
#if gcFEATURE_VG_PIXEL_MATRIX
    if (s_context.matrix_enable == 0 && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
#else
    if (s_context.color_transform == 0 && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
#endif
    else {
        prediv_flag = 1;
    }

    if ((source->premultiplied == 0 && target->premultiplied == 0)
        || (source->premultiplied == 1 && target->premultiplied == 0 && prediv_flag == 0)) {
        src_premultiply_enable = 0x01000100;
        in_premult = 0x10000000;
    }
    /* when src and dst all pre format, im pre_out set to 0 to perform data truncation to prevent data overflow */
    else if (source->premultiplied == 1 && target->premultiplied == 1 && prediv_flag == 0) {
        src_premultiply_enable = 0x01000100;
        in_premult = 0x10000000;
    }
    else if (source->premultiplied == 0 && target->premultiplied == 1) {
        src_premultiply_enable = 0x01000100;
        in_premult = 0x00000000;
    }
    else if ((source->premultiplied == 1 && target->premultiplied == 1 && prediv_flag == 1) ||
        (source->premultiplied == 1 && target->premultiplied == 0 && prediv_flag == 1)) {
        src_premultiply_enable = 0x00000100;
        in_premult = 0x00000000;
    }
    if (source->premultiplied == target->premultiplied) {
        target->apply_premult = 1;
    }
    else {
        target->apply_premult = 0;
    }

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix))
        return VG_LITE_SUCCESS;

#if gcFEATURE_VG_MATH_PRECISION_FIX_DISABLE

    /* Compute interpolation steps. */
    x_step[0] = inverse_matrix.m[0][0];
    x_step[1] = inverse_matrix.m[1][0];
    x_step[2] = inverse_matrix.m[2][0];
    y_step[0] = inverse_matrix.m[0][1];
    y_step[1] = inverse_matrix.m[1][1];
    y_step[2] = inverse_matrix.m[2][1];
    c_step[0] = (0.5f * (inverse_matrix.m[0][0] + inverse_matrix.m[0][1]) + inverse_matrix.m[0][2]);
    c_step[1] = (0.5f * (inverse_matrix.m[1][0] + inverse_matrix.m[1][1]) + inverse_matrix.m[1][2]);
    c_step[2] = 0.5f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[2][2];

#else

    /* Compute interpolation steps. */
    x_step[0] = inverse_matrix.m[0][0] / rect_w;
    x_step[1] = inverse_matrix.m[1][0] / rect_h;
    x_step[2] = inverse_matrix.m[2][0];
    y_step[0] = inverse_matrix.m[0][1] / rect_w;
    y_step[1] = inverse_matrix.m[1][1] / rect_h;
    y_step[2] = inverse_matrix.m[2][1];
    c_step[0] = (0.5f * (inverse_matrix.m[0][0] + inverse_matrix.m[0][1]) + inverse_matrix.m[0][2]) / rect_w;
    c_step[1] = (0.5f * (inverse_matrix.m[1][0] + inverse_matrix.m[1][1]) + inverse_matrix.m[1][2]) / rect_h;
    c_step[2] = 0.5f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[2][2];

#endif

    /* Determine image mode (NORMAL) depending on the color. */
    imageMode = 0x00001000;

    tiled_source = (source->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0;

#if gcFEATURE_VG_RECTANGLE_TILED_OUT
    if (target->tiled == VG_LITE_TILED) {
        tile_setting = 0x40;
        stripe_mode = 0x20000000;
    }
#endif
    compress_mode = (vg_lite_uint32_t)source->compress_mode << 25;

#if gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF6, factor_config.srcchannelmode | factor_config.dstchannelmode));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0xAF8, factor_config.factor_src_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0xAF9, factor_config.factor_src_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0xAFA, factor_config.factor_dst_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0xAFB, factor_config.factor_dst_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0xAF7, factor_config.final_equation_opcode));
#endif

    /* Setup the command buffer. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000001 | in_premult | imageMode | transparency_mode | tile_setting | eco_fifo | s_context.scissor_enable | stripe_mode));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A18, (vg_lite_pointer)&c_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A19, (vg_lite_pointer)&c_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1A, (vg_lite_pointer)&c_step[2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1C, (vg_lite_pointer)&x_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1D, (vg_lite_pointer)&x_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1E, (vg_lite_pointer)&x_step[2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1F, 0x00000001));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A20, (vg_lite_pointer)&y_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A21, (vg_lite_pointer)&y_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A22, (vg_lite_pointer)&y_step[2]));

    if (((source->format >= VG_LITE_YUY2) &&
        (source->format <= VG_LITE_AYUY2)) ||
        ((source->format >= VG_LITE_YUY2_TILED) &&
            (source->format <= VG_LITE_AYUY2_TILED))) {
        yuv2rgb = convert_yuv2rgb(source->yuv.yuv2rgb);
        uv_swiz = convert_uv_swizzle(source->yuv.swizzle);
    }

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A25, convert_source_format(source->format) | filter_mode | uv_swiz | yuv2rgb | conversion | compress_mode | src_premultiply_enable | index_endian));
    if (source->yuv.uv_planar) {
        /* Program u plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A51, source->yuv.uv_planar));
    }
    if (source->yuv.v_planar) {
        /* Program v plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source->yuv.v_planar));
    }

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A27, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source->address));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));
    /* 24bit format stride configured to 4bpp. */
    if (source->format >= VG_LITE_RGB888 && source->format <= VG_LITE_RGBA5658) {
        stride = source->stride / 3 * 4;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, stride | tiled_source));
    }
    else {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, source->stride | tiled_source));
    }

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2D, rect_x | (rect_y << 16)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2F, rect_w | (rect_h << 16)));
    VG_LITE_RETURN_ERROR(push_rectangle(&s_context, point_min.x, point_min.y, point_max.x - point_min.x, point_max.y - point_min.y));

#if !gcFEATURE_VG_STRIPE_MODE_DISABLE
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0E02, 0x10 | (0x7 << 8)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0F00, 0x10 | (0x7 << 8)));
#endif

    VG_LITE_BREAK_ERROR(push_state(&s_context, 0x0A1B, 0x00000011));
#if DUMP_CAPTURE
    if (source->compress_mode)
        ratio = _calc_decnano_compress_ratio(source->format, source->compress_mode);
    vglitemDUMP_BUFFER("image", (size_t)source->address, source->memory, 0, (size_t)((source->stride)*(source->height)*ratio));
#endif
#if DUMP_IMAGE
    dump_img(source->memory, source->width, source->height, source->format);
#endif

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_set_memory_pool(vg_lite_buffer_type_t type, vg_lite_memory_pool_t pool)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_MEMORY_POOL_API);

    if (!(pool >= VG_LITE_MEMORY_POOL_1 && pool <= VG_LITE_MEMORY_POOL_2))
        return VG_LITE_INVALID_ARGUMENT;

    switch (type) {
      case VG_LITE_COMMAND_BUFFER:
        s_context.command_buffer_pool = pool;
        break;

      case VG_LITE_TESSELLATION_BUFFER:
        s_context.tess_buffer_pool = pool;
        break;

      case VG_LITE_RENDER_BUFFER:
        s_context.render_buffer_pool = pool;
        break;

      default:
        return VG_LITE_INVALID_ARGUMENT;
    }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_frame_delimiter(vg_lite_frame_flag_t flag, vg_lite_bool_t stall_flag)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_FRAME_DELIMITER_API);
    VG_LITE_TRACE_API("vg_lite_frame_delimiter \n");

    vg_lite_error_t error;

    s_context.frame_flag = flag;
    if(stall_flag == 0)
        error = vg_lite_flush();
    else
        error = vg_lite_finish();

    return error;
}

vg_lite_error_t vg_lite_cache_command(vg_lite_cmdcache_operation_t operation, vg_lite_int32_t *buf_index, vg_lite_matrix_t *matrix)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_CACHE_COMMAND_API);

    vg_lite_error_t error = VG_LITE_SUCCESS;

#if gcFEATURE_VG_COMMAND_BUFFER_CACHE
    vg_lite_kernel_cmdcache_t data;

    switch (operation) {
    case VG_LITE_CMDCACHE_START:
    {
        s_context.backup_fb_command_flag = 1;
        vg_lite_cache_cmd_info* current_buf = (vg_lite_cache_cmd_info*)vg_lite_os_malloc(sizeof(vg_lite_cache_cmd_info));
        memset(current_buf, 0, sizeof(vg_lite_cache_cmd_info));

        if (!s_context.fb_command_buffer_start)
            s_context.fb_command_buffer_start = s_context.fb_command_buffer_end = current_buf;
        else
        {
            s_context.fb_command_buffer_end->next = current_buf;
            s_context.fb_command_buffer_end = current_buf;
        }

        while ((s_context.fb_command_offset & (64-1)) != 0)
            s_context.fb_command_offset += 8;

        if (matrix)
            current_buf->special_register_address = 0x0A40;

        current_buf->fb_command_offset_start = s_context.fb_command_offset;
        current_buf->next = NULL;
        
        *buf_index = s_context.fb_command_buffer_index;

        break;
    }

    case VG_LITE_CMDCACHE_END:
    {
        s_context.backup_fb_command_flag = 0;
        s_context.fb_command_buffer_end->fb_command_offset_end = s_context.fb_command_offset;
        s_context.fb_command_buffer_index++;

        break;
    }

    case VG_LITE_CMDCACHE_CLEAR:
    {
        s_context.fb_command_offset = 0;
        s_context.fb_command_buffer_index = 0;
        vg_lite_cache_cmd_info* buf_start = s_context.fb_command_buffer_start;
        vg_lite_cache_cmd_info* buf_next = NULL;

        while (buf_start != NULL)
        {
            buf_next = buf_start->next;
            vg_lite_os_free(buf_start);
            buf_start = buf_next;
        }
        
        s_context.fb_command_buffer_start = NULL;

        break;
    }                

    case VG_LITE_CMDCACHE_EXECUTE:
    {
        vg_lite_int32_t temp_index = *buf_index;
        vg_lite_cache_cmd_info* execute_buf = s_context.fb_command_buffer_start;

        for (vg_lite_int32_t i = 0; i < temp_index; i++) {
            execute_buf = execute_buf->next;
        }

        if (matrix && execute_buf->special_register_address)
            modify_matrix_cache_command(execute_buf, matrix);

        data.physical = s_context.fb_command_buffer_physical + execute_buf->fb_command_offset_start;
        data.size = execute_buf->fb_command_offset_end - execute_buf->fb_command_offset_start;
        vg_lite_kernel(VG_LITE_EXECUTE_BACKUP_COMMAND, &data);

        break;
    }

    default:
        error = VG_LITE_INVALID_ARGUMENT;
    }
#endif

    return error;
}

vg_lite_error_t vg_lite_set_dump_api(vg_lite_char flag)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_DUMP_API);

    dump_api_flag = flag;
    return VG_LITE_SUCCESS;
}
