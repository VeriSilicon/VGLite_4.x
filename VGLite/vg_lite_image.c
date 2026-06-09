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

#define MATRIX_FP_ABS(x)            (((x) < 0) ? -(x) : (x))
#define MATRIX_FP_EPS               2.2204460492503131e-14

extern vg_lite_matrix_t identity_mtx;

/* Get the plane memory pointer and strides info. */
static vg_lite_uint32_t get_buffer_planes(vg_lite_buffer_t *buffer,
                              vg_lite_uint8_t **memory,
                              vg_lite_uint32_t *strides)
{
    vg_lite_uint32_t count = 1;
    
    switch (buffer->format) {
        case VG_LITE_RGBA8888:
        case VG_LITE_BGRA8888:
        case VG_LITE_RGBX8888:
        case VG_LITE_BGRX8888:
        case VG_LITE_RGB565:
        case VG_LITE_BGR565:
        case VG_LITE_RGBA4444:
        case VG_LITE_BGRA4444:
        case VG_LITE_BGRA5551:
        case VG_LITE_A1:
        case VG_LITE_A2:
        case VG_LITE_A4:
        case VG_LITE_A8:
        case VG_LITE_L4:
        case VG_LITE_L8:
        case VG_LITE_A8L8:
        case VG_LITE_INDEX_1:
        case VG_LITE_INDEX_2:
        case VG_LITE_INDEX_4:
        case VG_LITE_INDEX_8:
        case VG_LITE_YUYV:
        case VG_LITE_YUY2:
        case VG_LITE_RGBA2222:
            count = 1;
            memory[0] = (vg_lite_uint8_t *)buffer->memory;
            memory[1] = memory[2] = ((vg_lite_uint8_t*)0);
            strides[0] = buffer->stride;
            strides[1] = strides[2] = 0;
            break;

        case VG_LITE_NV12:
        case VG_LITE_NV16:
        case VG_LITE_NV24:
        case VG_LITE_NV24_TILED:
            count = 2;
            memory[0] = (vg_lite_uint8_t *)buffer->memory;
            memory[1] = (vg_lite_uint8_t *)buffer->yuv.uv_memory;
            memory[2] = 0;
            strides[0] = buffer->stride;
            strides[1] = buffer->yuv.uv_stride;
            strides[2] = 0;
            break;
            
        case VG_LITE_AYUY2:
            count = 2;
            memory[0] = (vg_lite_uint8_t *)buffer->memory;
            memory[1] = 0;
            memory[2] = (vg_lite_uint8_t *)buffer->yuv.v_memory;
            strides[0] = buffer->stride;
            strides[1] = 0;
            strides[2] = buffer->yuv.alpha_stride;
            break;

        case VG_LITE_ANV12:
            count = 3;
            memory[0] = (vg_lite_uint8_t *)buffer->memory;
            memory[1] = (vg_lite_uint8_t *)buffer->yuv.uv_memory;
            memory[2] = (vg_lite_uint8_t *)buffer->yuv.v_memory;
            strides[0] = buffer->stride;
            strides[1] = buffer->yuv.uv_stride;
            strides[2] = buffer->yuv.alpha_stride;
            break;
            
        case VG_LITE_YV12:
        case VG_LITE_YV24:
        case VG_LITE_YV16:
            count = 3;
            memory[0] = (vg_lite_uint8_t *)buffer->memory;
            memory[1] = (vg_lite_uint8_t *)buffer->yuv.uv_memory;
            memory[2] = (vg_lite_uint8_t *)buffer->yuv.v_memory;
            strides[0] = buffer->stride;
            strides[1] = buffer->yuv.uv_stride;
            strides[2] = buffer->yuv.v_stride;
            break;
            
        case VG_LITE_YUY2_TILED:
        case VG_LITE_NV12_TILED:
        case VG_LITE_ANV12_TILED:
        case VG_LITE_AYUY2_TILED:
        default:
            count = 0;
            
            break;
    }
    return count;
}

vg_lite_error_t vg_lite_upload_buffer(vg_lite_buffer_t  *buffer,
                                      vg_lite_uint8_t *data[3],
                                      vg_lite_uint32_t stride[3])
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_UPLOAD_BUFFER_API);
    DUMP_API_CALL(vg_lite_upload_buffer, buffer, data, stride);
    VG_LITE_TRACE_API("vg_lite_upload_buffer %p %p %p\n", buffer, data, stride);

    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_int32_t plane_count;
    vg_lite_uint8_t  *buffer_memory[3] = {((vg_lite_uint8_t*)0)};
    vg_lite_uint32_t  buffer_strides[3] = {0};
    vg_lite_uint8_t  *pdata;
    vg_lite_int32_t i, j;


    /* Get buffer memory info. */
    plane_count = get_buffer_planes(buffer, buffer_memory, buffer_strides);

    if (plane_count > 0) {
        /* Copy the data to buffer. */
        for (i = 0; i < plane_count;  i++) {
            pdata = data[i];
            for (j = 0; j < buffer->height; j++) {
                memcpy(buffer_memory[i], pdata, buffer_strides[i]);
                buffer_memory[i] += buffer_strides[i];
                pdata += stride[i];
            }
        }
    }
    else {
        error = VG_LITE_INVALID_ARGUMENT;
    }

    return error;
}

static vg_lite_error_t swap(vg_lite_float_t* a, vg_lite_float_t* b)
{
    vg_lite_float_t temp;
    VG_LITE_CHECK_NULL_POINTER2(a, b);
    temp = *a;
    *a = *b;
    *b = temp;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_get_transform_matrix(vg_lite_float_point4_t src, vg_lite_float_point4_t dst, vg_lite_matrix_t* mat)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_GET_TRANSFORM_MATRIX_API);
    VG_LITE_TRACE_API("vg_lite_get_transform_matrix %p %p %p\n", src, dst, mat);

    vg_lite_float_t a[8][8], b[9], A[64];
    vg_lite_int32_t i, j, k, m = 8, n = 1;
    vg_lite_int32_t astep = 8, bstep = 1;
    vg_lite_float_t d;


    VG_LITE_CHECK_NULL_POINTER3(src, dst, mat);

    for (i = 0; i < 4; ++i)
    {
        a[i][0] = a[i + 4][3] = (vg_lite_float_t)src[i].x;
        a[i][1] = a[i + 4][4] = (vg_lite_float_t)src[i].y;
        a[i][2] = a[i + 4][5] = 1.0f;
        a[i][3] = a[i][4] = a[i][5] =
            a[i + 4][0] = a[i + 4][1] = a[i + 4][2] = 0.0f;
        a[i][6] = (vg_lite_float_t)(-src[i].x * dst[i].x);
        a[i][7] = (vg_lite_float_t)(-src[i].y * dst[i].x);
        a[i + 4][6] = (vg_lite_float_t)(-src[i].x * dst[i].y);
        a[i + 4][7] = (vg_lite_float_t)(-src[i].y * dst[i].y);
        b[i] = (vg_lite_float_t)dst[i].x;
        b[i + 4] = (vg_lite_float_t)dst[i].y;
    }
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            A[8 * i + j] = a[i][j];
        }
    }

    for (i = 0; i < m; i++)
    {
        k = i;
        for (j = i + 1; j < m; j++)
            if (MATRIX_FP_ABS(A[j * astep + i]) > MATRIX_FP_ABS(A[k * astep + i]))
                k = j;
        if (MATRIX_FP_ABS(A[k * astep + i]) < MATRIX_FP_EPS)
            return VG_LITE_INVALID_ARGUMENT;
        if (k != i)
        {
            for (j = i; j < m; j++)
                swap(&A[i * astep + j], &A[k * astep + j]);
            for (j = 0; j < n; j++)
                swap(&b[i * bstep + j], &b[k * bstep + j]);
        }
        d = -1 / A[i * astep + i];
        for (j = i + 1; j < m; j++)
        {
            vg_lite_float_t alpha = A[j * astep + i] * d;
            for (k = i + 1; k < m; k++)
                A[j * astep + k] += alpha * A[i * astep + k];
            for (k = 0; k < n; k++)
                b[j * bstep + k] += alpha * b[i * bstep + k];
        }
    }

    for (i = m - 1; i >= 0; i--)
        for (j = 0; j < n; j++)
        {
            vg_lite_float_t s = b[i * bstep + j];
            for (k = i + 1; k < m; k++)
                s -= A[i * astep + k] * b[k * bstep + j];
            b[i * bstep + j] = s / A[i * astep + i];
        }

    b[8] = 1;

    for (i = 0; i < 3; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            mat->m[i][j] = b[i * 3 + j];
        }
    }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_set_scissor(vg_lite_int32_t x, vg_lite_int32_t y, vg_lite_int32_t right, vg_lite_int32_t bottom)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_SCISSOR_API);
    DUMP_API_CALL(vg_lite_set_scissor, x, y, right, bottom);
    VG_LITE_TRACE_API("vg_lite_set_scissor %d %d %d %d\n", x, y, right, bottom);

#if gcFEATURE_VG_SCISSOR
    vg_lite_error_t error = VG_LITE_SUCCESS;


    /* Save scissor Box States. */
    s_context.scissor[0] = x;
    s_context.scissor[1] = y;
    s_context.scissor[2] = right;
    s_context.scissor[3] = bottom;

    /* Scissor dirty. */
    s_context.scissor_dirty = 1;
    s_context.scissor_set = (x == -1 && y == -1 && right == -1 && bottom == -1) ? 0 : 1;

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_enable_scissor()
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_ENABLE_SCISSOR_API);
    DUMP_API_CALL(vg_lite_enable_scissor);
    VG_LITE_TRACE_API("vg_lite_enable_scissor\n");

#if gcFEATURE_VG_MASK


    /* Enable scissor Mode. */
    if (!s_context.scissor_enable) {
        s_context.scissor_enable = 1 << 4;
        s_context.scissor_dirty = 1;
    }

    return VG_LITE_SUCCESS;
#else
    /* Noop */
    return VG_LITE_SUCCESS;
#endif
}

vg_lite_error_t vg_lite_disable_scissor()
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DISABLE_SCISSOR_API);
    DUMP_API_CALL(vg_lite_disable_scissor);
    VG_LITE_TRACE_API("vg_lite_disable_scissor\n");

#if gcFEATURE_VG_MASK


    /* Disable scissor Mode. */
    if (s_context.scissor_enable) {
        s_context.scissor_enable = 0;
        s_context.scissor_dirty = 1;
    }

    return VG_LITE_SUCCESS;
#else
    /* Noop */
    return VG_LITE_SUCCESS;
#endif
}

vg_lite_error_t vg_lite_set_CLUT(vg_lite_uint32_t count, vg_lite_uint32_t* colors)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_CLUT_API);
    VG_LITE_TRACE_API("vg_lite_set_CLUT %d %p\n", count, colors);

#if gcFEATURE_VG_IM_INDEX_FORMAT
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t addr = 0x0B00;


#if gcFEATURE_VG_NEW_IMAGE_INDEX
    {
        switch (count) {
        case 256:
        case 16:
        case 4:
        case 2:
            addr = 0x0B00;
            break;
        default:
            error = VG_LITE_INVALID_ARGUMENT;
            return error;
            break;
        }
    }
#else
    {
        switch (count) {
        case 256:
            addr = 0x0B00;
            break;
        case 16:
            addr = 0x0AA0;
            break;
        case 4:
            addr = 0x0A9C;
            break;
        case 2:
            addr = 0x0A98;
            break;
        default:
            error = VG_LITE_INVALID_ARGUMENT;
            return error;
            break;
        }
    }
#endif

    VG_LITE_RETURN_ERROR(push_clut(&s_context, addr, count, (vg_lite_uint32_t*)colors));

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_source_global_alpha(vg_lite_global_alpha_t alpha_mode, vg_lite_uint8_t alpha_value)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SOURCE_GLOBAL_ALPHA_API);
    DUMP_API_CALL(vg_lite_source_global_alpha, alpha_mode, alpha_value);
    VG_LITE_TRACE_API("vg_lite_source_global_alpha %d %d\n", alpha_mode, alpha_value);

    return feature_apply_source_global_alpha(&s_context, alpha_mode, alpha_value);
}

vg_lite_error_t vg_lite_dest_global_alpha(vg_lite_global_alpha_t alpha_mode, vg_lite_uint8_t alpha_value)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DEST_GLOBAL_ALPHA_API);
    DUMP_API_CALL(vg_lite_dest_global_alpha, alpha_mode, alpha_value);
    VG_LITE_TRACE_API("vg_lite_dest_global_alpha %d %d\n", alpha_mode, alpha_value);

    return feature_apply_dest_global_alpha(&s_context, alpha_mode, alpha_value);
}

vg_lite_error_t vg_lite_set_color_key(vg_lite_color_key4_t colorkey)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_COLOR_KEY_API);
    DUMP_API_CALL(vg_lite_set_color_key, colorkey);
    VG_LITE_TRACE_API("vg_lite_set_color_key %p\n", colorkey);

#if gcFEATURE_VG_COLOR_KEY
    vg_lite_uint8_t i;
    vg_lite_uint32_t value_low = 0;
    vg_lite_uint32_t value_high = 0;
    vg_lite_uint8_t r, g, b, a, e;
    vg_lite_error_t error = VG_LITE_SUCCESS;


    /* Set color key states. */
    for (i = 0; i < 4; i++)
    {
        if(colorkey[i].enable == 1) {   
            /* Set gcregVGPEColorKeyLow. Layout "E/R/G/B". */
            r = colorkey[i].low_r;
            g = colorkey[i].low_g;
            b = colorkey[i].low_b;
            e = colorkey[i].enable;
            value_low = (e << 24) | (r << 16) | (g << 8) | b;
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A90 + i, value_low));

            /* Set gcregVGPEColorKeyHigh. Layout "A/R/G/B". */
            r = colorkey[i].high_r;
            g = colorkey[i].high_g;
            b = colorkey[i].high_b;
            a = colorkey[i].alpha;
            value_high = (a << 24) | (r << 16) | (g << 8) | b;
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A94 + i, value_high));
        }
        else {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A90 + i, 0));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A94 + i, 0));
        }
    }

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_enable_dither()
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_ENABLE_DITHER_API);
    DUMP_API_CALL(vg_lite_enable_dither);
    VG_LITE_TRACE_API("vg_lite_enable_dither\n");

#if gcFEATURE_VG_DITHER
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t table_low = 0x7B48F3C0;
    vg_lite_uint32_t table_high = 0x596AD1E2;


    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A5A, table_low));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A5B, table_high));

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_disable_dither()
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DISABLE_DITHER_API);
    DUMP_API_CALL(vg_lite_disable_dither);
    VG_LITE_TRACE_API("vg_lite_disable_dither\n");

#if gcFEATURE_VG_DITHER
    vg_lite_error_t error = VG_LITE_SUCCESS;


    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A5A, 0xFFFFFFFF));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A5B, 0xFFFFFFFF));

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_enable_masklayer()
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_ENABLE_MASKLAYER_API);
    DUMP_API_CALL(vg_lite_enable_masklayer);
    VG_LITE_TRACE_API("vg_lite_enable_masklayer\n");

#if gcFEATURE_VG_MASK
    vg_lite_error_t error = VG_LITE_SUCCESS;


    s_context.enable_mask = (1 << 20);

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_disable_masklayer()
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DISABLE_MASKLAYER_API);
    DUMP_API_CALL(vg_lite_disable_masklayer);
    VG_LITE_TRACE_API("vg_lite_disable_masklayer\n");

#if gcFEATURE_VG_MASK
    vg_lite_error_t error = VG_LITE_SUCCESS;


    s_context.enable_mask = 0;

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_create_masklayer(vg_lite_buffer_t* masklayer, vg_lite_uint32_t width, vg_lite_uint32_t height)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_CREATE_MASKLAYER_API);
    DUMP_API_CALL(vg_lite_create_masklayer, masklayer, width, height);
    VG_LITE_TRACE_API("vg_lite_create_masklayer %p %d %d\n", masklayer, width, height);

#if gcFEATURE_VG_MASK
    vg_lite_error_t error = VG_LITE_SUCCESS;


    memset(masklayer, 0, sizeof(vg_lite_buffer_t));
    masklayer->width = width;
    masklayer->height = height;
    masklayer->format = VG_LITE_A8;
    VG_LITE_RETURN_ERROR(vg_lite_allocate(masklayer));

    VG_LITE_RETURN_ERROR(vg_lite_clear(masklayer, NULL, (vg_lite_color_t)(0xFF << 24)));

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_fill_masklayer(vg_lite_buffer_t* masklayer, vg_lite_rectangle_t* rect, vg_lite_uint8_t value)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_FILL_MASKLAYER_API);
    DUMP_API_CALL(vg_lite_fill_masklayer, masklayer, rect, value);
    VG_LITE_TRACE_API("vg_lite_fill_masklayer %p %p %d\n", masklayer, rect, value);

#if gcFEATURE_VG_MASK
    vg_lite_error_t error = VG_LITE_SUCCESS;


    error = vg_lite_clear(masklayer, rect, value << 24);

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_blend_masklayer(
    vg_lite_buffer_t* dst_masklayer,
    vg_lite_buffer_t* src_masklayer,
    vg_lite_mask_operation_t operation,
    vg_lite_rectangle_t* rect
)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_BLEND_MASKLAYER_API);
    DUMP_API_CALL(vg_lite_blend_masklayer, dst_masklayer, src_masklayer, operation, rect);
    VG_LITE_TRACE_API("vg_lite_blend_masklayer %p %p %d %p\n", dst_masklayer, src_masklayer, operation, rect);

#if gcFEATURE_VG_MASK
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    vg_lite_filter_t filter = VG_LITE_FILTER_POINT;
    vg_lite_rectangle_t area = *rect;


    vg_lite_identity(&matrix);
    vg_lite_translate((vg_lite_float_t)rect->x, (vg_lite_float_t)rect->y, &matrix);

    switch (operation)
    {
    case VG_LITE_CLEAR_MASK:
        VG_LITE_RETURN_ERROR(vg_lite_clear(dst_masklayer, &area, 0x0));
        break;
    case VG_LITE_FILL_MASK:
        VG_LITE_RETURN_ERROR(vg_lite_clear(dst_masklayer, &area, (vg_lite_color_t)(0xFF << 24)));
        break;
    case VG_LITE_SET_MASK:
        area.x = 0;
        area.y = 0;
        VG_LITE_RETURN_ERROR(vg_lite_blit_rect(dst_masklayer, src_masklayer, &area, &matrix, VG_LITE_BLEND_NONE, 0, filter));
        break;
    case VG_LITE_UNION_MASK:
        area.x = 0;
        area.y = 0;
        VG_LITE_RETURN_ERROR(vg_lite_blit_rect(dst_masklayer, src_masklayer, &area, &matrix, VG_LITE_BLEND_SCREEN, 0, filter));
        break;
    case VG_LITE_INTERSECT_MASK:
        area.x = 0;
        area.y = 0;
        VG_LITE_RETURN_ERROR(vg_lite_blit_rect(dst_masklayer, src_masklayer, &area, &matrix, VG_LITE_BLEND_DST_IN, 0, filter));
        break;
    case VG_LITE_SUBTRACT_MASK:
        area.x = 0;
        area.y = 0;
        VG_LITE_RETURN_ERROR(vg_lite_blit_rect(dst_masklayer, src_masklayer, &area, &matrix, VG_LITE_BLEND_SUBTRACT, 0, filter));
        break;
    default:
        break;
    }

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_set_masklayer(vg_lite_buffer_t* masklayer, vg_lite_int32_t x, vg_lite_int32_t y)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_MASKLAYER_API);
    DUMP_API_CALL(vg_lite_set_masklayer, masklayer);
    VG_LITE_TRACE_API("vg_lite_set_masklayer %p\n", masklayer);

#if gcFEATURE_VG_MASK
    vg_lite_error_t error = VG_LITE_SUCCESS;


#if gcFEATURE_VG_NEW_ROI_MASK
    if (x < 0 || y < 0)
        return VG_LITE_INVALID_ARGUMENT;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF2, y << 16 | x));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF3, masklayer->height << 16 | masklayer->width));
#endif  

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A14, masklayer->address));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A15, masklayer->stride));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000010));

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_render_masklayer(
    vg_lite_buffer_t* masklayer,
    vg_lite_mask_operation_t operation,
    vg_lite_path_t* path,
    vg_lite_fill_t fill_rule,
    vg_lite_color_t color,
    vg_lite_matrix_t* matrix
)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_RENDER_MASKLAYER_API);
    DUMP_API_CALL(vg_lite_render_masklayer, masklayer, operation, path, fill_rule, color, matrix);
    VG_LITE_TRACE_API("vg_lite_render_masklayer %p %d %p %d %d %p\n", masklayer, operation, path, fill_rule, color, matrix);

#if gcFEATURE_VG_MASK
    vg_lite_error_t error = VG_LITE_SUCCESS;


    if (!matrix) {
        matrix = &identity_mtx;
    }

    switch (operation)
    {
    case VG_LITE_CLEAR_MASK:
        VG_LITE_RETURN_ERROR(vg_lite_draw(masklayer, path, fill_rule, matrix, VG_LITE_BLEND_NONE, 0));
        break;
    case VG_LITE_FILL_MASK:
        VG_LITE_RETURN_ERROR(vg_lite_draw(masklayer, path, fill_rule, matrix, VG_LITE_BLEND_NONE, (vg_lite_color_t)(0xFF << 24)));
        break;
    case VG_LITE_SET_MASK:
        VG_LITE_RETURN_ERROR(vg_lite_draw(masklayer, path, fill_rule, matrix, VG_LITE_BLEND_NONE, color << 24));
        break;
    case VG_LITE_UNION_MASK:
        VG_LITE_RETURN_ERROR(vg_lite_draw(masklayer, path, fill_rule, matrix, VG_LITE_BLEND_SCREEN, color << 24));
        break;
    case VG_LITE_INTERSECT_MASK:
        VG_LITE_RETURN_ERROR(vg_lite_draw(masklayer, path, fill_rule, matrix, VG_LITE_BLEND_DST_IN, color << 24));
        break;
    case VG_LITE_SUBTRACT_MASK:
        VG_LITE_RETURN_ERROR(vg_lite_draw(masklayer, path, fill_rule, matrix, VG_LITE_BLEND_SUBTRACT, color << 24));
        break;
    default:
        break;
    }

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_destroy_masklayer(vg_lite_buffer_t* masklayer)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DESTROY_MASKLAYER_API);
    DUMP_API_CALL(vg_lite_destroy_masklayer, masklayer);
    VG_LITE_TRACE_API("vg_lite_destroy_masklayer %p\n", masklayer);

#if gcFEATURE_VG_MASK
    vg_lite_error_t error = VG_LITE_SUCCESS;


#if DUMP_CAPTURE
    vglitemDUMP_BUFFER("masklayer", (size_t)masklayer->address, masklayer->memory, 0, (masklayer->stride) * (masklayer->height));
#endif

    VG_LITE_RETURN_ERROR(vg_lite_free(masklayer));

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_set_pixel_matrix(vg_lite_pixel_matrix_t matrix, vg_lite_pixel_channel_enable_t* channel)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_PIXEL_MATRIX_API);
    DUMP_API_CALL(vg_lite_set_pixel_matrix, matrix, channel);
    VG_LITE_TRACE_API("vg_lite_set_pixel_matrix %p (%d %d %d %d)\n", matrix, channel->enable_a, channel->enable_b, channel->enable_g, channel->enable_r);

#if gcFEATURE_VG_PIXEL_MATRIX
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_int16_t pix_matrix[20] = { 0 };
    vg_lite_int32_t i = 0;


    s_context.matrix_enable = (channel->enable_a ? (1 << 7) : 0) |
        (channel->enable_r ? (1 << 23) : 0) |
        (channel->enable_g ? (1 << 22) : 0) |
        (channel->enable_b ? (1 << 21) : 0);

    if (s_context.matrix_enable)
    {
        for (i = 0; i < 20; i++) {
            if (matrix[i] > 127.0f || matrix[i] < -128.0f) {
                return VG_LITE_INVALID_ARGUMENT;
            }
            pix_matrix[i] = (vg_lite_int16_t)(matrix[i] * 256);
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0ADE + i, pix_matrix[i]));
        }
    }

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_gaussian_filter(vg_lite_float_t w0, vg_lite_float_t w1, vg_lite_float_t w2)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_GAUSSIAN_FILTER_API);
    DUMP_API_CALL(vg_lite_gaussian_filter, w0, w1, w2);
    VG_LITE_TRACE_API("vg_lite_gaussian_filter %f %f %f\n", w0, w1, w2);

#if gcFEATURE_VG_GAUSSIAN_BLUR
    vg_lite_error_t error = VG_LITE_SUCCESS;


    if (fabs(w0 + 4 * w1 + 4 * w2 - 1.0) > 0.000001 || w0 < 0 || w1 < 0 || w2 < 0)
        return VG_LITE_INVALID_ARGUMENT;
#if (CHIPID == 0X555)
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AD4 + 1, (vg_lite_uint32_t)(w1 * 1024)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AD6 + 1, (vg_lite_uint32_t)(w2 * 1024)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AD2 + 1, (vg_lite_uint32_t)(1024 - floor(w1 * 1024) * 4 - floor(w2 * 1024) * 4)));
#else
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AD4 + 1, (vg_lite_uint32_t)(w1 * 256)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AD6 + 1, (vg_lite_uint32_t)(w2 * 256)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AD2 + 1, (vg_lite_uint32_t)(256 - floor(w1 * 256) * 4 - floor(w2 * 256) * 4)));
#endif

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

#if gcFEATURE_VG_NEW_ROI_MASK
static vg_lite_error_t findMinBoundingRect(vg_lite_rectangle_t* rects, vg_lite_buffer_t* target, vg_lite_uint32_t nums, vg_lite_rectangle_t* boundingRect) {
    if (nums == 0 || rects == NULL || target == NULL || boundingRect == NULL)
    {
        return VG_LITE_INVALID_ARGUMENT;
    }

    vg_lite_int32_t minX = 0x7fffffff;
    vg_lite_int32_t minY = 0x7fffffff;
    vg_lite_int32_t maxX = 0;
    vg_lite_int32_t maxY = 0;
    vg_lite_int32_t found = 0;

    for (vg_lite_int32_t i = 0; i < (vg_lite_int32_t)nums; i++) {
        if (rects[i].width <= 0 || rects[i].height <= 0) {
            continue;
        }
        int64_t x0 = (int64_t)rects[i].x;
        int64_t y0 = (int64_t)rects[i].y;
        int64_t x1 = x0 + (int64_t)rects[i].width;
        int64_t y1 = y0 + (int64_t)rects[i].height;
        
         /* Clamp to target bounds. */
        if (x0 < 0) x0 = 0;
        if (y0 < 0) y0 = 0;
        if (x1 > (int64_t)target->width)  x1 = (int64_t)target->width;
        if (y1 > (int64_t)target->height) y1 = (int64_t)target->height;
        
            if (x1 <= x0 || y1 <= y0) {
            continue;
            
        }
        
        if ((vg_lite_int32_t)x0 < minX) minX = (vg_lite_int32_t)x0;
        if ((vg_lite_int32_t)y0 < minY) minY = (vg_lite_int32_t)y0;
        if ((vg_lite_int32_t)x1 > maxX) maxX = (vg_lite_int32_t)x1;
        if ((vg_lite_int32_t)y1 > maxY) maxY = (vg_lite_int32_t)y1;
        found = 1;
    }
        if (!found) {
        return VG_LITE_INVALID_ARGUMENT;
    }
    boundingRect->x = minX;
    boundingRect->y = minY;
    boundingRect->width = maxX - minX;
    boundingRect->height = maxY - minY;
    return VG_LITE_SUCCESS;
}
#endif

vg_lite_error_t vg_lite_scissor_rects(vg_lite_buffer_t *target, vg_lite_uint32_t nums, vg_lite_rectangle_t rect[])
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SCISSOR_RECTS_API);
    DUMP_API_CALL(vg_lite_scissor_rects, target, nums, rect);

#if gcFEATURE_VG_MASK
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_rectangle_t rect_clamp, rect_draw;
    vg_lite_int32_t left_x, right_x, left_len, middle_len, right_len, stride, j, max_x, max_y;
    vg_lite_uint8_t alpha;
    vg_lite_uint32_t i;
#if gcFEATURE_VG_NEW_ROI_MASK && gcFEATURE_VG_ROI_SCISSOR_LAYER
    vg_lite_rectangle_t rect_scissor_layer;
#endif

    VG_LITE_TRACE_API("vg_lite_scissor_rects %d %p\n", nums, rect);
    for (i = 0; i < nums; i++) {
        VG_LITE_TRACE_API("    Rect(%d, %d, %d, %d)\n", rect[i].x, rect[i].y, rect[i].width, rect[i].height);
    }

    //initial range
    if (nums == 0 && rect != NULL) {
        s_context.scissor_layer_range[0] = 0;
        s_context.scissor_layer_range[1] = 0;
        s_context.scissor_layer_range[2] = 0;
        s_context.scissor_layer_range[3] = 0;
    }
    else if (nums > 0 && rect != NULL) {
        s_context.scissor_layer_range[0] = rect[0].x;
        s_context.scissor_layer_range[1] = rect[0].y;
        s_context.scissor_layer_range[2] = rect[0].x + rect[0].width;
        s_context.scissor_layer_range[3] = rect[0].y + rect[0].height;
    }
    else
        return VG_LITE_INVALID_ARGUMENT;

    /* Record scissor enable flag and disable scissor. */
    vg_lite_uint8_t enable = (vg_lite_uint8_t)s_context.scissor_enable;
    s_context.scissor_enable = 0;

#if gcFEATURE_VG_NEW_ROI_MASK && gcFEATURE_VG_ROI_SCISSOR_LAYER
    vg_lite_error_t br_err = findMinBoundingRect(rect, target, nums, &rect_scissor_layer);
    if (br_err != VG_LITE_SUCCESS) {
        rect_scissor_layer.x = 0;
        rect_scissor_layer.y = 0;
        rect_scissor_layer.width = target->width;
        rect_scissor_layer.height = target->height;
    }
    vg_lite_int32_t req_w  = (rect_scissor_layer.width + 7) / 8;
    vg_lite_int32_t req_h = rect_scissor_layer.height;
#else
    vg_lite_int32_t req_w = (target->width + 7) / 8;
    vg_lite_int32_t req_h = target->height;
#endif

    /* Free the old scissor layer if its size is too small for target */
    if (s_context.scissor_layer && ((s_context.scissor_layer->width < req_w) || (s_context.scissor_layer->height < req_h))) {
        vg_lite_free(s_context.scissor_layer);
        vg_lite_os_free(s_context.scissor_layer);
        s_context.scissor_layer = NULL;
    }

    /* Allocate if scissor layer is NULL */
    if (s_context.scissor_layer == NULL) {
        s_context.scissor_layer = (vg_lite_buffer_t *)vg_lite_os_malloc(sizeof(vg_lite_buffer_t));
        if (!s_context.scissor_layer) {
            return VG_LITE_OUT_OF_RESOURCES;
        }

        memset(s_context.scissor_layer, 0, sizeof(vg_lite_buffer_t));
        s_context.scissor_layer->scissor_buffer = 1;

        s_context.scissor_layer->width = req_w;
        s_context.scissor_layer->height = req_h;
        s_context.scissor_layer->format = VG_LITE_A8;
        VG_LITE_RETURN_ERROR(vg_lite_allocate(s_context.scissor_layer));
    }
    s_context.scissor_layer->scissor_buffer = 1;

    /* Clear scissor layer*/
    VG_LITE_RETURN_ERROR(vg_lite_clear(s_context.scissor_layer, NULL, 0x00000000));
    vg_lite_finish();

#if gcFEATURE_VG_NEW_ROI_MASK && gcFEATURE_VG_ROI_SCISSOR_LAYER
    max_x = rect_scissor_layer.width;
    max_y = rect_scissor_layer.height;
#else
    max_x = s_context.scissor_layer->width * 8;
    max_y = s_context.scissor_layer->height;
#endif
    /* Draw rectangle to scissor layer, one bit data of scissor layer corresponds to one pixel. */
    /* Clear at first. */
    for (i = 0; i < nums; ++i) {

#if gcFEATURE_VG_NEW_ROI_MASK && gcFEATURE_VG_ROI_SCISSOR_LAYER
        /* Clamp the rect relative to the scissor_layer coordinates. */
        int64_t rx0 = (int64_t)rect[i].x;
        int64_t ry0 = (int64_t)rect[i].y;
        int64_t rx1 = rx0 + (int64_t)rect[i].width;
        int64_t ry1 = ry0 + (int64_t)rect[i].height;

        int64_t sx0 = (int64_t)rect_scissor_layer.x;
        int64_t sy0 = (int64_t)rect_scissor_layer.y;
        int64_t sx1 = sx0 + (int64_t)rect_scissor_layer.width;
        int64_t sy1 = sy0 + (int64_t)rect_scissor_layer.height;

        int64_t ix0 = (rx0 > sx0) ? rx0 : sx0;
        int64_t iy0 = (ry0 > sy0) ? ry0 : sy0;
        int64_t ix1 = (rx1 < sx1) ? rx1 : sx1;
        int64_t iy1 = (ry1 < sy1) ? ry1 : sy1;

        if (rect[i].width <= 0 || rect[i].height <= 0 || ix1 <= ix0 || iy1 <= iy0) {
            rect_clamp.x = rect_clamp.y = rect_clamp.width = rect_clamp.height = 0;
        }
        else {
            /* Convert to coordinates relative to the scissor layer origin. */
            rect_clamp.x = (vg_lite_int32_t)(ix0 - sx0);
            rect_clamp.y = (vg_lite_int32_t)(iy0 - sy0);
            rect_clamp.width = (vg_lite_int32_t)(ix1 - ix0);
            rect_clamp.height = (vg_lite_int32_t)(iy1 - iy0);
        }
#else
        /* Clamp the rect */
        memcpy(&rect_clamp, &rect[i], sizeof(vg_lite_rectangle_t));
        {
            if (rect_clamp.x < 0) {
                rect_clamp.width += rect_clamp.x;
                rect_clamp.x = 0;
            }

            if (rect_clamp.y < 0) {
                rect_clamp.height += rect_clamp.y;
                rect_clamp.y = 0;
            }

            if (rect_clamp.x >= max_x || rect_clamp.y >= max_y || rect_clamp.width <= 0 || rect_clamp.height <= 0) {
                rect_clamp.x = rect_clamp.y = rect_clamp.width = rect_clamp.height = 0;
            }
            if (rect_clamp.x + rect_clamp.width > max_x) {
                rect_clamp.width = max_x - rect_clamp.x;
            }
            if (rect_clamp.y + rect_clamp.height > max_y) {
                rect_clamp.height = max_y - rect_clamp.y;
            }
        }

#endif
        if (rect_clamp.width <= 0 || rect_clamp.height <= 0)
            continue;

        if (((rect_clamp.x + rect_clamp.width) >> 3) != (rect_clamp.x >> 3)) {
            /* Split the rect */
            left_x = (rect_clamp.x % 8 == 0) ? rect_clamp.x : ((rect_clamp.x + 7) & 0xFFFFFFF8);
            right_x = (rect_clamp.x + rect_clamp.width) & 0xFFFFFFF8;
            middle_len = right_x - left_x;

            /* Draw middle rect */
            if (middle_len) {
                rect_draw.x = left_x / 8;
                rect_draw.y = rect_clamp.y;
                rect_draw.width = middle_len / 8;
                rect_draw.height = rect_clamp.height;
                VG_LITE_RETURN_ERROR(vg_lite_clear(s_context.scissor_layer, &rect_draw, 0xFFFFFFFF));
            }
        }

        //update scissor_layer_range
        if (i != 0) {
            s_context.scissor_layer_range[0] = MIN(rect[i].x, s_context.scissor_layer_range[0]);
            s_context.scissor_layer_range[1] = MIN(rect[i].y, s_context.scissor_layer_range[1]);
            s_context.scissor_layer_range[2] = MAX(rect[i].x + rect[i].width, s_context.scissor_layer_range[2]);
            s_context.scissor_layer_range[3] = MAX(rect[i].y + rect[i].height, s_context.scissor_layer_range[3]);
        }

    }
    VG_LITE_RETURN_ERROR(vg_lite_frame_delimiter(VG_LITE_FRAME_END_FLAG, 1));

    /* write */
    for (i = 0; i < nums; ++i) {

#if gcFEATURE_VG_NEW_ROI_MASK && gcFEATURE_VG_ROI_SCISSOR_LAYER
        /* Clamp the rect relative to the scissor_layer coordinates. */
        int64_t rx0 = (int64_t)rect[i].x;
        int64_t ry0 = (int64_t)rect[i].y;
        int64_t rx1 = rx0 + (int64_t)rect[i].width;
        int64_t ry1 = ry0 + (int64_t)rect[i].height;

        int64_t sx0 = (int64_t)rect_scissor_layer.x;
        int64_t sy0 = (int64_t)rect_scissor_layer.y;
        int64_t sx1 = sx0 + (int64_t)rect_scissor_layer.width;
        int64_t sy1 = sy0 + (int64_t)rect_scissor_layer.height;

        int64_t ix0 = (rx0 > sx0) ? rx0 : sx0;
        int64_t iy0 = (ry0 > sy0) ? ry0 : sy0;
        int64_t ix1 = (rx1 < sx1) ? rx1 : sx1;
        int64_t iy1 = (ry1 < sy1) ? ry1 : sy1;

        if (rect[i].width <= 0 || rect[i].height <= 0 || ix1 <= ix0 || iy1 <= iy0) {
            rect_clamp.x = rect_clamp.y = rect_clamp.width = rect_clamp.height = 0;
        }
        else {
            /* Convert to coordinates relative to the scissor layer origin. */
            rect_clamp.x = (vg_lite_int32_t)(ix0 - sx0);
            rect_clamp.y = (vg_lite_int32_t)(iy0 - sy0);
            rect_clamp.width = (vg_lite_int32_t)(ix1 - ix0);
            rect_clamp.height = (vg_lite_int32_t)(iy1 - iy0);
        }
#else
        /* Clamp the rect */
        memcpy(&rect_clamp, &rect[i], sizeof(vg_lite_rectangle_t));
        {
            if (rect_clamp.x < 0) {
                rect_clamp.width += rect_clamp.x;
                rect_clamp.x = 0;
            }

            if (rect_clamp.y < 0) {
                rect_clamp.height += rect_clamp.y;
                rect_clamp.y = 0;
            }
            if (rect_clamp.x >= max_x || rect_clamp.y >= max_y || rect_clamp.width <= 0 || rect_clamp.height <= 0) {
                rect_clamp.x = rect_clamp.y = rect_clamp.width = rect_clamp.height = 0;
            }
            if (rect_clamp.x + rect_clamp.width > max_x) {
                rect_clamp.width = max_x - rect_clamp.x;
            }
            if (rect_clamp.y + rect_clamp.height > max_y) {
                rect_clamp.height = max_y - rect_clamp.y;
            }
        }

#endif
        if (rect_clamp.width <= 0 || rect_clamp.height <= 0)
            continue;

        if (((rect_clamp.x + rect_clamp.width) >> 3) == (rect_clamp.x >> 3)) {
            rect_draw.x = rect_clamp.x / 8;
            rect_draw.y = rect_clamp.y;
            rect_draw.width = 1;
            rect_draw.height = rect_clamp.height;
            alpha = (vg_lite_uint8_t)(((vg_lite_uint8_t)(0xff >> (8 - rect_clamp.width))) << (rect_clamp.x % 8));
            stride = s_context.scissor_layer->stride;
            for (j = rect_draw.y; j < rect_draw.height + rect_draw.y; ++j) {
                ((vg_lite_uint8_t*)s_context.scissor_layer->memory)[j * stride + rect_draw.x] |= alpha;
            }
        }
        else {
            /* Split the rect */
            left_x = (rect_clamp.x % 8 == 0) ? rect_clamp.x : ((rect_clamp.x + 7) & 0xFFFFFFF8);
            right_x = (rect_clamp.x + rect_clamp.width) & 0xFFFFFFF8;
            left_len = left_x - rect_clamp.x;
            right_len = rect_clamp.x + rect_clamp.width - right_x;

            /* Draw left rect */
            if (left_len) {
                rect_draw.x = rect_clamp.x / 8;
                rect_draw.y = rect_clamp.y;
                rect_draw.width = 1;
                rect_draw.height = rect_clamp.height;
                alpha = (vg_lite_uint8_t)(0xff << (8 - left_len));
                stride = s_context.scissor_layer->stride;
                for (j = rect_draw.y; j < rect_draw.height + rect_draw.y; ++j) {
                    ((vg_lite_uint8_t*)s_context.scissor_layer->memory)[j * stride + rect_draw.x] |= alpha;
                }
            }

            /* Draw right rect */
            if (right_len) {
                rect_draw.x = (rect_clamp.x + rect_clamp.width - right_len) / 8;
                rect_draw.y = rect_clamp.y;
                rect_draw.width = 1;
                rect_draw.height = rect_clamp.height;
                alpha = (vg_lite_uint8_t)(0xff >> (8 - right_len));
                stride = s_context.scissor_layer->stride;
                for (j = rect_draw.y; j < rect_draw.height + rect_draw.y; ++j) {
                    ((vg_lite_uint8_t*)s_context.scissor_layer->memory)[j * stride + rect_draw.x] |= alpha;
                }
            }
        }
    }

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A16, s_context.scissor_layer->address));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A17, s_context.scissor_layer->stride));
#if gcFEATURE_VG_NEW_ROI_MASK
#if gcFEATURE_VG_ROI_SCISSOR_LAYER
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF4, (rect_scissor_layer.y << 16) | rect_scissor_layer.x));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF5, (rect_scissor_layer.height) << 16 | rect_scissor_layer.width));
#else
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF4, 0x0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF5, max_y << 16 | max_x));
#endif
#endif
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000100));
    VG_LITE_RETURN_ERROR(vg_lite_finish());
    s_context.scissor_enable = enable;
    s_context.scissor_dirty = 1;

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_set_mirror(vg_lite_orientation_t orientation)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_MIRROR_API);
    DUMP_API_CALL(vg_lite_set_mirror, orientation);
    VG_LITE_TRACE_API("vg_lite_set_mirror %d\n", orientation);

#if gcFEATURE_VG_MIRROR
    vg_lite_error_t error = VG_LITE_SUCCESS;


    s_context.mirror_orient = orientation;
    s_context.mirror_dirty = 1;

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_set_gamma(vg_lite_gamma_conversion_t gamma_value)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_GAMMA_API);
    DUMP_API_CALL(vg_lite_set_gamma, gamma_value);
    VG_LITE_TRACE_API("vg_lite_set_gamma %d\n", gamma_value);

#if gcFEATURE_VG_GAMMA
    vg_lite_error_t error = VG_LITE_SUCCESS;


    s_context.gamma_value = gamma_value << 12;
    s_context.gamma_dirty = 1;

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

/* Set s_context.gamma_value base on target buffer */
vg_lite_void set_gamma_dest_only(vg_lite_buffer_t *target, vg_lite_int32_t stencil)
{
    vg_lite_uint32_t gamma_value = 0;

    /* Set gamma configuration of source buffer */
    /* Openvg paintcolor defaults to SRGB */
    s_context.gamma_src = 1;

    /* Set gamma configuration of dst buffer */
    if ((target->format >= OPENVG_lRGBX_8888 && target->format <= OPENVG_A_4) ||
        (target->format >= OPENVG_lXRGB_8888 && target->format <= OPENVG_lARGB_8888_PRE) ||
        (target->format >= OPENVG_lBGRX_8888 && target->format <= OPENVG_lBGRA_8888_PRE) ||
        (target->format >= OPENVG_lXBGR_8888 && target->format <= OPENVG_lABGR_8888_PRE) ||
        (target->format >= OPENVG_lRGBX_8888_PRE && target->format <= OPENVG_lRGBA_4444_PRE))
    {
        s_context.gamma_dst = 0;
    } 
    else
    {
        s_context.gamma_dst = 1;
    }

    if (s_context.gamma_src == 0 && s_context.gamma_dst == 1)
    {
        gamma_value = 0x00002000;
    }
    else if (s_context.gamma_src == 1 && s_context.gamma_dst == 0)
    {
        gamma_value = 0x00001000;
    }
    else
    {
        gamma_value = 0x00000000;
    }

    if (stencil && target->image_mode == VG_LITE_STENCIL_MODE)
    {
        s_context.gamma_stencil = gamma_value;
        gamma_value = 0x00000000;
    }

    if (s_context.gamma_dirty == 0 && gamma_value != s_context.gamma_value)
    {
        s_context.gamma_value = gamma_value;
        s_context.gamma_dirty = 1;
    }
}

/* Set s_context.gamma_value base on source and target buffers */
vg_lite_void get_st_gamma_src_dest(vg_lite_buffer_t *source, vg_lite_buffer_t *target)
{
    vg_lite_uint32_t gamma_value = 0;

    /* Set gamma configuration of source buffer */
    if ((source->format >= OPENVG_lRGBX_8888 && source->format <= OPENVG_A_4) ||
        (source->format >= OPENVG_lXRGB_8888 && source->format <= OPENVG_lARGB_8888_PRE) ||
        (source->format >= OPENVG_lBGRX_8888 && source->format <= OPENVG_lBGRA_8888_PRE) ||
        (source->format >= OPENVG_lXBGR_8888 && source->format <= OPENVG_lABGR_8888_PRE) ||
        (source->format >= OPENVG_lRGBX_8888_PRE && source->format <= OPENVG_lRGBA_4444_PRE))
    {
        s_context.gamma_src = 0;
    }
    else
    {
        s_context.gamma_src = 1;
    }
    /* Set gamma configuration of dst buffer */
    if ((target->format >= OPENVG_lRGBX_8888 && target->format <= OPENVG_A_4) ||
        (target->format >= OPENVG_lXRGB_8888 && target->format <= OPENVG_lARGB_8888_PRE) ||
        (target->format >= OPENVG_lBGRX_8888 && target->format <= OPENVG_lBGRA_8888_PRE) ||
        (target->format >= OPENVG_lXBGR_8888 && target->format <= OPENVG_lABGR_8888_PRE) ||
        (target->format >= OPENVG_lRGBX_8888_PRE && target->format <= OPENVG_lRGBA_4444_PRE))
    {
        s_context.gamma_dst = 0;
    }
    else
    {
        s_context.gamma_dst = 1;
    }

    if (s_context.gamma_src == 0 && s_context.gamma_dst == 1)
    {
        gamma_value = 0x00002000;
    }
    else if (s_context.gamma_src == 1 && s_context.gamma_dst == 0)
    {
        gamma_value = 0x00001000;
    }
    else
    {
        gamma_value = 0x00000000;
    }

    if (source->image_mode == VG_LITE_STENCIL_MODE)
    {
        if (source->paintType == VG_LITE_PAINT_PATTERN
            || source->paintType == VG_LITE_PAINT_RADIAL_GRADIENT
            || source->paintType == VG_LITE_PAINT_LINEAR_GRADIENT) {
            gamma_value = s_context.gamma_stencil;
        }
        else if (source->paintType == VG_LITE_PAINT_COLOR && s_context.gamma_dst == 0) {
            gamma_value = 0x00001000;
        }
        else {
            gamma_value = 0x00000000;
        }
    }

    if (s_context.gamma_dirty == 0 && gamma_value != s_context.gamma_value)
    {
        s_context.gamma_value = gamma_value;
        s_context.gamma_dirty = 1;
    }
}

/* Set s_context.gamma_value base on source and target buffers */
vg_lite_void save_st_gamma_src_dest(vg_lite_buffer_t *source, vg_lite_buffer_t *target)
{
    vg_lite_uint32_t gamma_value = 0;

    /* Set gamma configuration of source buffer */
    if ((source->format >= OPENVG_lRGBX_8888 && source->format <= OPENVG_A_4) ||
        (source->format >= OPENVG_lXRGB_8888 && source->format <= OPENVG_lARGB_8888_PRE) ||
        (source->format >= OPENVG_lBGRX_8888 && source->format <= OPENVG_lBGRA_8888_PRE) ||
        (source->format >= OPENVG_lXBGR_8888 && source->format <= OPENVG_lABGR_8888_PRE) ||
        (source->format >= OPENVG_lRGBX_8888_PRE && source->format <= OPENVG_lRGBA_4444_PRE))
    {
        s_context.gamma_src = 0;
    }
    else
    {
        s_context.gamma_src = 1;
    }
    /* Set gamma configuration of dst buffer */
    if ((target->format >= OPENVG_lRGBX_8888 && target->format <= OPENVG_A_4) ||
        (target->format >= OPENVG_lXRGB_8888 && target->format <= OPENVG_lARGB_8888_PRE) ||
        (target->format >= OPENVG_lBGRX_8888 && target->format <= OPENVG_lBGRA_8888_PRE) ||
        (target->format >= OPENVG_lXBGR_8888 && target->format <= OPENVG_lABGR_8888_PRE) ||
        (target->format >= OPENVG_lRGBX_8888_PRE && target->format <= OPENVG_lRGBA_4444_PRE))
    {
        s_context.gamma_dst = 0;
    }
    else
    {
        s_context.gamma_dst = 1;
    }

    if (s_context.gamma_src == 0 && s_context.gamma_dst == 1)
    {
        gamma_value = 0x00002000;
    }
    else if (s_context.gamma_src == 1 && s_context.gamma_dst == 0)
    {
        gamma_value = 0x00001000;
    }
    else
    {
        gamma_value = 0x00000000;
    }

    if (target->image_mode == VG_LITE_STENCIL_MODE)
    {
        s_context.gamma_stencil = gamma_value;
        gamma_value = 0x00000000;
    }

    if (s_context.gamma_dirty == 0 && gamma_value != s_context.gamma_value)
    {
        s_context.gamma_value = gamma_value;
        s_context.gamma_dirty = 1;
    }
}

vg_lite_error_t vg_lite_enable_color_transform()
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_ENABLE_COLOR_TRANSFORM_API);
    DUMP_API_CALL(vg_lite_enable_color_transform);
    VG_LITE_TRACE_API("vg_lite_enable_color_transform\n");

#if gcFEATURE_VG_PIXEL_MATRIX
    vg_lite_error_t error = VG_LITE_SUCCESS;


    s_context.matrix_enable = (1 << 7) | (1 << 23) | (1 << 22) | (1 << 21);
    return error;

#elif gcFEATURE_VG_COLOR_TRANSFORMATION
    vg_lite_error_t error = VG_LITE_SUCCESS;


    s_context.color_transform = (1 << 16);

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_disable_color_transform()
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DISABLE_COLOR_TRANSFORM_API);
    DUMP_API_CALL(vg_lite_disable_color_transform);
    VG_LITE_TRACE_API("vg_lite_disable_color_transform\n");

#if gcFEATURE_VG_PIXEL_MATRIX
    vg_lite_error_t error = VG_LITE_SUCCESS;


    s_context.matrix_enable = 0;
    return error;
#elif gcFEATURE_VG_COLOR_TRANSFORMATION
    vg_lite_error_t error = VG_LITE_SUCCESS;


    s_context.color_transform = 0;

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_set_color_transform(vg_lite_color_transform_t* values)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_COLOR_TRANSFORM_API);
    DUMP_API_CALL(vg_lite_enable_color_transform);
    VG_LITE_TRACE_API("vg_lite_set_color_transform %p\n", values);

#if gcFEATURE_VG_PIXEL_MATRIX
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_pixel_matrix_t matrix = { 0 };
    vg_lite_int16_t pix_matrix[20] = { 0 };
    matrix[0] = values->a_scale;
    matrix[4] = values->a_bias;
    matrix[6] = values->r_scale;
    matrix[9] = values->r_bias;
    matrix[12] = values->g_scale;
    matrix[14] = values->g_bias;
    matrix[18] = values->b_scale;
    matrix[19] = values->b_bias;
    vg_lite_int32_t i = 0;

    if (s_context.matrix_enable)
    {
        for (i = 0; i < 20; i++) {
            if (i % 6 == 0)
            {

                matrix[i] = CLAMP(matrix[i], -127.0f, 127.0f);
            }
            if (i % 5 == 4)
            {
                matrix[i] = CLAMP(matrix[i], -1.0f, 1.0f);
            }
            pix_matrix[i] = (vg_lite_int16_t)(matrix[i] * 256);
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0ADE + i, pix_matrix[i]));
        }
    }

    return error;
#elif gcFEATURE_VG_COLOR_TRANSFORMATION
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_float_t* color_transformations = (vg_lite_float_t*)values;
    vg_lite_int32_t color_elements = 0;
    vg_lite_int16_t temp_transform[8] = { 0 };
    vg_lite_uint32_t final_transform[8] = { 0 };


    for (color_elements = 0; color_elements < 8; color_elements++) {
        if (color_elements % 2) {
            color_transformations[color_elements] = CLAMP(color_transformations[color_elements], -1.0f, 1.0f);
        }
        else {
            color_transformations[color_elements] = CLAMP(color_transformations[color_elements], -127.0f, 127.0f);
        }
        temp_transform[color_elements] = (vg_lite_int16_t)(color_transformations[color_elements] * 256);
        final_transform[color_elements] = (vg_lite_uint32_t)temp_transform[color_elements] & 0x0000FFFF;
    }

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A0C, final_transform[2] | final_transform[3] << 16));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A0D, final_transform[4] | final_transform[5] << 16));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A0E, final_transform[6] | final_transform[7] << 16));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A0F, final_transform[0] | final_transform[1] << 16));

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

#if !gcFEATURE_VG_LVGL_SUPPORT

typedef struct {
    vg_lite_float_t                 r;
    vg_lite_float_t                 g;
    vg_lite_float_t                 b;
    vg_lite_float_t                 a;
} Color;

vg_lite_int32_t colorToInt(vg_lite_float_t c, vg_lite_int32_t maxc)
{
    return MIN(MAX((vg_lite_int32_t)floor((vg_lite_double_t)(c * (vg_lite_float_t)maxc + 0.5f)), 0), maxc);
}

vg_lite_float_t intToColor(vg_lite_uint32_t i, vg_lite_uint32_t maxi)
{
    return (vg_lite_float_t)(i & maxi) / (vg_lite_float_t)maxi;
}

Color readPixel(vg_lite_buffer_t* src, vg_lite_int32_t x, vg_lite_int32_t y)
{
    vg_lite_uint32_t p = 0;
    Color c;
    vg_lite_uint8_t* scanline = (vg_lite_uint8_t*)src->memory + y * src->stride;

    vg_lite_uint8_t bitsPerPixel = 0;
    vg_lite_int32_t rb = 0;
    vg_lite_int32_t gb = 0;
    vg_lite_int32_t bb = 0;
    vg_lite_int32_t ab = 0;
    vg_lite_int32_t rs = 0;
    vg_lite_int32_t gs = 0;
    vg_lite_int32_t bs = 0;
    vg_lite_int32_t as = 0;
    switch (src->format) {
    case VG_LITE_A8:
    case VG_LITE_L8:
        ab = 8;
        bitsPerPixel = 8;
        break;
    case VG_LITE_ABGR4444:
        rs = 12;
        gs = 8;
        bs = 4;
        as = 0;
        rb = 4;
        gb = 4;
        bb = 4;
        ab = 4;
        bitsPerPixel = 16;
        break;
    case VG_LITE_ARGB4444:
        bs = 12;
        gs = 8;
        rs = 4;
        as = 0;
        rb = 4;
        gb = 4;
        bb = 4;
        ab = 4;
        bitsPerPixel = 16;
        break;
    case VG_LITE_RGBA4444:
        as = 12;
        bs = 8;
        gs = 4;
        rs = 0;
        rb = 4;
        gb = 4;
        bb = 4;
        ab = 4;
        bitsPerPixel = 16;
        break;
    case VG_LITE_BGRA4444:
        as = 12;
        rs = 8;
        gs = 4;
        bs = 0;
        rb = 4;
        gb = 4;
        bb = 4;
        ab = 4;
        bitsPerPixel = 16;
        break;
    case VG_LITE_RGB565:
        rs = 0;
        gs = 5;
        bs = 11;
        as = 0;
        rb = 5;
        gb = 6;
        bb = 5;
        ab = 0;
        bitsPerPixel = 16;
        break;
    case VG_LITE_BGR565:
        rs = 11;
        gs = 5;
        bs = 0;
        as = 0;
        rb = 5;
        gb = 6;
        bb = 5;
        ab = 0;
        bitsPerPixel = 16;
        break;
    case VG_LITE_ABGR8888:
    case VG_LITE_XBGR8888:
        rs = 24;
        gs = 16;
        bs = 8;
        as = 0;
        rb = 8;
        gb = 8;
        bb = 8;
        ab = 8;
        bitsPerPixel = 32;
        break;
    case VG_LITE_ARGB8888:
    case VG_LITE_XRGB8888:
        rs = 8;
        gs = 16;
        bs = 24;
        as = 0;
        rb = 8;
        gb = 8;
        bb = 8;
        ab = 8;
        bitsPerPixel = 32;
        break;
    case VG_LITE_RGBA8888:
    case VG_LITE_RGBX8888:
        rs = 0;
        gs = 8;
        bs = 16;
        as = 24;
        rb = 8;
        gb = 8;
        bb = 8;
        ab = 8;
        bitsPerPixel = 32;
        break;
    case VG_LITE_BGRA8888:
    case VG_LITE_BGRX8888:
        rs = 16;
        gs = 8;
        bs = 0;
        as = 24;
        rb = 8;
        gb = 8;
        bb = 8;
        ab = 8;
        bitsPerPixel = 32;
        break;
    case VG_LITE_ABGR1555:
        rs = 11;
        gs = 6;
        bs = 1;
        as = 0;
        rb = 5;
        gb = 5;
        bb = 5;
        ab = 1;
        bitsPerPixel = 16;
        break;
    case VG_LITE_RGBA5551:
        rs = 0;
        gs = 5;
        bs = 10;
        as = 15;
        rb = 5;
        gb = 5;
        bb = 5;
        ab = 1;
        bitsPerPixel = 16;
        break;
    case VG_LITE_ARGB1555:
        rs = 1;
        gs = 6;
        bs = 11;
        as = 0;
        rb = 5;
        gb = 5;
        bb = 5;
        ab = 1;
        bitsPerPixel = 16;
        break;
    case VG_LITE_BGRA5551:
        rs = 10;
        gs = 5;
        bs = 0;
        as = 15;
        rb = 5;
        gb = 5;
        bb = 5;
        ab = 1;
        bitsPerPixel = 16;
        break;
    case VG_LITE_BGRA2222:
        rs = 4;
        gs = 2;
        bs = 0;
        as = 6;
        rb = 2;
        gb = 2;
        bb = 2;
        ab = 2;
        bitsPerPixel = 8;
        break;
    case VG_LITE_RGBA2222:
        rs = 0;
        gs = 2;
        bs = 4;
        as = 6;
        rb = 2;
        gb = 2;
        bb = 2;
        ab = 2;
        bitsPerPixel = 8;
        break;
    case VG_LITE_ABGR2222:
        rs = 6;
        gs = 4;
        bs = 2;
        as = 0;
        rb = 2;
        gb = 2;
        bb = 2;
        ab = 2;
        bitsPerPixel = 8;
        break;
    case VG_LITE_ARGB2222:
        rs = 2;
        gs = 4;
        bs = 6;
        as = 0;
        rb = 2;
        gb = 2;
        bb = 2;
        ab = 2;
        bitsPerPixel = 8;
        break;
    default:
        break;
    }

    switch (bitsPerPixel)
    {
    case 32:
    {
        vg_lite_uint32_t* s = (((vg_lite_uint32_t*)scanline) + x);
        p = (vg_lite_uint32_t)*s;
        break;
    }

    case 16:
    {
        uint16_t* s = ((uint16_t*)scanline) + x;
        p = (vg_lite_uint32_t)*s;
        break;
    }

    case 8:
    {
        vg_lite_uint8_t* s = ((vg_lite_uint8_t*)scanline) + x;
        p = (vg_lite_uint32_t)*s;
        break;
    }
    case 4:
    {
        vg_lite_uint8_t* s = ((vg_lite_uint8_t*)scanline) + (x >> 1);
        p = (vg_lite_uint32_t)(*s >> ((x & 1) << 2)) & 0xf;
        break;
    }
    case 2:
    {
        vg_lite_uint8_t* s = ((vg_lite_uint8_t*)scanline) + (x >> 2);
        p = (vg_lite_uint32_t)(*s >> ((x & 3) << 1)) & 0x3;
        break;
    }
    default:
    {
        vg_lite_uint8_t* s = ((vg_lite_uint8_t*)scanline) + (x >> 3);
        p = (vg_lite_uint32_t)(*s >> (x & 7)) & 0x1;
        break;
    }
    }

    //rgba
    c.r = rb ? intToColor(p >> rs, (1 << rb) - 1) : (vg_lite_float_t)1.0f;
    c.g = gb ? intToColor(p >> gs, (1 << gb) - 1) : (vg_lite_float_t)1.0f;
    c.b = bb ? intToColor(p >> bs, (1 << bb) - 1) : (vg_lite_float_t)1.0f;
    c.a = ab ? intToColor(p >> as, (1 << ab) - 1) : (vg_lite_float_t)1.0f;

    return c;
}

vg_lite_void writePixel(vg_lite_buffer_t* temp, vg_lite_int32_t x, vg_lite_int32_t y, Color* c)
{
    vg_lite_uint8_t bitsPerPixel = 0;
    vg_lite_int32_t rb = 0;
    vg_lite_int32_t gb = 0;
    vg_lite_int32_t bb = 0;
    vg_lite_int32_t ab = 0;
    vg_lite_int32_t rs = 0;
    vg_lite_int32_t gs = 0;
    vg_lite_int32_t bs = 0;
    vg_lite_int32_t as = 0;
    switch (temp->format) {
    case VG_LITE_A8:
    case VG_LITE_L8:
        ab = 8;
        bitsPerPixel = 8;
        break;
    case VG_LITE_ABGR4444:
        rs = 12;
        gs = 8;
        bs = 4;
        as = 0;
        rb = 4;
        gb = 4;
        bb = 4;
        ab = 4;
        bitsPerPixel = 16;
        break;
    case VG_LITE_ARGB4444:
        bs = 12;
        gs = 8;
        rs = 4;
        as = 0;
        rb = 4;
        gb = 4;
        bb = 4;
        ab = 4;
        bitsPerPixel = 16;
        break;
    case VG_LITE_RGBA4444:
        as = 12;
        bs = 8;
        gs = 4;
        rs = 0;
        rb = 4;
        gb = 4;
        bb = 4;
        ab = 4;
        bitsPerPixel = 16;
        break;
    case VG_LITE_BGRA4444:
        as = 12;
        rs = 8;
        gs = 4;
        bs = 0;
        rb = 4;
        gb = 4;
        bb = 4;
        ab = 4;
        bitsPerPixel = 16;
        break;
    case VG_LITE_RGB565:
        rs = 0;
        gs = 5;
        bs = 11;
        as = 0;
        rb = 5;
        gb = 6;
        bb = 5;
        ab = 0;
        bitsPerPixel = 16;
        break;
    case VG_LITE_BGR565:
        rs = 11;
        gs = 5;
        bs = 0;
        as = 0;
        rb = 5;
        gb = 6;
        bb = 5;
        ab = 0;
        bitsPerPixel = 16;
        break;
    case VG_LITE_ABGR8888:
    case VG_LITE_XBGR8888:
        rs = 24;
        gs = 16;
        bs = 8;
        as = 0;
        rb = 8;
        gb = 8;
        bb = 8;
        ab = 8;
        bitsPerPixel = 32;
        break;
    case VG_LITE_ARGB8888:
    case VG_LITE_XRGB8888:
        rs = 8;
        gs = 16;
        bs = 24;
        as = 0;
        rb = 8;
        gb = 8;
        bb = 8;
        ab = 8;
        bitsPerPixel = 32;
        break;
    case VG_LITE_RGBA8888:
    case VG_LITE_RGBX8888:
        rs = 0;
        gs = 8;
        bs = 16;
        as = 24;
        rb = 8;
        gb = 8;
        bb = 8;
        ab = 8;
        bitsPerPixel = 32;
        break;
    case VG_LITE_BGRA8888:
    case VG_LITE_BGRX8888:
        rs = 16;
        gs = 8;
        bs = 0;
        as = 24;
        rb = 8;
        gb = 8;
        bb = 8;
        ab = 8;
        bitsPerPixel = 32;
        break;
    case VG_LITE_ABGR1555:
        rs = 11;
        gs = 6;
        bs = 1;
        as = 0;
        rb = 5;
        gb = 5;
        bb = 5;
        ab = 1;
        bitsPerPixel = 16;
        break;
    case VG_LITE_RGBA5551:
        rs = 0;
        gs = 5;
        bs = 10;
        as = 15;
        rb = 5;
        gb = 5;
        bb = 5;
        ab = 1;
        bitsPerPixel = 16;
        break;
    case VG_LITE_ARGB1555:
        rs = 1;
        gs = 6;
        bs = 11;
        as = 0;
        rb = 5;
        gb = 5;
        bb = 5;
        ab = 1;
        bitsPerPixel = 16;
        break;
    case VG_LITE_BGRA5551:
        rs = 10;
        gs = 5;
        bs = 0;
        as = 15;
        rb = 5;
        gb = 5;
        bb = 5;
        ab = 1;
        bitsPerPixel = 16;
        break;
    case VG_LITE_BGRA2222:
        rs = 4;
        gs = 2;
        bs = 0;
        as = 6;
        rb = 2;
        gb = 2;
        bb = 2;
        ab = 2;
        bitsPerPixel = 8;
        break;
    case VG_LITE_RGBA2222:
        rs = 0;
        gs = 2;
        bs = 4;
        as = 6;
        rb = 2;
        gb = 2;
        bb = 2;
        ab = 2;
        bitsPerPixel = 8;
        break;
    case VG_LITE_ABGR2222:
        rs = 6;
        gs = 4;
        bs = 2;
        as = 0;
        rb = 2;
        gb = 2;
        bb = 2;
        ab = 2;
        bitsPerPixel = 8;
        break;
    case VG_LITE_ARGB2222:
        rs = 2;
        gs = 4;
        bs = 6;
        as = 0;
        rb = 2;
        gb = 2;
        bb = 2;
        ab = 2;
        bitsPerPixel = 8;
        break;
    default:
        break;
    }

    vg_lite_uint32_t cr = rb ? colorToInt(c->r, (1 << rb) - 1) : 0;
    vg_lite_uint32_t cg = gb ? colorToInt(c->g, (1 << gb) - 1) : 0;
    vg_lite_uint32_t cb = bb ? colorToInt(c->b, (1 << bb) - 1) : 0;
    vg_lite_uint32_t ca = ab ? colorToInt(c->a, (1 << ab) - 1) : 0;

    vg_lite_uint32_t p = (cr << rs) | (cg << gs) | (cb << bs) | (ca << as);
    vg_lite_char* scanline = (vg_lite_char*)temp->memory + y * temp->stride;
    switch (bitsPerPixel)
    {
    case 32:
    {
        vg_lite_uint32_t* s = ((vg_lite_uint32_t*)scanline) + x;
        *s = (vg_lite_uint32_t)p;
        break;
    }

    case 16:
    {
        uint16_t* s = ((uint16_t*)scanline) + x;
        *s = (uint16_t)p;
        break;
    }

    case 8:
    {
        vg_lite_char* s = ((vg_lite_char*)scanline) + x;
        *s = (vg_lite_char)p;
        break;
    }
    case 4:
    {
        vg_lite_char* s = ((vg_lite_char*)scanline) + (x >> 1);
        *s = (vg_lite_char)((p << ((x & 1) << 2)) | ((vg_lite_uint32_t)*s & ~(0xf << ((x & 1) << 2))));
        break;
    }

    case 2:
    {
        vg_lite_char* s = ((vg_lite_char*)scanline) + (x >> 2);
        *s = (vg_lite_char)((p << ((x & 3) << 1)) | ((vg_lite_uint32_t)*s & ~(0x3 << ((x & 3) << 1))));
        break;
    }

    default:
    {
        break;
    }
    }
}

vg_lite_void setup_lvgl_image(vg_lite_buffer_t* dst, vg_lite_buffer_t* src, vg_lite_buffer_t* lvgl_buf, vg_lite_blend_t operation)
{
    Color c_src = {0}, c_dst = {0}, c_temp = {0};
    /* copy source region to tmp dst */
    for (vg_lite_int32_t j = 0; j < src->height; j++)
    {
        for (vg_lite_int32_t i = 0; i < src->width; i++)
        {
            c_src = readPixel(src, i, j);
            c_dst = readPixel(dst, i, j);

            switch (operation)
            {
            case VG_LITE_BLEND_NORMAL_LVGL:
                c_temp.a = c_src.a;
                c_temp.r = c_src.a * c_src.r;
                c_temp.g = c_src.a * c_src.g;
                c_temp.b = c_src.a * c_src.b;
                break;
            case VG_LITE_BLEND_ADDITIVE_LVGL:
                c_temp.a = c_src.a;
                c_temp.r = (c_src.r + c_dst.r) * c_src.a;
                c_temp.g = (c_src.g + c_dst.g) * c_src.a;
                c_temp.b = (c_src.b + c_dst.b) * c_src.a;
                break;
            case VG_LITE_BLEND_SUBTRACT_LVGL:
                c_temp.a = c_src.a;
                c_temp.r = c_src.r < c_dst.r ? (c_dst.r - c_src.r) * c_src.a : 0;
                c_temp.g = c_src.g < c_dst.g ? (c_dst.g - c_src.g) * c_src.a : 0;
                c_temp.b = c_src.b < c_dst.b ? (c_dst.b - c_src.b) * c_src.a : 0;
                break;
            case VG_LITE_BLEND_MULTIPLY_LVGL:
                c_temp.a = c_src.a;
                c_temp.r = c_src.r * c_dst.r * c_src.a;
                c_temp.g = c_src.g * c_dst.g * c_src.a;
                c_temp.b = c_src.b * c_dst.b * c_src.a;
                break;
            case VG_LITE_BLEND_DIFFERENCE_LVGL:
                c_temp.a = c_src.a;
                c_temp.r = c_src.r < c_dst.r ? (c_dst.r - c_src.r) * c_src.a : (c_src.r - c_dst.r) * c_src.a;
                c_temp.g = c_src.g < c_dst.g ? (c_dst.g - c_src.g) * c_src.a : (c_src.g - c_dst.g) * c_src.a;
                c_temp.b = c_src.b < c_dst.b ? (c_dst.b - c_src.b) * c_src.a : (c_src.b - c_dst.b) * c_src.a;
                break;
            default:
                break;
            }
            if (lvgl_buf) {
                writePixel(lvgl_buf, i, j, &c_temp);
            }
#if !gcFEATURE_VG_GLOBAL_ALPHA
            c_dst.a = 1.0;
            writePixel(dst, i, j, &c_dst);
#endif
        }
    }
    return;
}

#endif  /* !gcFEATURE_VG_LVGL_SUPPORT */

/****************** FLEXA Support ***************/

vg_lite_error_t vg_lite_flexa_enable()
{
    DUMP_API_CALL(vg_lite_flexa_enable);
    VG_LITE_TRACE_API("vg_lite_flexa_enable\n");

#if gcFEATURE_VG_FLEXA
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_flexa_info_t flexa_data;

    /*setting MULTI_HSYNC*/
    flexa_data.sync_mode = s_context.sync_mode = BIT(1);
    flexa_data.mesh_size = s_context.flexa_mesh_size;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FLEXA_ENABLE, &flexa_data));

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_flexa_disable()
{
    DUMP_API_CALL(vg_lite_flexa_disable);
    VG_LITE_TRACE_API("vg_lite_flexa_disable\n");

#if gcFEATURE_VG_FLEXA
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_flexa_info_t flexa_data;

    flexa_data.sync_mode = s_context.sync_mode = BIT(0);
    flexa_data.stream_id = s_context.stream_id = 0;
    flexa_data.sbi_mode = s_context.sbi_mode = 0x0;
    flexa_data.start_flag = s_context.start_flag = 0x0;
    flexa_data.stop_flag = s_context.stop_flag = 0x0;
    flexa_data.reset_flag = s_context.reset_flag = 0x0;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FLEXA_DISABLE, &flexa_data));

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_flexa_set_consumer(vg_lite_flexa_config_t* flexa_cfg, vg_lite_buffer_t* buffer)
{
    DUMP_API_CALL(vg_lite_flexa_set_consumer, stream_id, buffer, bg_seg_count, bg_seg_size);
#if gcFEATURE_VG_FLEXA
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_flexa_info_t flexa_data;

    DUMP_API_CALL(vg_lite_flexa_set_consumer, stream_id, buffer, bg_seg_count, bg_seg_size);

    flexa_data.sbi_mode = s_context.sbi_mode = 0x1;
    flexa_data.segment_address = s_context.segment_address = buffer->address;
    flexa_data.segment_count = s_context.segment_count = flexa_cfg->seg_count;
    flexa_data.segment_size = s_context.segment_size = flexa_cfg->seg_size;
    flexa_data.mesh_size = s_context.flexa_mesh_size = flexa_cfg->mesh_size << 15;
    flexa_data.plane1_stream_id = flexa_cfg->plane1_stream_id << 1;
    flexa_data.plane2_stream_id = flexa_cfg->plane2_stream_id << 1;
    flexa_data.start_flag = s_context.start_flag = BIT(9);
    flexa_data.reset_flag = s_context.reset_flag = flexa_cfg->offset_reset_mode << 10;
    flexa_data.segment_offset = s_context.segment_offset = flexa_cfg->seg_offset;
    flexa_data.consumer1_start_timeout_mode = s_context.consumer1_start_timeout_mode = flexa_cfg->start_timeout_mode << 12;
    flexa_data.consumer1_request_timeout_mode = s_context.consumer1_request_timeout_mode = flexa_cfg->request_timeout_mode << 14;
    flexa_data.consumer1_consumer_id = s_context.consumer1_consumer_id = flexa_cfg->consumer_id << 16;

    flexa_data.consumer0_start_timeout_mode = s_context.consumer0_start_timeout_mode = flexa_cfg->start_timeout_mode << 12;
    flexa_data.consumer0_request_timeout_mode = s_context.consumer0_request_timeout_mode = flexa_cfg->request_timeout_mode << 14;
    flexa_data.consumer0_consumer_id = s_context.consumer0_consumer_id = flexa_cfg->consumer_id << 16;

    switch (buffer->format)
    {
    case VG_LITE_NV12:
    case VG_LITE_ANV12:
    case VG_LITE_NV12_TILED:
    case VG_LITE_ANV12_TILED:
    case VG_LITE_NV16:
    case VG_LITE_NV24:
    case VG_LITE_NV24_TILED:
        flexa_data.consumer0_set = 1;
        flexa_data.consumer1_set = 0 << 1;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FLEXA_SET_DISTRIBUTE_CONSUMER1, &flexa_data));
        flexa_data.segment_address = s_context.segment_address = buffer->yuv.uv_planar;
        if (buffer->format == VG_LITE_NV16)
            flexa_data.segment_size = s_context.segment_size = flexa_cfg->seg_size;
        else if (buffer->format == VG_LITE_NV24)
            flexa_data.segment_size = s_context.segment_size = flexa_cfg->seg_size * 2;
        else
            flexa_data.segment_size = s_context.segment_size = flexa_cfg->seg_size / 2;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FLEXA_SET_DISTRIBUTE_CONSUMER0, &flexa_data));
        break;
    case VG_LITE_AYUY2:
    case VG_LITE_AYUY2_TILED:
    case VG_LITE_ABGR8565_PLANAR:
    case VG_LITE_BGRA5658_PLANAR:
    case VG_LITE_ARGB8565_PLANAR:
    case VG_LITE_RGBA5658_PLANAR:
        flexa_data.consumer0_set = 1;
        flexa_data.consumer1_set = 0 << 1;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FLEXA_SET_DISTRIBUTE_CONSUMER1, &flexa_data));
        flexa_data.segment_address = s_context.segment_address = buffer->yuv.alpha_planar;
        flexa_data.segment_size = s_context.segment_size = flexa_cfg->seg_size / 2;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FLEXA_SET_DISTRIBUTE_CONSUMER0, &flexa_data));
        break;
    case VG_LITE_YV12:
    case VG_LITE_YV16:
    case VG_LITE_YV24:
        return VG_LITE_NOT_SUPPORT;
    default:
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FLEXA_SET_CONSUMER1_ADDRESS, &flexa_data));
        break;
    }

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_flexa_set_producer(vg_lite_flexa_config_t* flexa_cfg, vg_lite_buffer_t* buffer)
{
    DUMP_API_CALL(vg_lite_flexa_set_producer, stream_id, buffer, bg_seg_count, bg_seg_size);
#if gcFEATURE_VG_FLEXA
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_flexa_info_t flexa_data;

    DUMP_API_CALL(vg_lite_flexa_set_consumer, stream_id, buffer, bg_seg_count, bg_seg_size);

    flexa_data.sbi_mode = s_context.sbi_mode = 0x1;
    flexa_data.segment_address = s_context.segment_address = buffer->address;
    flexa_data.segment_count = s_context.segment_count = flexa_cfg->seg_count;
    flexa_data.segment_size = s_context.segment_size = flexa_cfg->seg_size;
    flexa_data.mesh_size = s_context.flexa_mesh_size = flexa_cfg->mesh_size << 15;
    flexa_data.plane1_stream_id = flexa_cfg->plane1_stream_id << 1;
    flexa_data.start_flag = s_context.start_flag = BIT(9);

    flexa_data.reset_flag = s_context.reset_flag = flexa_cfg->offset_reset_mode << 10;
    flexa_data.segment_offset = s_context.segment_offset = flexa_cfg->seg_offset;
    flexa_data.producer0_start_timeout_mode = s_context.producer0_start_timeout_mode = flexa_cfg->start_timeout_mode << 12;
    flexa_data.producer0_request_timeout_mode = s_context.producer0_request_timeout_mode = flexa_cfg->request_timeout_mode << 14;
    flexa_data.producer0_consumer_id = s_context.producer0_consumer_id = flexa_cfg->consumer_id << 16;

    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FLEXA_SET_PRODUCER_ADDRESS, &flexa_data));

    return error;

#else
    return VG_LITE_NOT_SUPPORT;
#endif

}

vg_lite_error_t vg_lite_flexa_stop_consumer(vg_lite_flexa_config_t* flexa_cfg)
{
    DUMP_API_CALL(vg_lite_flexa_stop_consumer);
    VG_LITE_TRACE_API("vg_lite_flexa_stop_consumer\n");

#if gcFEATURE_VG_FLEXA
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_flexa_info_t flexa_data;

    flexa_data.stop_flag = s_context.stop_flag = BIT(11);
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FLEXA_STOP_CONSUMER, &flexa_data));

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_flexa_stop_producer(vg_lite_flexa_config_t* flexa_cfg)
{
    DUMP_API_CALL(vg_lite_flexa_stop_produer);
    VG_LITE_TRACE_API("vg_lite_flexa_stop_produer\n");

#if gcFEATURE_VG_FLEXA
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_flexa_info_t flexa_data;

    flexa_data.stop_flag = s_context.stop_flag = BIT(11);
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FLEXA_STOP_PRODUCER, &flexa_data));

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}
