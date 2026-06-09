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

#ifndef _vg_lite_chip_h_
#define _vg_lite_chip_h_

#include "vg_lite_context.h"

vg_lite_error_t feature_check_compress(
    vg_lite_buffer_format_t format,
    vg_lite_compress_mode_t compress_mode,
    vg_lite_buffer_layout_t tiled,
    vg_lite_uint32_t width,
    vg_lite_uint32_t height
);

vg_lite_float_t _calc_decnano_compress_ratio(
    vg_lite_buffer_format_t format,
    vg_lite_compress_mode_t compress_mode
);

vg_lite_error_t check_compress_stride_align(
    vg_lite_buffer_format_t format,
    vg_lite_uint32_t stride_mul_height
);

vg_lite_error_t check_compress_target_address_align(vg_lite_uint32_t address);

vg_lite_error_t chip_check_target_format(vg_lite_buffer_format_t format);
#define chip_check_source_format chip_check_target_format

vg_lite_error_t check_draw_pattern_source_format(vg_lite_buffer_format_t format);

vg_lite_void chip_adjust_src_premultiply_enable(vg_lite_uint32_t *src_premultiply_enable);
vg_lite_uint32_t chip_get_mirror_mode(vg_lite_orientation_t mirror_orient);

vg_lite_uint32_t chip_patch_close_command(
    vg_lite_uint8_t curr_cmd,
    vg_lite_uint8_t next_cmd,
    vg_lite_int32_t data_size,
    vg_lite_uint8_t *path_data,
    vg_lite_uint32_t *offset
);

vg_lite_void chip_adjust_interpolation_rounding(
    const vg_lite_matrix_t *matrix,
    const vg_lite_rectangle_t *bounding_box,
    const vg_lite_matrix_t *inverse_matrix,
    const vg_lite_float_t *xs,
    const vg_lite_float_t *ys,
    vg_lite_float_t *dx,
    vg_lite_float_t *dy
);
vg_lite_void chip_setup_tessellation_buffer_layout(vg_lite_context_t *context, vg_lite_uint32_t *tessellation_size);
vg_lite_error_t chip_program_tessellation(vg_lite_context_t *context);

vg_lite_error_t chip_set_rec_tile(vg_lite_buffer_t* target,
    vg_lite_buffer_t* source,
    vg_lite_matrix_t* matrix,
    vg_lite_uint32_t* tile_setting,
    vg_lite_uint32_t* stripe_mode);

vg_lite_error_t chip_set_tes_tile(vg_lite_buffer_t* target, vg_lite_uint32_t* tile_setting);
vg_lite_void set_interpolation_update_csteps(vg_lite_float_t c_step[3], vg_lite_float_t offset0, vg_lite_float_t offset1);

vg_lite_int32_t chip_process_blit_boundary_point(vg_lite_uint32_t rect_x, vg_lite_uint32_t rect_y,
    vg_lite_uint32_t rect_w);

vg_lite_error_t chip_calculate_blit_image_steps(vg_lite_int32_t width, vg_lite_int32_t height, vg_lite_filter_t filter,
    vg_lite_matrix_t* matrix, vg_lite_matrix_t* inverse_matrix, vg_lite_float_t x_step[3], vg_lite_float_t y_step[3],
    vg_lite_float_t c_step[3], vg_lite_uint8_t enable_sw_pre_opt);

vg_lite_void chip_stroke_cpath_process_last_close_opcode(vg_lite_char last_opcode, vg_lite_char_ptr* cpath,
    vg_lite_float_ptr* pfloat, vg_lite_float_t* real_size, vg_lite_uint32_t delta);
vg_lite_void chip_stroke_path_update_last_opcode(vg_lite_char* last_opcode, vg_lite_char cpath);
vg_lite_uint8_t chip_get_cpath_append_close_flag(vg_lite_uint32_t current, vg_lite_uint32_t next);
vg_lite_void chip_cpath_append_close_opcode(vg_lite_uint8_t flag, vg_lite_int32_t data_size, vg_lite_char_ptr cpath);
extern vg_lite_void get_format_bytes(vg_lite_buffer_format_t format, vg_lite_uint32_t* mul, vg_lite_uint32_t* div, vg_lite_uint32_t* bytes_align);

vg_lite_void feature_border_culling_special_process(
    vg_lite_blend_t *blend,
    vg_lite_uint32_t *transparency_mode);

vg_lite_error_t feature_check_8x_csaa_support(vg_lite_quality_t quality);

vg_lite_error_t feature_push_global_alpha_state(vg_lite_context_t *context);
vg_lite_error_t feature_war_legacy_lvgl_blend(vg_lite_blend_t blend, vg_lite_global_alpha_t dest_alpha_mode);

vg_lite_error_t feature_apply_source_global_alpha(vg_lite_context_t *context,
    vg_lite_global_alpha_t alpha_mode, vg_lite_uint8_t alpha_value);
vg_lite_error_t feature_apply_dest_global_alpha(vg_lite_context_t *context,
    vg_lite_global_alpha_t alpha_mode, vg_lite_uint8_t alpha_value);

vg_lite_error_t feature_check_source_rgba8888_etc2_eac(vg_lite_buffer_format_t format, vg_lite_uint32_t width, vg_lite_uint32_t height);
vg_lite_error_t feature_check_source_rgb888_etc2_eac(vg_lite_buffer_format_t format, vg_lite_uint32_t width, vg_lite_uint32_t height);
vg_lite_error_t feature_check_source_packed_yuy_input(vg_lite_buffer_format_t format);
vg_lite_error_t feature_check_source_planar_yuv_input(vg_lite_buffer_format_t format);
vg_lite_error_t feature_check_source_planar_nv24_input(vg_lite_buffer_format_t format);
vg_lite_error_t feature_check_source_ayuv_input(vg_lite_buffer_format_t format);
vg_lite_error_t feature_check_source_yuv_tiled_input(vg_lite_buffer_format_t format);
vg_lite_error_t feature_check_24bit_packed_format(vg_lite_buffer_format_t format);
vg_lite_error_t feature_check_new_blend_mode(vg_lite_blend_t blend);
vg_lite_error_t feature_check_lvgl_blend_mode(vg_lite_blend_t blend);
vg_lite_error_t feature_check_grad_spread_mode(vg_lite_gradient_spreadmode_t spread_mode);
vg_lite_error_t feature_check_mesh_blt_sw_lvgl_blend(vg_lite_blend_t blend, vg_lite_mesh_mode_t mesh_mode);
vg_lite_error_t feature_check_render_target_mesh_for_frame(
    vg_lite_buffer_t *target,
    vg_lite_mesh_mode_t mesh_mode,
    vg_lite_uint32_t mirror_dirty,
    vg_lite_uint32_t mesh_height);
vg_lite_error_t feature_check_mesh_render_target(
    vg_lite_buffer_t *rtbuffer,
    vg_lite_buffer_t *target,
    vg_lite_mesh_mode_t mesh_mode,
    vg_lite_uint8_t mesh_dirty,
    vg_lite_uint32_t mesh_height,
    vg_lite_uint8_t mesh_count);
vg_lite_error_t feature_check_flexa_render_target(
    vg_lite_buffer_t *rtbuffer,
    vg_lite_buffer_t *target,
    vg_lite_uint32_t sync_mode,
    vg_lite_uint8_t flexa_dirty);
vg_lite_error_t feature_check_lvgl_recolor_image_mode(vg_lite_buffer_image_mode_t mode);
vg_lite_error_t feature_check_a124_a8l8_target_format(vg_lite_buffer_format_t format);
vg_lite_error_t feature_check_a124_a8l8_source_format(vg_lite_buffer_format_t format);
vg_lite_uint32_t feature_a124_a8l8_l8_conversion(vg_lite_buffer_format_t target_format, vg_lite_buffer_format_t source_format);
vg_lite_error_t feature_check_24bit_planar_format(vg_lite_buffer_format_t format);
vg_lite_error_t feature_check_stencil_image_mode(vg_lite_buffer_image_mode_t mode);
vg_lite_error_t feature_check_im_dec_input_compress(vg_lite_compress_mode_t compress_mode);
vg_lite_uint32_t feature_set_reg_filed_for_read_destination(vg_lite_buffer_t *target);
vg_lite_error_t feature_check_dst_screen_copy_blend(vg_lite_buffer_t *target, vg_lite_blend_t *blend);

vg_lite_error_t feature_check_target_yuv_output_format(vg_lite_buffer_format_t format);
vg_lite_error_t feature_check_target_rectangle_tiled_out(vg_lite_buffer_layout_t tiled);

vg_lite_error_t feature_check_source_index_endian(vg_lite_buffer_format_t format, vg_lite_index_endian_t index_endian);
vg_lite_void chip_get_source_index_endian_bits(
    vg_lite_buffer_format_t format,
    vg_lite_index_endian_t src_index_endian,
    vg_lite_uint32_t *index_endian);
vg_lite_error_t feature_check_hw_stall_scissor_target();

#endif
