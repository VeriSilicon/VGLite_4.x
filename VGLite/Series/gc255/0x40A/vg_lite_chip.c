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

#include "vg_lite_chip.h"

static vg_lite_float_t offsetTable[7] = { 0, 0.000575f, -0.000575f, 0.0001f, -0.0001f, 0.0000375f, -0.0000375f };

#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW
vg_lite_uint8_t GetIndex(vg_lite_uint32_t RotationStep, vg_lite_uint32_t ScaleValue)
{
    vg_lite_uint8_t index = 0;
    switch (RotationStep) {
        case 0: //rotate 0
            switch (ScaleValue) {
                case 10:
                case 15:
                case 25:
                case 30:
                case 70:
                case 75:
                    index = 1;
                    break;
                case 45:
                case 50:
                case 60:
                case 65:
                case 550:
                    index = 3;
                    break;
                case 55:
                case 250:
                case 350:
                    index = 4;
                    break;
                case 85:
                case 90:
                case 95:
                case 150:
                case 450:
                case 650:
                case 750:
                case 850:
                case 950:
                    index = 5;
                    break;
                case 125:
                    index = 2;
                    break;
                default:
                    index = 0;
                    break;
            }
            break;
        case 2:  //rotate 90
            switch (ScaleValue) {
                case 10:
                    index = 2;
                    break;
                case 15:
                case 25:
                case 30:
                case 45:
                case 75:
                case 85:
                case 90:
                case 95:
                case 150:
                case 250:
                case 350:
                case 450:
                case 550:
                case 850:
                    index = 5;
                    break;
                case 35:
                case 750:
                    index = 4;
                    break;
                case 50:
                    index = 1;
                    break;
                case 55:
                case 60:
                case 65:
                case 70:
                    index = 3;
                    break;
                default:
                    index = 0;
                    break;
            }
            break;
        case 3:  //rotate 135
            switch (ScaleValue) {
                case 10:
                case 15:
                case 20:
                case 35:
                case 45:
                case 50:
                case 60:
                case 75:
                    index = 2;
                    break;
                case 85:
                case 90:
                case 100:
                case 400:
                case 450:
                case 500:
                case 550:
                case 850:
                    index = 4;
                    break;
                default:
                    index = 0;
                    break;
            }
            break;
        case 4:  //rotate 180
            switch (ScaleValue) {
                case 10:
                case 15:
                case 25:
                case 30:
                case 35:
                case 50:
                    index = 1;
                    break;
                case 45:
                case 55:
                case 65:
                case 70:
                case 75:
                case 85:
                case 90:
                case 95:
                case 150:
                case 250:
                case 350:
                case 450:
                case 550:
                case 650:
                case 750:
                case 850:
                case 950:
                    index = 5;
                    break;
                default:
                    index = 0;
                    break;
            }
            break;
        case 5: //rotate 225
            switch (ScaleValue) {
                case 10:
                case 15:
                case 20:
                case 30:
                case 35:
                case 40:
                case 45:
                case 55:
                case 60:
                case 90:
                    index = 6;
                    break;
                default:
                    index = 0;
                    break;
            }
            break;
        case 6: //rotate 270
            switch (ScaleValue) {
                case 10:
                case 25:
                case 30:
                case 35:
                case 45:
                case 55:
                case 60:
                case 65:
                case 70:
                case 75:
                case 80:
                case 85:
                case 90:
                case 95:
                case 150:
                case 350:
                case 450:
                case 550:
                case 650:
                case 750:
                case 850:
                case 950:
                    index = 5;
                    break;
                default:
                    index = 0;
                    break;
            }
            break;
        case 7: //rotate 315
            switch (ScaleValue) {
                case 20:
                case 25:
                case 30:
                case 35:
                case 40:
                case 45:
                case 50:
                case 55:
                case 60:
                case 65:
                case 70:
                case 80:
                case 85:
                case 90:
                case 95:
                case 350:
                case 550:
                case 900:
                    index = 5;
                    break;
                default:
                    index = 0;
                    break;
            }
            break;
        default :
            index = 0;
            break;
    }
    return index;
}
#endif /* gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW */

extern vg_lite_error_t set_interpolation_steps(vg_lite_int32_t s_width,
                                               vg_lite_int32_t s_height,
                                               vg_lite_matrix_t *matrix,
                                               vg_lite_uint8_t push_states,
                                               vg_lite_float_t **steps);

vg_lite_error_t chip_check_target_format(vg_lite_buffer_format_t format)
{
    (vg_lite_void)format;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t check_draw_pattern_source_format(vg_lite_buffer_format_t format)
{
    if (format == VG_LITE_A1 || format == VG_LITE_A2 || format == VG_LITE_A4 ||
        format == VG_LITE_A8 || format == VG_LITE_A8L8 || format == VG_LITE_L4) {
        return VG_LITE_NOT_SUPPORT;
    }
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_lvgl_blend_mode(vg_lite_blend_t blend)
{
    return is_lvgl_blend_mode(blend) ? VG_LITE_NOT_SUPPORT : VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_grad_spread_mode(vg_lite_gradient_spreadmode_t spread_mode)
{
    if (spread_mode == VG_LITE_GRADIENT_SPREAD_REPEAT || spread_mode == VG_LITE_GRADIENT_SPREAD_REFLECT) {
        return VG_LITE_NOT_SUPPORT;
    }
    return VG_LITE_SUCCESS;
}

vg_lite_void chip_adjust_src_premultiply_enable(vg_lite_uint32_t *src_premultiply_enable)
{
    if (src_premultiply_enable != NULL) {
        *src_premultiply_enable = 0x00000000;
    }
}

vg_lite_uint32_t chip_get_mirror_mode(vg_lite_orientation_t mirror_orient)
{
    return (mirror_orient == VG_LITE_ORIENTATION_TOP_BOTTOM) ? (1U << 16) : 0U;
}

vg_lite_uint32_t chip_patch_close_command(
    vg_lite_uint8_t curr_cmd,
    vg_lite_uint8_t next_cmd,
    vg_lite_int32_t data_size,
    vg_lite_uint8_t *path_data,
    vg_lite_uint32_t *offset
)
{
    (vg_lite_void)curr_cmd;
    (vg_lite_void)next_cmd;
    (vg_lite_void)data_size;
    (vg_lite_void)path_data;
    (vg_lite_void)offset;
    return 0;
}

vg_lite_void chip_adjust_interpolation_rounding(
    const vg_lite_matrix_t *matrix,
    const vg_lite_rectangle_t *bounding_box,
    const vg_lite_matrix_t *inverse_matrix,
    const vg_lite_float_t *xs,
    const vg_lite_float_t *ys,
    vg_lite_float_t *dx,
    vg_lite_float_t *dy
)
{
#define ERR_LIMIT 0.0000610351562f
    if (matrix == NULL || bounding_box == NULL || inverse_matrix == NULL ||
        xs == NULL || ys == NULL || dx == NULL || dy == NULL) {
        return;
    }

    /* Check if matrix has rotation or perspective transformations. */
    if (matrix->m[0][1] != 0.0f || matrix->m[1][0] != 0.0f ||
        matrix->m[2][0] != 0.0f || matrix->m[2][1] != 0.0f ||
        matrix->m[2][2] != 1.0f) {
        if (xs[0] != 0.0f && -ERR_LIMIT < xs[0] && xs[0] < ERR_LIMIT) {
            *dx = 0.5f * (2 * bounding_box->x + bounding_box->width) * inverse_matrix->m[0][0];
        }
        else if (ys[0] != 0.0f && -ERR_LIMIT < ys[0] && ys[0] < ERR_LIMIT) {
            *dx = 0.5f * (2 * bounding_box->y + bounding_box->height) * inverse_matrix->m[0][1];
        }
        if (xs[1] != 0.0f && -ERR_LIMIT < xs[1] && xs[1] < ERR_LIMIT) {
            *dy = 0.5f * (2 * bounding_box->x + bounding_box->width) * inverse_matrix->m[1][0];
        }
        else if (ys[1] != 0.0f && -ERR_LIMIT < ys[1] && ys[1] < ERR_LIMIT) {
            *dy = 0.5f * (2 * bounding_box->y + bounding_box->height) * inverse_matrix->m[1][1];
        }
    }
#undef ERR_LIMIT
}

vg_lite_void chip_setup_tessellation_buffer_layout(vg_lite_context_t *context, vg_lite_uint32_t *tessellation_size)
{
    vg_lite_uint32_t width;
    if (context == NULL || tessellation_size == NULL) {
        return;
    }

    width = (context->tessbuf.tess_w_h & 0xFFFF);
    context->tessbuf.tess_stride = VG_LITE_ALIGN(width * 8, 64);

    /* Each bit in the L1 cache represents 64 bytes of tessellation data. */
    context->tessbuf.L1_size = VG_LITE_ALIGN(VG_LITE_ALIGN(context->tessbuf.tessbuf_size / 64, 64) / 8, 64);
    *tessellation_size = context->tessbuf.L1_size;
}

vg_lite_error_t chip_program_tessellation(vg_lite_context_t *context)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t tessellation_size = 0;

    chip_setup_tessellation_buffer_layout(context, &tessellation_size);

    context->tessbuf.L1_phyaddr = context->tessbuf.physical_addr + context->tessbuf.tessbuf_size;
    context->tessbuf.L2_phyaddr = context->tessbuf.L1_phyaddr + context->tessbuf.L1_size;
    context->tessbuf.L1_logical = context->tessbuf.logical_addr + context->tessbuf.tessbuf_size;
    context->tessbuf.L2_logical = context->tessbuf.L1_logical + context->tessbuf.L1_size;

    /* Program tessellation buffer: input for VG module. */
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A30, context->tessbuf.physical_addr));   /* Tessellation buffer address. */
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A31, context->tessbuf.L1_phyaddr));   /* L1 address of tessellation buffer. */
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A32, context->tessbuf.L2_phyaddr));   /* L2 address of tessellation buffer. */
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A33, context->tessbuf.tess_stride));

    /* Program tessellation control: for TS module. */
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A35, context->tessbuf.physical_addr));
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A36, context->tessbuf.L1_phyaddr));
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A37, context->tessbuf.L2_phyaddr));
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A38, context->tessbuf.tess_stride));
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A3A, context->tessbuf.tess_w_h));


    VG_LITE_RETURN_ERROR(push_state(context, 0x0A3D, tessellation_size / 64));

    return error;
}

static vg_lite_error_t vg_rec_tile_set(vg_lite_char support, vg_lite_buffer_t* target, vg_lite_buffer_t* source, vg_lite_matrix_t* matrix, vg_lite_uint32_t* tile_setting, vg_lite_uint32_t* stripe_mode) {
    if (support)
    {
        if ((source->tiled == VG_LITE_TILED) || (((matrix->angle - 49.0) >= 0.0f) && ((matrix->angle - 133.0) <= 0.0f)) || (((matrix->angle - 229.0) >= 0.0f) && ((matrix->angle - 313.0) <= 0.0f))
            || (target->tiled == VG_LITE_TILED && (target->screen_copy == 1 || target->compress_mode != VG_LITE_DEC_DISABLE))) {
            *tile_setting = 0x40;
            *stripe_mode = 0x20000000;
        }
    }
    else
    {
        if (target->tiled == VG_LITE_TILED) {
            * tile_setting = 0x40;
            *stripe_mode = 0x20000000;
        }
    }
    return VG_LITE_SUCCESS;
}


vg_lite_error_t chip_set_rec_tile(vg_lite_buffer_t* target,
    vg_lite_buffer_t* source,
    vg_lite_matrix_t* matrix,
    vg_lite_uint32_t* tile_setting,
    vg_lite_uint32_t* stripe_mode)
{
    vg_lite_char support;
    support = 0;
    return vg_rec_tile_set(support, target, source, matrix, tile_setting, stripe_mode);
}

vg_lite_error_t vg_tes_tile_set(vg_lite_char support, vg_lite_buffer_t* target, vg_lite_uint32_t* tile_setting) {
    if (support)
    {
    }
    else
    {
        if (target->tiled == VG_LITE_TILED) {
            return VG_LITE_NOT_SUPPORT;
        }
    }
    return VG_LITE_SUCCESS;
}


vg_lite_error_t chip_set_tes_tile(vg_lite_buffer_t* target, vg_lite_uint32_t* tile_setting)
{
    vg_lite_char support;
    support = 0;

    return vg_tes_tile_set(support, target, tile_setting);
}

vg_lite_void set_interpolation_update_csteps(vg_lite_float_t c_step[3], vg_lite_float_t offset0, vg_lite_float_t offset1)
{
    c_step[0] = c_step[0] + offset0;
    c_step[1] = c_step[1] + offset1;
}

vg_lite_int32_t chip_process_blit_boundary_point(vg_lite_uint32_t rect_x, vg_lite_uint32_t rect_y,
    vg_lite_uint32_t rect_w)
{
    vg_lite_int32_t ret = rect_w;
#if gcFEATURE_COMBO_VG_BLIT_PRECISION_OPT_BY_SW && gcFEATURE_VG_BOUNDARY_FILTER_BYPASS && gcFEATURE_VG_16PIXELS_ALIGNED
    if (tem_rect_w && rect_x == 0 && rect_y == 0) {
        ret = (rect_w + 15) & ~15;
    }
#endif
    return ret;
}

vg_lite_error_t chip_calculate_blit_image_steps(vg_lite_int32_t width, vg_lite_int32_t height, vg_lite_filter_t filter,
    vg_lite_matrix_t* matrix, vg_lite_matrix_t* inverse_matrix, vg_lite_float_t x_step[3], vg_lite_float_t y_step[3],
    vg_lite_float_t c_step[3], vg_lite_uint8_t enable_sw_pre_opt)
{
    vg_lite_float_t* steps[3];
    vg_lite_error_t error = VG_LITE_SUCCESS;

    steps[0] = x_step;
    steps[1] = y_step;
    steps[2] = c_step;

    VG_LITE_RETURN_ERROR(set_interpolation_steps(width, height, matrix, 0, steps));
    set_interpolation_update_csteps(c_step, offsetTable[0], offsetTable[0]);

    return error;
}

vg_lite_void chip_stroke_cpath_process_last_close_opcode(vg_lite_char last_opcode, vg_lite_char_ptr* cpath,
    vg_lite_float_ptr* pfloat, vg_lite_float_t* real_size, vg_lite_uint32_t delta)
{
    vg_lite_float_ptr ifloat = *pfloat;
    vg_lite_char_ptr ipath = NULL;

    *real_size += (vg_lite_float_t)delta;

    if (last_opcode == VLC_OP_CLOSE) {
        ipath = (vg_lite_char_ptr)(ifloat - 1) + 1;
        *ipath++ = VLC_OP_MOVE;
        ipath = (vg_lite_char_ptr)ifloat;
        *real_size -= 4;
    }
    else
    {
        ipath = (vg_lite_char_ptr)ifloat;
        *ipath = VLC_OP_MOVE;
        ifloat++;
    }

    *cpath = ipath;
    *pfloat = ifloat;
}

vg_lite_void chip_stroke_path_update_last_opcode(vg_lite_char* last_opcode, vg_lite_char cpath)
{
    *last_opcode = cpath;
}

vg_lite_uint8_t chip_get_cpath_append_close_flag(vg_lite_uint32_t current, vg_lite_uint32_t next)
{
    return VGL_FALSE;
}

vg_lite_void chip_cpath_append_close_opcode(vg_lite_uint8_t flag, vg_lite_int32_t data_size, vg_lite_char_ptr cpath)
{
    (vg_lite_void)flag;
    (vg_lite_void)data_size;
    *cpath = VLC_OP_CLOSE;
}


vg_lite_error_t feature_check_compress(
    vg_lite_buffer_format_t format,
    vg_lite_compress_mode_t compress_mode,
    vg_lite_buffer_layout_t tiled,
    vg_lite_uint32_t width,
    vg_lite_uint32_t height
)
{
    (vg_lite_void)format;
    (vg_lite_void)compress_mode;
    (vg_lite_void)tiled;
    (vg_lite_void)width;
    (vg_lite_void)height;
    return VG_LITE_SUCCESS;
}

vg_lite_float_t _calc_decnano_compress_ratio(
    vg_lite_buffer_format_t format,
    vg_lite_compress_mode_t compress_mode
)
{
    (vg_lite_void)format;
    (vg_lite_void)compress_mode;
    return -1.0f;
}

vg_lite_error_t check_compress_stride_align(
    vg_lite_buffer_format_t format,
    vg_lite_uint32_t stride_mul_height
)
{
    (vg_lite_void)format;
    (vg_lite_void)stride_mul_height;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t check_compress_target_address_align(vg_lite_uint32_t address)
{
    (vg_lite_void)address;
    return VG_LITE_SUCCESS;
}

vg_lite_void feature_border_culling_special_process(
    vg_lite_blend_t *blend,
    vg_lite_uint32_t *transparency_mode)
{
    if (blend == NULL || transparency_mode == NULL) {
        return;
    }
    (vg_lite_void)blend;
    /* Mark that we have rotation (border culling path). */
    *transparency_mode = 0x8000;
}

vg_lite_error_t feature_check_8x_csaa_support(vg_lite_quality_t quality)
{
    (vg_lite_void)quality;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_apply_source_global_alpha(vg_lite_context_t *context,
    vg_lite_global_alpha_t alpha_mode, vg_lite_uint8_t alpha_value)
{
    (vg_lite_void)context;
    (vg_lite_void)alpha_mode;
    (vg_lite_void)alpha_value;
    return VG_LITE_NOT_SUPPORT;
}

vg_lite_error_t feature_apply_dest_global_alpha(vg_lite_context_t *context,
    vg_lite_global_alpha_t alpha_mode, vg_lite_uint8_t alpha_value)
{
    (vg_lite_void)context;
    (vg_lite_void)alpha_mode;
    (vg_lite_void)alpha_value;
    return VG_LITE_NOT_SUPPORT;
}

vg_lite_error_t feature_check_source_rgba8888_etc2_eac(vg_lite_buffer_format_t format, vg_lite_uint32_t width, vg_lite_uint32_t height)
{
    (vg_lite_void)width;
    (vg_lite_void)height;
    return (format != VG_LITE_RGBA8888_ETC2_EAC) ? VG_LITE_SUCCESS : VG_LITE_NOT_SUPPORT;
}

vg_lite_error_t feature_check_source_rgb888_etc2_eac(vg_lite_buffer_format_t format, vg_lite_uint32_t width, vg_lite_uint32_t height)
{
    (vg_lite_void)width;
    (vg_lite_void)height;
    return (format != VG_LITE_RGB888_ETC2_EAC) ? VG_LITE_SUCCESS : VG_LITE_NOT_SUPPORT;
}

vg_lite_error_t feature_check_source_packed_yuy_input(vg_lite_buffer_format_t format)
{
    (vg_lite_void)format;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_source_planar_yuv_input(vg_lite_buffer_format_t format)
{
    return ((format >= VG_LITE_NV12 && format <= VG_LITE_NV16) || format == VG_LITE_NV24)
        ? VG_LITE_NOT_SUPPORT
        : VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_source_planar_nv24_input(vg_lite_buffer_format_t format)
{
    (vg_lite_void)format;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_source_ayuv_input(vg_lite_buffer_format_t format)
{
    return (format == VG_LITE_ANV12 || format == VG_LITE_AYUY2)
        ? VG_LITE_NOT_SUPPORT
        : VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_source_yuv_tiled_input(vg_lite_buffer_format_t format)
{
    if ((format >= VG_LITE_YUY2_TILED && format <= VG_LITE_AYUY2_TILED) || format == VG_LITE_NV24_TILED) {
        return VG_LITE_NOT_SUPPORT;
    }
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_target_yuv_output_format(vg_lite_buffer_format_t format)
{
    if (format == VG_LITE_YUY2 ||
        format == VG_LITE_AYUY2 ||
        format == VG_LITE_YUY2_TILED ||
        format == VG_LITE_AYUY2_TILED) {
        return VG_LITE_NOT_SUPPORT;
    }
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_source_index_endian(vg_lite_buffer_format_t format, vg_lite_index_endian_t index_endian)
{
    return (((format >= VG_LITE_INDEX_1) && (format <= VG_LITE_INDEX_4) && index_endian)
        ? VG_LITE_NOT_SUPPORT
        : VG_LITE_SUCCESS);
}

vg_lite_void chip_get_source_index_endian_bits(
    vg_lite_buffer_format_t format,
    vg_lite_index_endian_t src_index_endian,
    vg_lite_uint32_t *index_endian)
{
    (vg_lite_void)format;
    (vg_lite_void)src_index_endian;
    (vg_lite_void)index_endian;
}

vg_lite_error_t feature_check_24bit_packed_format(vg_lite_buffer_format_t format)
{
    return (format >= VG_LITE_RGB888 && format <= VG_LITE_RGBA5658)
        ? VG_LITE_NOT_SUPPORT
        : VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_new_blend_mode(vg_lite_blend_t blend)
{
    return (blend == VG_LITE_BLEND_DARKEN || blend == VG_LITE_BLEND_LIGHTEN)
        ? VG_LITE_NOT_SUPPORT
        : VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_mesh_blt_sw_lvgl_blend(vg_lite_blend_t blend, vg_lite_mesh_mode_t mesh_mode)
{
    (vg_lite_void)blend;
    (vg_lite_void)mesh_mode;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_render_target_mesh_for_frame(
    vg_lite_buffer_t *target,
    vg_lite_mesh_mode_t mesh_mode,
    vg_lite_uint32_t mirror_dirty,
    vg_lite_uint32_t mesh_height)
{
    (vg_lite_void)target;
    (vg_lite_void)mesh_mode;
    (vg_lite_void)mirror_dirty;
    (vg_lite_void)mesh_height;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_mesh_render_target(
    vg_lite_buffer_t *rtbuffer,
    vg_lite_buffer_t *target,
    vg_lite_mesh_mode_t mesh_mode,
    vg_lite_uint8_t mesh_dirty,
    vg_lite_uint32_t mesh_height,
    vg_lite_uint8_t mesh_count)
{
    (vg_lite_void)rtbuffer;
    (vg_lite_void)target;
    (vg_lite_void)mesh_mode;
    (vg_lite_void)mesh_dirty;
    (vg_lite_void)mesh_height;
    (vg_lite_void)mesh_count;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_flexa_render_target(
    vg_lite_buffer_t *rtbuffer,
    vg_lite_buffer_t *target,
    vg_lite_uint32_t sync_mode,
    vg_lite_uint8_t flexa_dirty)
{
    (vg_lite_void)rtbuffer;
    (vg_lite_void)target;
    (vg_lite_void)sync_mode;
    (vg_lite_void)flexa_dirty;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_target_rectangle_tiled_out(vg_lite_buffer_layout_t tiled)
{
    (vg_lite_void)tiled;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_lvgl_recolor_image_mode(vg_lite_buffer_image_mode_t mode)
{
    return (mode == VG_LITE_RECOLOR_MODE) ? VG_LITE_NOT_SUPPORT : VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_a124_a8l8_target_format(vg_lite_buffer_format_t format)
{
    return (format == VG_LITE_A1 || format == VG_LITE_A2 || format == VG_LITE_A4 || format == VG_LITE_A8L8
        || format == VG_LITE_L4)
        ? VG_LITE_NOT_SUPPORT
        : VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_a124_a8l8_source_format(vg_lite_buffer_format_t format)
{
    return (format == VG_LITE_A1 || format == VG_LITE_A2 || format == VG_LITE_A8L8 || format == VG_LITE_L4)
        ? VG_LITE_NOT_SUPPORT
        : VG_LITE_SUCCESS;
}

vg_lite_uint32_t feature_a124_a8l8_l8_conversion(vg_lite_buffer_format_t target_format, vg_lite_buffer_format_t source_format)
{
    if ((target_format == VG_LITE_L8) && ((source_format != VG_LITE_L8) && (source_format != VG_LITE_A8))) {
        return 0x80000000;
    }
    return 0;
}

vg_lite_error_t feature_check_24bit_planar_format(vg_lite_buffer_format_t format)
{
    return (format >= VG_LITE_ABGR8565_PLANAR && format <= VG_LITE_RGBA5658_PLANAR)
        ? VG_LITE_NOT_SUPPORT
        : VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_stencil_image_mode(vg_lite_buffer_image_mode_t mode)
{
    return (mode == VG_LITE_STENCIL_MODE) ? VG_LITE_NOT_SUPPORT : VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_im_dec_input_compress(vg_lite_compress_mode_t compress_mode)
{
    return (compress_mode != VG_LITE_DEC_DISABLE) ? VG_LITE_NOT_SUPPORT : VG_LITE_SUCCESS;
}

vg_lite_uint32_t feature_set_reg_filed_for_read_destination(vg_lite_buffer_t *target)
{
    return (!target->screen_copy && target->compress_mode) ? 0x00100000 : 0;
}

vg_lite_error_t feature_check_dst_screen_copy_blend(vg_lite_buffer_t *target, vg_lite_blend_t *blend)
{
    if (!target->screen_copy) {
        return VG_LITE_SUCCESS;
    }
    if (!target->compress_mode) {
        return VG_LITE_INVALID_ARGUMENT;
    }
    if (*blend > VG_LITE_BLEND_NONE && *blend <= VG_LITE_BLEND_MULTIPLY_LVGL) {
        *blend = VG_LITE_BLEND_NONE;
    }
    if (*blend > OPENVG_BLEND_SRC && *blend <= OPENVG_BLEND_ADDITIVE) {
        *blend = OPENVG_BLEND_SRC;
    }
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_push_global_alpha_state(vg_lite_context_t *context)
{
    (vg_lite_void)context;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_war_legacy_lvgl_blend(vg_lite_blend_t blend, vg_lite_global_alpha_t dest_alpha_mode)
{
    (vg_lite_void)blend;
    (vg_lite_void)dest_alpha_mode;
    return VG_LITE_SUCCESS;
}

vg_lite_error_t feature_check_hw_stall_scissor_target()
{
    vg_flush_previous_rt();
    return VG_LITE_SUCCESS;
}
