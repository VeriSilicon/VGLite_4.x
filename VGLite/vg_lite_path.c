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

/* Path data operations. */
#define CDALIGN(value, by) (((value) + (by) - 1) & ~((by) - 1))
#define CDMIN(x, y) ((x) > (y) ? (y) : (x))
#define CDMAX(x, y) ((x) > (y) ? (x) : (y))


extern vg_lite_uint32_t transform(vg_lite_point_t* result, vg_lite_float_t x, vg_lite_float_t y, vg_lite_matrix_t* matrix);
extern vg_lite_uint32_t convert_blend(vg_lite_blend_t blend);
extern vg_lite_uint32_t inverse(vg_lite_matrix_t* result, vg_lite_matrix_t* matrix);
extern vg_lite_uint32_t convert_yuv2rgb(vg_lite_yuv2rgb_t yuv);
extern vg_lite_uint32_t convert_uv_swizzle(vg_lite_swizzle_t swizzle);
extern vg_lite_uint32_t convert_source_format(vg_lite_buffer_format_t format);
extern vg_lite_void get_format_bytes(vg_lite_buffer_format_t format, vg_lite_uint32_t* mul, vg_lite_uint32_t* div, vg_lite_uint32_t* bytes_align);
extern vg_lite_error_t srcbuf_align_check(vg_lite_buffer_t* source);
extern vg_lite_void config_factor_parameter(vg_lite_blend_t blend, vg_lite_porter_duff_config_t porter_duff_config, vg_factor_config_t* factor_config);

extern vg_lite_matrix_t identity_mtx;

/* Convert VGLite data format to HW value. */
static vg_lite_uint32_t convert_path_format(vg_lite_format_t format)
{
    switch (format) {
        case VG_LITE_S8:
            return 0;
            
        case VG_LITE_S16:
            return 0x100000;
            
        case VG_LITE_S32:
            return 0x200000;
            
        case VG_LITE_FP32:
            return 0x300000;
            
        default:
            return 0;
    }
}

/* Convert VGLite quality enums to HW values. */
static vg_lite_uint32_t convert_path_quality(vg_lite_quality_t quality)
{
    switch (quality) {
        case VG_LITE_HIGH:
            return 0x3;
            
        case VG_LITE_UPPER:
            return 0x2;
            
        case VG_LITE_MEDIUM:
            return 0x1;
            
        default:
            return 0x0;
    }
}

static vg_lite_int32_t get_data_count(vg_lite_uint8_t cmd)
{
    static vg_lite_int32_t count[] = {
        0,
        0,
        2,
        2,
        2,
        2,
        4,
        4,
        6,
        6,
        0,
        1,
        1,
        1,
        1,
        2,
        2,
        4,
        4,
        5,
        5,
        5,
        5,
        5,
        5,
        5,
        5
    };

    if (cmd > VLC_OP_LCWARC_REL) {
        return -1;
    }
    else {
        return count[cmd];
    }
}

static vg_lite_void compute_pathbounds(vg_lite_float_t* xmin, vg_lite_float_t* ymin, vg_lite_float_t* xmax, vg_lite_float_t* ymax, vg_lite_float_t x, vg_lite_float_t y)
{
    if (xmin != NULL)
    {
        *xmin = *xmin < x ? *xmin : x;
    }

    if (xmax != NULL)
    {
        *xmax = *xmax > x ? *xmax : x;
    }

    if (ymin != NULL)
    {
        *ymin = *ymin < y ? *ymin : y;
    }

    if (ymax != NULL)
    {
        *ymax = *ymax > y ? *ymax : y;
    }
}

static vg_lite_error_t push_state_tess_path_ts_regs(vg_lite_uint32_t tessellation_size,
    vg_lite_uint32_t xy, vg_lite_uint32_t wh, vg_lite_int32_t grid_mode)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00011000));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A39, xy));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3D, tessellation_size / 64));
    if (grid_mode)
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, xy));
    else
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3A, wh));
    return error;
}

vg_lite_int32_t get_data_size(vg_lite_format_t format)
{
    vg_lite_int32_t data_size = 0;

    switch (format) {
    case VG_LITE_S8:
        data_size = sizeof(int8_t);
        break;

    case VG_LITE_S16:
        data_size = sizeof(int16_t);
        break;

    case VG_LITE_S32:
        data_size = sizeof(vg_lite_int32_t);
        break;

    default:
        data_size = sizeof(vg_lite_float_t);
        break;
    }

    return data_size;
}

static vg_lite_void path_replace_trailing_close_op(vg_lite_pointer path_data, vg_lite_int32_t num, vg_lite_int32_t data_size)
{
    vg_lite_char *last_cmd;

    if (path_data == NULL || num < 1 || data_size < 1)
        return;
    last_cmd = (vg_lite_char *)path_data + (num - 1) * data_size;
    if (*last_cmd == VLC_OP_CLOSE)
        *last_cmd = VLC_OP_END;
}

vg_lite_error_t vg_lite_init_path(vg_lite_path_t* path,
                                vg_lite_format_t data_format,
                                vg_lite_quality_t quality,
                                vg_lite_uint32_t path_length,
                                vg_lite_pointer path_data,
                                vg_lite_float_t min_x, vg_lite_float_t min_y,
                                vg_lite_float_t max_x, vg_lite_float_t max_y)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_INIT_PATH_API);

    vg_lite_int32_t data_size, num = 0;

    VG_LITE_CHECK_NULL_POINTER(path);

    memset(path, 0, sizeof(*path));
    path->format = data_format;
    path->quality = quality;
    path->bounding_box[0] = min_x;
    path->bounding_box[1] = min_y;
    path->bounding_box[2] = max_x;
    path->bounding_box[3] = max_y;

    /* Path data cannot end with a CLOSE op. Replace CLOSE with END for path_data */
    data_size = get_data_size(data_format);
    num = path_length / data_size;

    path_replace_trailing_close_op(path_data, num, data_size);

    path->path_length = path_length;
    path->path = path_data;

    path->path_changed = 1;
    path->uploaded.address = 0;
    path->uploaded.bytes = 0;
    path->uploaded.handle = NULL;
    path->uploaded.memory = NULL;
    path->pdata_internal = 0;
    path->pdata_memory_size = 0;
    s_context.path_lastX = 0;
    s_context.path_lastY = 0;
    /* Default FILL path type*/
    path->path_type = VG_LITE_DRAW_FILL_PATH;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_set_path_type(vg_lite_path_t* path, vg_lite_path_type_t path_type)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SET_PATH_TYPE_API);

    VG_LITE_CHECK_NULL_POINTER(path);
    if (path_type != VG_LITE_DRAW_FILL_PATH &&
        path_type != VG_LITE_DRAW_STROKE_PATH &&
        path_type != VG_LITE_DRAW_FILL_STROKE_PATH) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    path->path_type = path_type;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_clear_path(vg_lite_path_t* path)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    DUMP_LAST_FRAME_CAPTURE(VG_LITE_CLEAR_PATH_API);

    if (path->uploaded.handle != NULL) {
        vg_lite_kernel_free_t free_cmd;
        free_cmd.memory_handle = path->uploaded.handle;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free_cmd));
    }

    path->uploaded.address = 0;
    path->uploaded.bytes = 0;
    path->uploaded.handle = NULL;
    path->uploaded.memory = NULL;
    path->append_path_flag = 0;

    if (path->pdata_internal == 1 && path->path != NULL) {
        vg_lite_os_free(path->path);
        path->pdata_internal = 0;
    }
    path->path = NULL;
    path->pdata_memory_size = 0;

    if (path->stroke_path) {
        vg_lite_os_free(path->stroke_path);
        path->stroke_path = NULL;
    }

#if gcFEATURE_VG_STROKE_PATH
    if (path->stroke) {
        if (path->stroke->path_list_divide) {
            vg_lite_path_list_ptr cur_list;
            while (path->stroke->path_list_divide) {
                cur_list = path->stroke->path_list_divide->next;
                if (path->stroke->path_list_divide->path_points) {
                    vg_lite_path_point_ptr temp_point;
                    while (path->stroke->path_list_divide->path_points) {
                        temp_point = path->stroke->path_list_divide->path_points->next;
                        vg_lite_os_free(path->stroke->path_list_divide->path_points);
                        path->stroke->path_list_divide->path_points = temp_point;
                    }
                    temp_point = NULL;
                }
                vg_lite_os_free(path->stroke->path_list_divide);
                path->stroke->path_list_divide = cur_list;
            }
            cur_list = 0;
        }

        if (path->stroke->stroke_paths) {
            vg_lite_sub_path_ptr temp_sub_path;
            while (path->stroke->stroke_paths) {
                temp_sub_path = path->stroke->stroke_paths->next;
                if (path->stroke->stroke_paths->point_list) {
                    vg_lite_path_point_ptr temp_point;
                    while (path->stroke->stroke_paths->point_list) {
                        temp_point = path->stroke->stroke_paths->point_list->next;
                        vg_lite_os_free(path->stroke->stroke_paths->point_list);
                        path->stroke->stroke_paths->point_list = temp_point;
                    }
                    temp_point = NULL;
                }
                vg_lite_os_free(path->stroke->stroke_paths);
                path->stroke->stroke_paths = temp_sub_path;
            }
            temp_sub_path = NULL;
        }

        if (path->stroke->dash_pattern)
            vg_lite_os_free(path->stroke->dash_pattern);

        vg_lite_os_free(path->stroke);
        path->stroke = NULL;
        path->stroke_valid = 0;


        path->stroke_size = 0;
    }
#endif

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_upload_path(vg_lite_path_t * path)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_UPLOAD_PATH_API);
    DUMP_API_CALL(vg_lite_upload_path, path);

#if VG_PRE_UPLOAD_PATH_SUPPORT
    if (path->path_length && path->path_length > 64 && path->append_path_flag)
        return VG_LITE_SUCCESS;
#endif

    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t bytes;
    vg_lite_buffer_t Buf = {0}, *buffer;

    buffer = &Buf;

    /* Compute the number of bytes required for path + command buffer prefix/postfix. */
    bytes = (8 + path->path_length + 7 + 8) & ~7;

    /* Allocate GPU memory. */
    buffer->width  = bytes;
    buffer->height = 1;
    buffer->stride = 0;
    buffer->format = VG_LITE_A8;
    VG_LITE_RETURN_ERROR(vg_lite_allocate(buffer));

    VG_LITE_CHECK_NULL_POINTER(buffer->memory);

    /* Initialize command buffer prefix. */
    ((vg_lite_uint32_t *) buffer->memory)[0] = VG_LITE_DATA((path->path_length + 7) / 8);
    ((vg_lite_uint32_t *) buffer->memory)[1] = 0;
    
    /* Copy the path data. */
    memcpy((vg_lite_uint32_t *) buffer->memory + 2, path->path, path->path_length);

    /* Initialize command buffer postfix. */
    ((vg_lite_uint32_t *) buffer->memory)[(bytes >> 2) - 2] = VG_LITE_RETURN();
    ((vg_lite_uint32_t *) buffer->memory)[(bytes >> 2) - 1] = 0;

    /* Mark path as uploaded. */
    if (path->pdata_internal) {
        if(path->path)
            vg_lite_os_free(path->path);
        path->pdata_internal = 0;
        path->pdata_memory_size = 0;
    }
    
    path->path = buffer->memory;
    path->uploaded.handle = buffer->handle;
    path->uploaded.address = buffer->address;
    path->uploaded.memory = buffer->memory;
    path->uploaded.bytes = bytes;
    path->path_changed = 0;
    VLM_PATH_ENABLE_UPLOAD(*path);      /* Implicitly enable path uploading. */
    
    /* Return pointer to vg_lite_buffer structure. */
    return error;
}

vg_lite_uint32_t vg_lite_get_path_length(vg_lite_uint8_t *cmd, vg_lite_uint32_t count, vg_lite_format_t format)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_GET_PATH_LENGTH_API);

    vg_lite_uint32_t size = 0;
    vg_lite_int32_t dCount = 0;
    vg_lite_uint32_t i = 0;
    vg_lite_int32_t data_size = 0;
    
    data_size = get_data_size(format);
    
    for (i = 0; i < count; i++) {
        size++;     /* OP CODE. */
        
        dCount = get_data_count(cmd[i]);
        size = CDALIGN(size, data_size);
        size += dCount * data_size;

    }
    if (cmd[count - 1] == VLC_OP_END) {
        return size;
    }
    else {
        size++;
        size = CDALIGN(size, data_size);
    }
    
    return size;
}

vg_lite_error_t vg_lite_append_path(vg_lite_path_t *path,
                                    vg_lite_uint8_t *cmd,
                                    vg_lite_pointer data,
                                    vg_lite_uint32_t seg_count)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_APPEND_PATH_API);

    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t i;
    vg_lite_int32_t j;
    vg_lite_uint32_t offset = 0;
    vg_lite_int32_t dataCount = 0;
    vg_lite_float_t *dataf = (vg_lite_float_t*) data;
    vg_lite_float_t *pathf = NULL;
    vg_lite_int32_t *data_s32 = (vg_lite_int32_t*) data;
    vg_lite_int32_t *path_s32 = NULL;
    int16_t *data_s16 = (int16_t*) data;
    int16_t *path_s16 = NULL;
    int8_t *data_s8 = (int8_t*) data;
    int8_t *path_s8 = NULL;
    vg_lite_uint8_t *pathc = NULL;
    vg_lite_int32_t data_size;
    vg_lite_uint8_t arc_path = 0;
    vg_lite_uint8_t h_v_path = 0;
    vg_lite_uint8_t smooth_path = 0;
    vg_lite_pointer path_path = NULL;
    vg_lite_float_t px = 0.0f, py = 0.0f, cx = 0.0f, cy = 0.0f;
    vg_lite_int32_t rel = 0;

    VG_LITE_CHECK_NULL_POINTER3(cmd, data, path);

    for(i = 0; i < seg_count; i++) {
        if (cmd[i] > VLC_OP_LCWARC_REL)
            return VG_LITE_INVALID_ARGUMENT;
    }

    if (!path->path) {
        path->path_length = vg_lite_get_path_length(cmd, seg_count, path->format);

#if VG_PRE_UPLOAD_PATH_SUPPORT
        if (path->path_length > 64)
            path->uploaded.property = 1;
#else
        path->uploaded.property = 0;
#endif

        if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
            vg_lite_uint32_t bytes;
            vg_lite_buffer_t buffer;
            memset(&buffer, 0, sizeof(vg_lite_buffer_t));

            /* Compute the number of bytes required for path + command buffer prefix/postfix. */
            bytes = (8 + path->path_length + 7 + 8) & ~7;

            /* Allocate GPU memory. */
            buffer.width = bytes;
            buffer.height = 1;
            buffer.stride = 0;
            buffer.format = VG_LITE_A8;

            VG_LITE_RETURN_ERROR(vg_lite_allocate(&buffer));
            memset(buffer.memory, 0, buffer.width);

            /* Initialize command buffer prefix. */
            ((vg_lite_uint32_t*)buffer.memory)[0] = VG_LITE_DATA((path->path_length + 7) / 8);
            ((vg_lite_uint32_t*)buffer.memory)[1] = 0;

            /* Mark path as uploaded. */
            path_path = buffer.memory;
            path->path = &((vg_lite_uint32_t*)buffer.memory)[2];

            path->uploaded.handle = buffer.handle;
            path->uploaded.address = buffer.address;
            path->uploaded.memory = buffer.memory;
            path->uploaded.bytes = bytes;
            path->path_changed = 0;
            path->pdata_internal = 0;
        }
        else {
            path->path = (vg_lite_pointer)vg_lite_os_malloc(path->path_length);
            path->pdata_memory_size = path->path_length;
            if (!path->path)
                return VG_LITE_OUT_OF_RESOURCES;

            path->pdata_internal = 1;
            memset(path->path, 0, path->path_length);
            path->path_changed = 1;
        }
    }
    else {
        vg_lite_uint32_t temp_path_length = vg_lite_get_path_length(cmd, seg_count, path->format);
        path->pdata_memory_size = path->path_length;

        if (path->uploaded.handle) {
            if (path->uploaded.bytes < ((8 + temp_path_length + 7 + 8) & ~7))
            {
                printf("Error! The second path length is longer than the first! \n");
                printf("Path should be freed and reinitialized. \n");
                return VG_LITE_OUT_OF_MEMORY;
            }

            memset(path->uploaded.memory, 0, path->uploaded.bytes);
            path->path_length = temp_path_length;
            ((vg_lite_uint32_t*)path->path)[0] = VG_LITE_DATA((path->path_length + 7) / 8);
            ((vg_lite_uint32_t*)path->path)[1] = 0;

            path_path = path->path;
            path->path = &((vg_lite_uint32_t*)path->path)[2];
        }
        else
        {
            if (temp_path_length > path->pdata_memory_size)
            {
                vg_lite_os_free(path->path);
                path->path = vg_lite_os_malloc(temp_path_length);
                path->pdata_memory_size = temp_path_length;
                path->path_length = temp_path_length;
                path->pdata_internal = 1;
                
                memset(path->path, 0, path->path_length);
            }

            path->path_changed = 1;
        }
    }
    
    path->append_path_flag = 1;
    data_size = get_data_size(path->format);    
    pathf = (vg_lite_float_t *)path->path;
    path_s32 = (vg_lite_int32_t *)path->path;
    path_s16 = (int16_t *)path->path;
    path_s8 = (int8_t *)path->path;
    pathc = (vg_lite_uint8_t *)path->path;
    /* Set bounding box if the first opcode is VLC_OP_MOVE_* */
    if ((cmd[0] & 0xfe) == VLC_OP_MOVE) {
        switch (path->format)
        {
        case VG_LITE_S8:
            cx = (vg_lite_float_t)data_s8[0];
            cy = (vg_lite_float_t)data_s8[1];
            break;
        case VG_LITE_S16:
            cx = (vg_lite_float_t)data_s16[0];
            cy = (vg_lite_float_t)data_s16[1];
            break;
        case VG_LITE_S32:
            cx = (vg_lite_float_t)data_s32[0];
            cy = (vg_lite_float_t)data_s32[1];
            break;
        case VG_LITE_FP32:
            cx = (vg_lite_float_t)dataf[0];
            cy = (vg_lite_float_t)dataf[1];
            break;
        }
        path->bounding_box[0] = path->bounding_box[2] = cx;
        path->bounding_box[1] = path->bounding_box[3] = cy;
    }

    /* Loop to fill path data. */
    for (i = 0; i < seg_count; i++) {
        if (!chip_patch_close_command(cmd[i],
                                      cmd[i + 1],
                                      data_size,
                                      pathc,
                                      &offset)) {
            *(pathc + offset) = cmd[i];
            offset++;
        }

        dataCount = get_data_count(cmd[i]);
        /* compute the bounding_box. */
        if (dataCount >= 0) {
            offset = CDALIGN(offset, data_size);
            if ((cmd[i] > VLC_OP_CLOSE) &&
                (cmd[i] < VLC_OP_HLINE) &&
                ((cmd[i] & 0x01) == 1)) {
                rel = 1;
            }
            else if ((cmd[i] >= VLC_OP_HLINE) && 
                ((cmd[i] & 0x01) == 0)) {
                rel = 1;
            }
            else {
                rel = 0;
            }
            if (cmd[i] >= VLC_OP_HLINE && cmd[i] <= VLC_OP_VLINE_REL) {
                switch (path->format) {
                case VG_LITE_S8:
                    path_s8 = (int8_t*)(pathc + offset);
                    path_s8[0] = *data_s8;
                    data_s8++;
                    if (rel) {
                        cx = px + (vg_lite_float_t)path_s8[0];
                        cy = py + (vg_lite_float_t)path_s8[1];
                    }
                    else {
                        cx = (vg_lite_float_t)path_s8[0];
                        cy = (vg_lite_float_t)path_s8[1];
                    }
                    break;

                case VG_LITE_S16:
                    path_s16 = (int16_t*)(pathc + offset);
                    path_s16[0] = *data_s16;
                    data_s16++;
                    if (rel) {
                        cx = px + (vg_lite_float_t)path_s16[0];
                        cy = py + (vg_lite_float_t)path_s16[1];
                    }
                    else {
                        cx = (vg_lite_float_t)path_s16[0];
                        cy = (vg_lite_float_t)path_s16[1];
                    }
                    break;

                case VG_LITE_S32:
                    path_s32 = (vg_lite_int32_t*)(pathc + offset);
                    path_s32[0] = *data_s32;
                    data_s32++;
                    if (rel) {
                        cx = px + (vg_lite_float_t)path_s32[0];
                        cy = py + (vg_lite_float_t)path_s32[1];
                    }
                    else {
                        cx = (vg_lite_float_t)path_s32[0];
                        cy = (vg_lite_float_t)path_s32[1];
                    }
                    break;

                case VG_LITE_FP32:
                    pathf = (vg_lite_float_t*)(pathc + offset);
                    pathf[0] = *dataf;
                    dataf++;
                    if (rel) {
                        cx = px + (vg_lite_float_t)pathf[0];
                        cy = py + (vg_lite_float_t)pathf[1];
                    }
                    else {
                        cx = (vg_lite_float_t)pathf[0];
                        cy = (vg_lite_float_t)pathf[1];
                    }
                    break;
                }
                h_v_path = 1;
                /* Update path bounds. */
                path->bounding_box[0] = CDMIN(path->bounding_box[0], cx);
                path->bounding_box[2] = CDMAX(path->bounding_box[2], cx);
                path->bounding_box[1] = CDMIN(path->bounding_box[1], cy);
                path->bounding_box[3] = CDMAX(path->bounding_box[3], cy);
            }
            else if (cmd[i] < VLC_OP_SCCWARC) {
                /* Mark smooth path,convert it in next step. */
                if (cmd[i] <= VLC_OP_SCUBIC_REL && cmd[i] >= VLC_OP_SQUAD) {
                    smooth_path = 1;
                }
                for (j = 0; j < dataCount / 2; j++) {
                    switch (path->format) {
                    case VG_LITE_S8:
                        path_s8 = (int8_t *)(pathc + offset);
                        path_s8[j * 2] = *data_s8;
                        data_s8++;
                        path_s8[j * 2 + 1] = *data_s8;
                        data_s8++;

                        if (rel) {
                            cx = px + path_s8[j * 2];
                            cy = py + path_s8[j * 2 + 1];
                        }
                        else {
                            cx = path_s8[j * 2];
                            cy = path_s8[j * 2 + 1];
                        }
                        break;
                    case VG_LITE_S16:
                        path_s16 = (int16_t *)(pathc + offset);
                        path_s16[j * 2] = *data_s16;
                        data_s16++;
                        path_s16[j * 2 + 1] = *data_s16;
                        data_s16++;

                        if (rel) {
                            cx = px + path_s16[j * 2];
                            cy = py + path_s16[j * 2 + 1];
                        }
                        else {
                            cx = path_s16[j * 2];
                            cy = path_s16[j * 2 + 1];
                        }
                        break;
                    case VG_LITE_S32:
                        path_s32 = (vg_lite_int32_t *)(pathc + offset);
                        path_s32[j * 2] = *data_s32;
                        data_s32++;
                        path_s32[j * 2 + 1] = *data_s32;
                        data_s32++;

                        if (rel) {
                            cx = px + path_s32[j * 2];
                            cy = py + path_s32[j * 2 + 1];
                        }
                        else {
                            cx = (vg_lite_float_t)path_s32[j * 2];
                            cy = (vg_lite_float_t)path_s32[j * 2 + 1];
                        }
                        break;
                    case VG_LITE_FP32:
                        pathf = (vg_lite_float_t *)(pathc + offset);
                        pathf[j * 2] = *dataf;
                        dataf++;
                        pathf[j * 2 + 1] = *dataf;
                        dataf++;

                        if (rel) {
                            cx = px + pathf[j * 2];
                            cy = py + pathf[j * 2 + 1];
                        }
                        else {
                            cx = pathf[j * 2];
                            cy = pathf[j * 2 + 1];
                        }
                        break;

                    default:
                        return VG_LITE_INVALID_ARGUMENT;
                    }
                    /* Update move to and line path bounds. */
                    path->bounding_box[0] = CDMIN(path->bounding_box[0], cx);
                    path->bounding_box[2] = CDMAX(path->bounding_box[2], cx);
                    path->bounding_box[1] = CDMIN(path->bounding_box[1], cy);
                    path->bounding_box[3] = CDMAX(path->bounding_box[3], cy);
                }
            }
#if gcFEATURE_VG_ARC_PATH
            else {
                arc_path = 1;
                switch (path->format) {
                case VG_LITE_S8:
                    path_s8 = (int8_t*)(pathc + offset);
                    path_s8[0] = *data_s8;
                    data_s8++;
                    path_s8[1] = *data_s8;
                    data_s8++;
                    path_s8[2] = *data_s8;
                    data_s8++;
                    path_s8[3] = *data_s8;
                    data_s8++;
                    path_s8[4] = *data_s8;
                    data_s8++;

                    if (rel) {
                        cx = px + path_s8[3];
                        cy = py + path_s8[4];
                    }
                    else {
                        cx = path_s8[3];
                        cy = path_s8[4];
                    }
                    /* Update path bounds. */
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],cx + 2 * path_s8[0],cy + 2 * path_s8[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],px + 2 * path_s8[1],py + 2 * path_s8[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],cx - 2 * path_s8[0],cy - 2 * path_s8[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],px - 2 * path_s8[1],py - 2 * path_s8[1]);
                    break;

                case VG_LITE_S16:
                    path_s16 = (int16_t*)(pathc + offset);
                    path_s16[0] = *data_s16;
                    data_s16++;
                    path_s16[1] = *data_s16;
                    data_s16++;
                    path_s16[2] = *data_s16;
                    data_s16++;
                    path_s16[3] = *data_s16;
                    data_s16++;
                    path_s16[4] = *data_s16;
                    data_s16++;

                    if (rel) {
                        cx = px + path_s16[3];
                        cy = py + path_s16[4];
                    }
                    else {
                        cx = path_s16[3];
                        cy = path_s16[4];
                    }
                    /* Update path bounds. */
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],cx + 2 * path_s16[0],cy + 2 * path_s16[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],px + 2 * path_s16[1],py + 2 * path_s16[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],cx - 2 * path_s16[0],cy - 2 * path_s16[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],px - 2 * path_s16[1],py - 2 * path_s16[1]);
                    break;

                case VG_LITE_S32:
                    path_s32 = (vg_lite_int32_t*)(pathc + offset);
                    path_s32[0] = *data_s32;
                    data_s32++;
                    path_s32[1] = *data_s32;
                    data_s32++;
                    path_s32[2] = *data_s32;
                    data_s32++;
                    path_s32[3] = *data_s32;
                    data_s32++;
                    path_s32[4] = *data_s32;
                    data_s32++;

                    if (rel) {
                        cx = px + path_s32[3];
                        cy = py + path_s32[4];
                    }
                    else {
                        cx = (vg_lite_float_t)path_s32[3];
                        cy = (vg_lite_float_t)path_s32[4];
                    }
                    /* Update path bounds. */
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],cx + 2 * path_s32[0],cy + 2 * path_s32[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],px + 2 * path_s32[1],py + 2 * path_s32[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],cx - 2 * path_s32[0],cy - 2 * path_s32[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],px - 2 * path_s32[1],py - 2 * path_s32[1]);
                    break;

                case VG_LITE_FP32:
                    pathf = (vg_lite_float_t*)(pathc + offset);
                    pathf[0] = *dataf;
                    dataf++;
                    pathf[1] = *dataf;
                    dataf++;
                    pathf[2] = *dataf;
                    dataf++;
                    pathf[3] = *dataf;
                    dataf++;
                    pathf[4] = *dataf;
                    dataf++;

                    if (rel) {
                        cx = px + pathf[3];
                        cy = py + pathf[4];
                    }
                    else {
                        cx = pathf[3];
                        cy = pathf[4];
                    }
                    /* Update path bounds. */
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],cx + 2 * pathf[0],cy + 2 * pathf[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],px + 2 * pathf[1],py + 2 * pathf[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],cx - 2 * pathf[0],cy - 2 * pathf[1]);
                    compute_pathbounds(&path->bounding_box[0], &path->bounding_box[1], &path->bounding_box[2], &path->bounding_box[3],px - 2 * pathf[1],py - 2 * pathf[1]);
                    break;
                }

            }
#endif
            px = cx;
            py = cy;

            offset += dataCount * data_size;
        }
    }
    if (cmd[seg_count - 1] == VLC_OP_END
#if gcFEATURE_VG_ARC_PATH
        || (cmd[seg_count - 1] == VLC_OP_CLOSE  && (arc_path | h_v_path | smooth_path))
#endif
        ) {
        path->path_length = offset;
    }
    else {
        path->path_length = offset + data_size;
        path->add_end = 1;
        ((vg_lite_uint8_t*)(path->path))[offset] = 0;
    }

    if (seg_count >= 3 &&
        cmd[seg_count - 3] == VLC_OP_MOVE &&
        cmd[seg_count - 1] == VLC_OP_END) {
        path->add_end = 1;
    }

#if gcFEATURE_VG_ARC_PATH
    if (arc_path | h_v_path | smooth_path) {
        error = vg_lite_init_arc_path(path,
                    path->format,
                    path->quality,
                    path->path_length,
                    path->path,
                    path->bounding_box[0], path->bounding_box[1],
                    path->bounding_box[2], path->bounding_box[3]);
    }
#endif

    if (VLM_PATH_GET_UPLOAD_BIT(*path)) {
        if (path_path == NULL)
            return VG_LITE_INVALID_ARGUMENT;

        path->path = path_path;

        vg_lite_uint32_t bytes = (8 + path->path_length + 7 + 8) & ~7;

        /* Initialize command buffer postfix. */
        ((vg_lite_uint32_t*)path_path)[(bytes >> 2) - 2] = VG_LITE_RETURN();
        ((vg_lite_uint32_t*)path_path)[(bytes >> 2) - 1] = 0;
    }

    s_context.path_lastX = cx;
    s_context.path_lastY = cy;
    return error;
}

#if (CHIPID==0x355 || CHIPID==0x255) 

#define UPDATE_BOUNDING_BOX(bbx, point)                                 \
    do {                                                                \
        if ((point).x < (bbx).x) {                                      \
            (bbx).width += (bbx).x - (point).x;                         \
            (bbx).x = (point).x;                                        \
        }                                                               \
        if ((point).y < (bbx).y) {                                      \
            (bbx).height += (bbx).y - (point).y;                        \
            (bbx).y = (point).y;                                        \
        }                                                               \
        if ((point).x > (bbx).x + (bbx).width)                          \
            (bbx).width = (point).x - (bbx).x;                          \
        if ((point).y > (bbx).y + (bbx).height)                         \
            (bbx).height = (point).y - (bbx).y;                         \
    } while(0)

static vg_lite_error_t transform_bounding_box(vg_lite_rectangle_t *in_bbx,
                                                     vg_lite_matrix_t *matrix,
                                                     vg_lite_rectangle_t *clip,
                                                     vg_lite_rectangle_t *out_bbx,
                                                     vg_lite_point_t *origin)
{
    vg_lite_point_t temp;

    memset(out_bbx, 0, sizeof(vg_lite_rectangle_t));

    /* Transform image point (0, 0). */
    if (!transform(&temp, 0.0f, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    out_bbx->x = temp.x;
    out_bbx->y = temp.y;

    /* Provide position of the new origin to the caller if requested. */
    if (origin != NULL) {
        origin->x = temp.x;
        origin->y = temp.y;
    }

    /* Transform image point (0, height). */
    if (!transform(&temp, 0.0f, (vg_lite_float_t)in_bbx->height, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    UPDATE_BOUNDING_BOX(*out_bbx, temp);

    /* Transform image point (width, height). */
    if (!transform(&temp, (vg_lite_float_t)in_bbx->width, (vg_lite_float_t)in_bbx->height, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    UPDATE_BOUNDING_BOX(*out_bbx, temp);

    /* Transform image point (width, 0). */
    if (!transform(&temp, (vg_lite_float_t)in_bbx->width, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    UPDATE_BOUNDING_BOX(*out_bbx, temp);

    /* Clip is required */
    if (clip) {
        out_bbx->x = MAX(out_bbx->x, clip->x);
        out_bbx->y = MAX(out_bbx->y, clip->y);
        out_bbx->width = MIN((out_bbx->x + out_bbx->width), (clip->x + clip->width)) - out_bbx->x;
        out_bbx->height = MIN((out_bbx->y + out_bbx->height), (clip->y + clip->height)) - out_bbx->y;
    }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t compute_interpolation_steps(vg_lite_int32_t s_width,
                                            vg_lite_int32_t s_height,
                                            vg_lite_matrix_t *matrix,
                                            vg_lite_float_t *xs,
                                            vg_lite_float_t *ys,
                                            vg_lite_float_t *cs)
{
    vg_lite_matrix_t    im;
    vg_lite_rectangle_t src_bbx, bounding_box, clip;
    vg_lite_error_t     error = VG_LITE_SUCCESS;
    vg_lite_float_t               dx = 0.0f, dy = 0.0f;

    /* Get bounding box. */
    memset(&src_bbx, 0, sizeof(vg_lite_rectangle_t));
    memset(&clip, 0, sizeof(vg_lite_rectangle_t));
    src_bbx.width       = (vg_lite_int32_t)s_width;
    src_bbx.height      = (vg_lite_int32_t)s_height;

    if (s_context.scissor_set) {
        clip.x = s_context.scissor[0];
        clip.y = s_context.scissor[1];
        clip.width  = s_context.scissor[2];
        clip.height = s_context.scissor[3];
    } else {
        clip.x = clip.y = 0;
        clip.width  = s_context.rtbuffer->width;
        clip.height = s_context.rtbuffer->height;
    }
    VG_LITE_RETURN_ERROR(transform_bounding_box(&src_bbx, matrix, &clip, &bounding_box, NULL));
    /* Compute inverse matrix. */
    if (!inverse(&im, matrix))
        return VG_LITE_SUCCESS;
    /* Compute interpolation steps. */
    /* X step */
    xs[0] = im.m[0][0] / s_width;
    xs[1] = im.m[1][0] / s_height;
    xs[2] = im.m[2][0];
    /* Y step */
    ys[0] = im.m[0][1] / s_width;
    ys[1] = im.m[1][1] / s_height;
    ys[2] = im.m[2][1];
    /* C step 2 */
    cs[2] = 0.5f * (im.m[2][0] + im.m[2][1]) + im.m[2][2];

    chip_adjust_interpolation_rounding(matrix, &bounding_box, &im, xs, ys, &dx, &dy);

    /* C step 0, 1*/
    cs[0] = (0.5f * (im.m[0][0] + im.m[0][1]) + im.m[0][2] + dx) / s_width;
    cs[1] = (0.5f * (im.m[1][0] + im.m[1][1]) + im.m[1][2] + dy) / s_height;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t set_interpolation_steps(vg_lite_int32_t s_width,
                                               vg_lite_int32_t s_height,
                                               vg_lite_matrix_t *matrix,
                                               vg_lite_uint8_t push_states,
                                               vg_lite_float_t **steps)
{
    vg_lite_error_t     error = VG_LITE_SUCCESS;
    vg_lite_float_t     xs[3], ys[3], cs[3];

    VG_LITE_RETURN_ERROR(compute_interpolation_steps(s_width, s_height, matrix, xs, ys, cs));

    if (push_states) {
        /* Set command buffer */
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A18, (vg_lite_void *)&cs[0]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A19, (vg_lite_void *)&cs[1]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1A, (vg_lite_void *)&cs[2]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1C, (vg_lite_void *)&xs[0]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1D, (vg_lite_void *)&xs[1]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1E, (vg_lite_void *)&xs[2]));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1F, 0x00000001));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A20, (vg_lite_void *)&ys[0]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A21, (vg_lite_void *)&ys[1]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A22, (vg_lite_void *)&ys[2]));
    } else {
        /* Save the interpolation steps for later use */
        VG_LITE_CHECK_NULL_POINTER(steps);
        steps[0][0] = xs[0];
        steps[0][1] = xs[1];
        steps[0][2] = xs[2];
        steps[1][0] = ys[0];
        steps[1][1] = ys[1];
        steps[1][2] = ys[2];
        steps[2][0] = cs[0];
        steps[2][1] = cs[1];
        steps[2][2] = cs[2];
    }

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t set_interpolation_steps_draw_paint(vg_lite_int32_t s_width,
                                                          vg_lite_int32_t s_height,
                                                          vg_lite_matrix_t* matrix)
{
    vg_lite_error_t     error = VG_LITE_SUCCESS;
    vg_lite_float_t     xs[3], ys[3], cs[3];

    VG_LITE_RETURN_ERROR(compute_interpolation_steps(s_width, s_height, matrix, xs, ys, cs));

    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A04, (vg_lite_pointer)&cs[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A05, (vg_lite_pointer)&cs[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A06, (vg_lite_pointer)&xs[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A07, (vg_lite_pointer)&xs[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A08, (vg_lite_pointer)&ys[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A09, (vg_lite_pointer)&ys[1]));
    /* Set command buffer */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A18, (vg_lite_pointer)&cs[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A19, (vg_lite_pointer)&cs[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1A, (vg_lite_pointer)&cs[2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1C, (vg_lite_pointer)&xs[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1D, (vg_lite_pointer)&xs[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1E, (vg_lite_pointer)&xs[2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1F, 0x00000001));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A20, (vg_lite_pointer)&ys[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A21, (vg_lite_pointer)&ys[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A22, (vg_lite_pointer)&ys[2]));

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t push_state_tess_path_w_h(vg_lite_int32_t tem_width, vg_lite_int32_t tem_height,
    vg_lite_int32_t width, vg_lite_int32_t height)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t wh;

    if (tem_height < height && tem_width < width)
        wh = (vg_lite_uint32_t)(tem_width | (tem_height << 16));
    else if (tem_width < width)
        wh = (vg_lite_uint32_t)(tem_width | (height << 16));
    else if (tem_height < height)
        wh = (vg_lite_uint32_t)(width | (tem_height << 16));
    else
        wh = (vg_lite_uint32_t)(width | (height << 16));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3A, wh));
    return error;
}

/* GC355/GC255 vg_lite_draw API implementation
 */
vg_lite_error_t vg_lite_draw(vg_lite_buffer_t *target,
                             vg_lite_path_t *path,
                             vg_lite_fill_t fill_rule,
                             vg_lite_matrix_t * matrix,
                             vg_lite_blend_t blend,
                             vg_lite_color_t color)
{
    vg_lite_uint32_t blend_mode;
    vg_lite_uint32_t format, quality, tiling, fill;
    vg_lite_uint32_t tessellation_size;
    vg_lite_error_t error;
    vg_lite_int32_t dst_align_width;
    vg_lite_uint32_t mul, div, align;
    vg_lite_point_t point_min = {0}, point_max = {0}, temp = {0};
    vg_lite_int32_t x, y, width, height;
    vg_lite_uint8_t ts_is_fullscreen = 0;
    vg_lite_uint32_t in_premult = 0;
    vg_lite_uint32_t premul_flag = 0;

    VG_LITE_TRACE_API("vg_lite_draw %p %p %d %p %d 0x%08X\n", target, path, fill_rule, matrix, blend, color);
    VG_LITE_TRACE_API("    path_type %d, path_length %d, stroke_size %d\n", path->path_type, path->path_length, path->stroke_size);

#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_CHECK_NULL_POINTER2(path, path->path);
    VG_LITE_RETURN_ERROR(feature_check_8x_csaa_support(path->quality));
    VG_LITE_RETURN_ERROR(chip_check_target_format(target->format));
#endif /* gcFEATURE_VG_ERROR_CHECK */

    if (!path->path_length) {
        return VG_LITE_SUCCESS;
    }

    if (!matrix) {
        matrix = &identity_mtx;
    }

#if gcFEATURE_VG_GAMMA
    set_gamma_dest_only(target, VGL_FALSE);
#endif

    /*blend input into context*/
    s_context.blend_mode = blend;

    /* Adjust premultiply setting according to openvg condition */
    target->apply_premult = 0;
    premul_flag = (s_context.blend_mode >= OPENVG_BLEND_SRC_OVER && s_context.blend_mode <= OPENVG_BLEND_ADDITIVE);
    if (target->premultiplied == 0 && premul_flag == 0) {
        in_premult = 0x10000000;
        target->apply_premult = 1;
    }
    else if ((target->premultiplied == 1) ||
             (target->premultiplied == 0 && premul_flag == 1)) {
        in_premult = 0x00000000;
    }
    if (blend == VG_LITE_BLEND_NORMAL_LVGL) {
        in_premult = 0x00000000;
    }

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        if (error == VG_LITE_NO_CONTEXT) {
            /* If scissoring is enabled and no valid scissoring rectangles
               are present, no drawing occurs */
            return VG_LITE_SUCCESS;
        }
        else {
            return error;
        }
    }

    width = s_context.tessbuf.tess_w_h & 0xFFFF;
    height = s_context.tessbuf.tess_w_h >> 16;
    get_format_bytes(target->format, &mul, &div, &align);
    dst_align_width = target->stride * div / mul;
    if (width == 0 || height == 0)
        return VG_LITE_NO_CONTEXT;
    if ((dst_align_width <= width) && (target->height <= height) && !s_context.scissor_set)
    {
        ts_is_fullscreen = 1;
        point_min.x = 0;
        point_min.y = 0;
        point_max.x = dst_align_width;
        point_max.y = target->height;
    }

    if (ts_is_fullscreen == 0){
        if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH){
            vg_lite_float_t add_width = path->stroke->line_width;
            if (path->stroke->join_style == VG_LITE_JOIN_MITER)
                add_width += path->stroke->miter_limit;
            add_width = 1.5f * add_width;
            path->bounding_box[0] -= add_width;
            path->bounding_box[1] -= add_width;
            path->bounding_box[2] += add_width;
            path->bounding_box[3] += add_width;
        }
        
        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[1], matrix);
        point_min = point_max = temp;
    
        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[1], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;
    
        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;
    
        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        if (point_min.x < 0) point_min.x = 0;
        if (point_min.y < 0) point_min.y = 0;
        if (point_max.x > dst_align_width) point_max.x = dst_align_width;
        if (point_max.y > target->height) point_max.y = target->height;

        if (s_context.scissor_set) {
            point_min.x = MAX(point_min.x, s_context.scissor[0]);
            point_min.y = MAX(point_min.y, s_context.scissor[1]);
            point_max.x = MIN(point_max.x, s_context.scissor[0] + s_context.scissor[2]);
            point_max.y = MIN(point_max.y, s_context.scissor[1] + s_context.scissor[3]);
        }
    }

    if (point_min.x < 0) point_min.x = 0;
    if (point_min.y < 0) point_min.y = 0;
    if (point_max.x > target->width) point_max.x = target->width;
    if (point_max.y > target->height) point_max.x = target->height;

    /* Convert states into hardware values. */
    blend_mode = convert_blend(blend);
    format = convert_path_format(path->format);
    quality = convert_path_quality(path->quality);
    tiling = (s_context.capabilities.cap.tiled == 2) ? 0x2000000 : 0;
    fill = (fill_rule == VG_LITE_FILL_EVEN_ODD) ? 0x10 : 0;
    tessellation_size = s_context.tessbuf.L2_size ? s_context.tessbuf.L2_size : s_context.tessbuf.L1_size;

    /* Setup the command buffer. */
    /* Program color register. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, in_premult | s_context.capabilities.cap.tiled | blend_mode | s_context.enable_mask | s_context.scissor_enable | s_context.color_transform | s_context.matrix_enable));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color));
    /* Program tessellation control: for TS module. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | fill));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3B, 0x3F800000));      /* Path tessellation SCALE. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3C, 0x00000000));      /* Path tessellation BIAS.  */
    /* Program matrix. */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A40, (vg_lite_void *) &matrix->m[0][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A41, (vg_lite_void *) &matrix->m[0][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A42, (vg_lite_void *) &matrix->m[0][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A43, (vg_lite_void *) &matrix->m[1][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A44, (vg_lite_void *) &matrix->m[1][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A45, (vg_lite_void *) &matrix->m[1][2]));

    /* Setup tessellation loop. */
    vg_lite_int32_t tem_width = point_max.x - point_min.x;
    vg_lite_int32_t tem_height = point_max.y - point_min.y;    
    if (path->path_type == VG_LITE_DRAW_FILL_PATH || path->path_type == VG_LITE_DRAW_ZERO || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH)
    {
        for (y = point_min.y; y < point_max.y; y += height) {
            for (x = point_min.x; x < point_max.x; x += width) {
                /* Tessellate path. */
                VG_LITE_RETURN_ERROR(push_stall(&s_context, 15));
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(x | (y << 16)), 0, 1));
                VG_LITE_RETURN_ERROR(push_state_tess_path_w_h(tem_width, tem_height, width, height));

                if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
                } 
                else {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
                }
                tem_width -= width;
            }
            tem_width = point_max.x - point_min.x;
            tem_height -= height;
        }
    }
    /* Setup tessellation loop. */
    tem_width = point_max.x - point_min.x;
    tem_height = point_max.y - point_min.y;
    if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {
        for (y = point_min.y; y < point_max.y; y += height) {
            for (x = point_min.x; x < point_max.x; x += width) {
                /* Tessellate path. */
                VG_LITE_RETURN_ERROR(push_stall(&s_context, 15));
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(x | (y << 16)), 0, 1));
                format = convert_path_format(VG_LITE_FP32);
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0x0));
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, path->stroke_color));
                VG_LITE_RETURN_ERROR(push_state_tess_path_w_h(tem_width, tem_height, width, height));

                if (VLM_PATH_STROKE_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->stroke->uploaded.address, path->stroke->uploaded.bytes));
                } 
                else {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
                }
                tem_width -= width;
            }
            tem_width = point_max.x - point_min.x;
            tem_height -= height;
        }
    }
    /* Finialize command buffer. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));

    return error;
}

/* GC355/GC255 vg_lite_draw_pattern API implementation
 */
vg_lite_error_t vg_lite_draw_pattern(vg_lite_buffer_t *target,
                                     vg_lite_path_t *path,
                                     vg_lite_fill_t fill_rule,
                                     vg_lite_matrix_t *path_matrix,
                                     vg_lite_buffer_t *source,
                                     vg_lite_matrix_t *pattern_matrix,
                                     vg_lite_blend_t blend,
                                     vg_lite_pattern_mode_t pattern_mode,
                                     vg_lite_color_t  pattern_color,
                                     vg_lite_color_t  color,
                                     vg_lite_filter_t filter)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t imageMode;
    vg_lite_uint32_t blend_mode;
    vg_lite_uint32_t filter_mode = 0;
    vg_lite_int32_t dst_align_width;
    vg_lite_uint32_t mul, div, align;
    vg_lite_uint32_t conversion = 0;
    vg_lite_uint32_t tiled_source;
    vg_lite_matrix_t matrix;
    vg_lite_uint32_t pattern_tile = 0;
    vg_lite_uint32_t transparency_mode = 0;
    
    /* The following code is from "draw path" */
    vg_lite_uint32_t format, quality, tiling, fill;
    vg_lite_uint32_t tessellation_size;

    vg_lite_point_t point_min = {0}, point_max = {0}, temp = {0};
    vg_lite_int32_t x, y, width, height;
    vg_lite_uint8_t ts_is_fullscreen = 0;
    vg_lite_uint32_t in_premult = 0;
    vg_lite_uint32_t src_premultiply_enable = 0;
    vg_lite_uint32_t paintType = 0;
    vg_lite_uint32_t premul_flag = 0;
    vg_lite_uint32_t prediv_flag = 0;
#if DUMP_CAPTURE
    vg_lite_float_t ratio = 1;
#endif
#if !gcFEATURE_VG_LVGL_SUPPORT
    vg_lite_uint8_t  lvgl_sw_blend = 0;
#endif

    VG_LITE_TRACE_API("vg_lite_draw_pattern %p %p %d %p %p %p %d %d 0x%08X %d\n",
        target, path, fill_rule, path_matrix, source, pattern_matrix, blend, pattern_mode, pattern_color, filter);

#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_CHECK_NULL_POINTER2(path, path->path);
    VG_LITE_RETURN_ERROR(feature_check_8x_csaa_support(path->quality));
    VG_LITE_RETURN_ERROR(check_draw_pattern_source_format(source->format));
    VG_LITE_RETURN_ERROR(chip_check_target_format(target->format));
#endif /* gcFEATURE_VG_ERROR_CHECK */

#if !gcFEATURE_VG_LVGL_SUPPORT
    if (is_lvgl_blend_mode(blend)) {
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

    if (!path->path_length) {
        return VG_LITE_SUCCESS;
    }

    if (!path_matrix) {
        path_matrix = &identity_mtx;
    }
    if (!pattern_matrix) {
        pattern_matrix = &identity_mtx;
    }

    /* Work on pattern states. */
    matrix = *pattern_matrix;
    if (source->paintType == VG_LITE_PAINT_PATTERN)
    {
        matrix.m[2][0] = 0;
        matrix.m[2][1] = 0;
        matrix.m[2][2] = 1;
        source->image_mode = VG_LITE_NONE_IMAGE_MODE;
    }

#if gcFEATURE_VG_GAMMA
    save_st_gamma_src_dest(source, target);
#endif

    /*blend input into context*/
    s_context.blend_mode = blend;
    in_premult = 0x00000000;

    /* Adjust premultiply setting according to openvg condition */
    src_premultiply_enable = 0x01000100;
    if (s_context.color_transform == 0 && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
    else {
        prediv_flag = 1;
    }
    if ((s_context.blend_mode >= OPENVG_BLEND_SRC_OVER && s_context.blend_mode <= OPENVG_BLEND_ADDITIVE) || source->image_mode == VG_LITE_STENCIL_MODE) {
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
    if ((source->format == VG_LITE_A4 || source->format == VG_LITE_A8) && blend >= VG_LITE_BLEND_SRC_OVER && blend <= VG_LITE_BLEND_SUBTRACT) {
        in_premult = 0x00000000;
    }
    if (blend == VG_LITE_BLEND_NORMAL_LVGL) {
        in_premult = 0x00000000;
    }
    if (source->premultiplied == target->premultiplied && premul_flag == 0) {
        target->apply_premult = 1;
    }
    else {
        target->apply_premult = 0;
    }

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        if (error == VG_LITE_NO_CONTEXT) {
            /* If scissoring is enabled and no valid scissoring rectangles
               are present, no drawing occurs */
            return VG_LITE_SUCCESS;
        }
        else {
            return error;
        }
    }

    transparency_mode = (source->transparency_mode == VG_LITE_IMAGE_TRANSPARENT ? 0x8000:0);
    width = s_context.tessbuf.tess_w_h & 0xFFFF;
    height = s_context.tessbuf.tess_w_h >> 16;
    get_format_bytes(target->format, &mul, &div, &align);
    dst_align_width = target->stride * div / mul;
    if (width == 0 || height == 0)
        return VG_LITE_NO_CONTEXT;
    if ((dst_align_width <= width) && (target->height <= height) && !s_context.scissor_set)
    {
        ts_is_fullscreen = 1;
        point_min.x = 0;
        point_min.y = 0;
        point_max.x = dst_align_width;
        point_max.y = target->height;
    }

    /* If target is L8 and source is in YUV or RGB (not L8 or A8) then we have to convert RGB into L8. */
    if ((target->format == VG_LITE_L8) && ((source->format != VG_LITE_L8) && (source->format != VG_LITE_A8))) {
        conversion = 0x80000000;
    }

    /* Determine image mode (NORMAL or MULTIPLY) depending on the color. */
    imageMode = (source->image_mode == VG_LITE_NONE_IMAGE_MODE) ? 0 : (source->image_mode == VG_LITE_MULTIPLY_IMAGE_MODE) ? 0x00002000 : 0x00001000;
    tiled_source = (source->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0 ;
    
    if (pattern_mode == VG_LITE_PATTERN_COLOR)
    {
        vg_lite_uint8_t a,r,g,b;
        pattern_tile = 0;
        a = pattern_color >> 24;
        r = pattern_color >> 16;
        g = pattern_color >> 8;
        b = pattern_color;
        pattern_color = (a << 24) | (b << 16) | (g << 8) | r;
    }
    else if (pattern_mode == VG_LITE_PATTERN_PAD)
    {
        pattern_tile = 0x1000;
    }
#if gcFEATURE_VG_IM_REPEAT_REFLECT
    else if (pattern_mode == VG_LITE_PATTERN_REPEAT)
    {
        pattern_tile = 0x2000;
    }
    else if (pattern_mode == VG_LITE_PATTERN_REFLECT)
    {
        pattern_tile = 0x3000;
    }
#endif
    else
    {
        return VG_LITE_INVALID_ARGUMENT;
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

    if (source->paintType == VG_LITE_PAINT_PATTERN)
    {
        VG_LITE_RETURN_ERROR(set_interpolation_steps_draw_paint(source->width, source->height, &matrix));
        /* enable pre-multiplied in image unit */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A24, convert_source_format(source->format) |
            filter_mode | pattern_tile | conversion | src_premultiply_enable));

        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A26, pattern_color));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A28, source->address));

        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2A, source->stride | tiled_source));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2C, 0));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2E, source->width | (source->height << 16)));
    }
    else
    {
        VG_LITE_RETURN_ERROR(set_interpolation_steps(source->width, source->height, &matrix, 1, NULL));
        /* enable pre-multiplied in image unit */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A25, convert_source_format(source->format) |
            filter_mode | pattern_tile | conversion | src_premultiply_enable));

        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A27, pattern_color));

#if !gcFEATURE_VG_LVGL_SUPPORT
        if (lvgl_sw_blend) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source->lvgl_buffer->address));
        }
        else
#endif
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source->address));

        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, source->stride | tiled_source));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2D, 0));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2F, source->width | (source->height << 16)));
    }

    /* Work on path states. */
    matrix = *path_matrix;

    if (ts_is_fullscreen == 0){
        if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH){
            vg_lite_float_t add_width = path->stroke->line_width;
            if (path->stroke->join_style == VG_LITE_JOIN_MITER)
                add_width += path->stroke->miter_limit;
            add_width = 1.5f * add_width;
            path->bounding_box[0] -= add_width;
            path->bounding_box[1] -= add_width;
            path->bounding_box[2] += add_width;
            path->bounding_box[3] += add_width;
        }
        
        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[1], &matrix);
        point_min = point_max = temp;
    
        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[1], &matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;
    
        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[3], &matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;
    
        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[3], &matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;
    
        point_min.x = MAX(point_min.x, 0);
        point_min.y = MAX(point_min.y, 0);
        point_max.x = MIN(point_max.x, dst_align_width);
        point_max.y = MIN(point_max.y, target->height);

        if (s_context.scissor_set) {
            point_min.x = MAX(point_min.x, s_context.scissor[0]);
            point_min.y = MAX(point_min.y, s_context.scissor[1]);
            point_max.x = MIN(point_max.x, s_context.scissor[0] + s_context.scissor[2]);
            point_max.y = MIN(point_max.y, s_context.scissor[1] + s_context.scissor[3]);
        }
    }

    if (point_min.x < 0) point_min.x = 0;
    if (point_min.y < 0) point_min.y = 0;
    if (point_max.x > target->width) point_max.x = target->width;
    if (point_max.y > target->height) point_max.x = target->height;

    /* Convert states into hardware values. */
    blend_mode = convert_blend(blend);
    format = convert_path_format(path->format);
    quality = convert_path_quality(path->quality);
    tiling = (s_context.capabilities.cap.tiled == 2) ? 0x2000000 : 0;
    fill = (fill_rule == VG_LITE_FILL_EVEN_ODD) ? 0x10 : 0;
    tessellation_size = s_context.tessbuf.L2_size ? s_context.tessbuf.L2_size : s_context.tessbuf.L1_size;

    /* Setup the command buffer. */
    /* Program color register. */
    if (source->paintType == VG_LITE_PAINT_PATTERN) {
        paintType = 1 << 24 | 1 << 25;
    }
    /* enable pre-multiplied from VG to VGPE */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x2 | in_premult | paintType  | s_context.capabilities.cap.tiled | imageMode | blend_mode | transparency_mode | s_context.enable_mask | s_context.scissor_enable | s_context.color_transform | s_context.matrix_enable));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000400 | format | quality | tiling | fill));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3B, 0x3F800000));      /* Path tessellation SCALE. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3C, 0x00000000));      /* Path tessellation BIAS.  */
    /* Program matrix. */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A40, (vg_lite_void *) &matrix.m[0][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A41, (vg_lite_void *) &matrix.m[0][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A42, (vg_lite_void *) &matrix.m[0][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A43, (vg_lite_void *) &matrix.m[1][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A44, (vg_lite_void *) &matrix.m[1][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A45, (vg_lite_void *) &matrix.m[1][2]));

    /* Setup tessellation loop. */
    vg_lite_int32_t tem_width = point_max.x - point_min.x;
    vg_lite_int32_t tem_height = point_max.y - point_min.y;
    if (path->path_type == VG_LITE_DRAW_FILL_PATH || path->path_type == VG_LITE_DRAW_ZERO || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH)
    {
        for (y = point_min.y; y < point_max.y; y += height) {
            for (x = point_min.x; x < point_max.x; x += width) {
                /* Tessellate path. */
                VG_LITE_RETURN_ERROR(push_stall(&s_context, 15));
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(x | (y << 16)), 0, 1));
                VG_LITE_RETURN_ERROR(push_state_tess_path_w_h(tem_width, tem_height, width, height));

                if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
                }
                else {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
                }
                tem_width -= width;
            }
            tem_width = point_max.x - point_min.x;
            tem_height -= height;
        }
    }
    /* Setup tessellation loop. */
    tem_width = point_max.x - point_min.x;
    tem_height = point_max.y - point_min.y;
    if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {
        for (y = point_min.y; y < point_max.y; y += height) {
            for (x = point_min.x; x < point_max.x; x += width) {
                /* Tessellate path. */
                VG_LITE_RETURN_ERROR(push_stall(&s_context, 15));
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(x | (y << 16)), 0, 1));
                format = convert_path_format(VG_LITE_FP32);
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0x0));
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, path->stroke_color));
                VG_LITE_RETURN_ERROR(push_state_tess_path_w_h(tem_width, tem_height, width, height));

                if (VLM_PATH_STROKE_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->stroke->uploaded.address, path->stroke->uploaded.bytes));
                } 
                else {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
                }
                tem_width -= width;
            }
            tem_width = point_max.x - point_min.x;
            tem_height -= height;
        }
    }

    /* Finialize command buffer. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));

#if DUMP_CAPTURE
    if (source->compress_mode)
        ratio = _calc_decnano_compress_ratio(source->format, source->compress_mode);
    vglitemDUMP_BUFFER("image", (size_t)source->address, source->memory, 0, (source->stride)* (source->height)*ratio);
#endif

    return error;
}

/* GC355/GC255 vg_lite_draw_linear_grad API implementation
 */
vg_lite_error_t vg_lite_draw_linear_grad(vg_lite_buffer_t * target,
                                     vg_lite_path_t * path,
                                     vg_lite_fill_t fill_rule,
                                     vg_lite_matrix_t * path_matrix,
                                     vg_lite_linear_gradient_ext_t *grad,
                                     vg_lite_color_t paint_color,
                                     vg_lite_blend_t blend,
                                     vg_lite_filter_t filter)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t image_mode;
    vg_lite_uint32_t blend_mode;
    vg_lite_uint32_t filter_mode = 0;
    vg_lite_uint32_t conversion = 0;
    vg_lite_uint32_t tiled_source;
    vg_lite_int32_t dst_align_width;
    vg_lite_uint32_t mul, div, align;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_buffer_t * source = &grad->image;
    vg_lite_matrix_t * matrix = &grad->matrix;
    vg_lite_uint32_t linear_tile = 0;
    vg_lite_uint32_t transparency_mode = 0;
    vg_lite_uint32_t in_premult = 0;
    vg_lite_uint32_t src_premultiply_enable = 0;
    vg_lite_uint32_t premul_flag = 0;
    vg_lite_uint32_t prediv_flag = 0;
    vg_lite_void *data;

    /* The following code is from "draw path" */
    vg_lite_uint32_t format, quality, tiling, fill;
    vg_lite_uint32_t tessellation_size;

    vg_lite_kernel_allocate_t memory;
    vg_lite_kernel_free_t free_memory;
    vg_lite_uint32_t return_offset = 0;

    vg_lite_point_t point_min = {0}, point_max = {0}, temp = {0};
    vg_lite_int32_t x, y, width, height;
    vg_lite_uint8_t ts_is_fullscreen = 0;

    vg_lite_float_t dx, dy, dxdx_dydy;
    vg_lite_float_t lg_step_x_lin, lg_step_y_lin, lg_constant_lin;

    VG_LITE_TRACE_API("vg_lite_draw_linear_grad %p %p %d %p %p 0x%08X %d %d\n",
        target, path, fill_rule, path_matrix, grad, paint_color, blend, filter);

#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_CHECK_NULL_POINTER2(path, path->path);
    VG_LITE_RETURN_ERROR(feature_check_lvgl_blend_mode(blend));
    VG_LITE_RETURN_ERROR(feature_check_8x_csaa_support(path->quality));
    VG_LITE_RETURN_ERROR(check_draw_pattern_source_format(source->format));
    VG_LITE_RETURN_ERROR(chip_check_target_format(target->format));
#endif /* gcFEATURE_VG_ERROR_CHECK */

    if (!path_matrix) {
        path_matrix = &identity_mtx;
    }

#if gcFEATURE_VG_GAMMA
    set_gamma_dest_only(target, VGL_TRUE);
#endif

    /*blend input into context*/
    s_context.blend_mode = blend;

    src_premultiply_enable = 0x01000100;
    if (s_context.color_transform == 0 && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
    else {
        prediv_flag = 1;
    }
    if ((s_context.blend_mode >= OPENVG_BLEND_SRC_OVER && s_context.blend_mode <= OPENVG_BLEND_ADDITIVE) || source->image_mode == VG_LITE_STENCIL_MODE) {
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

    if ((source->format == VG_LITE_A4 || source->format == VG_LITE_A8) && blend >= VG_LITE_BLEND_SRC_OVER && blend <= VG_LITE_BLEND_SUBTRACT) {
        chip_adjust_src_premultiply_enable(&src_premultiply_enable);
        in_premult = 0x00000000;
    }

    if (blend == VG_LITE_BLEND_NORMAL_LVGL) {
        in_premult = 0x00000000;
    }
    
    if (source->premultiplied == target->premultiplied && premul_flag == 0) {
        target->apply_premult = 1;
    }
    else {
        target->apply_premult = 0;
    }

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        if (error == VG_LITE_NO_CONTEXT) {
            /* If scissoring is enabled and no valid scissoring rectangles
               are present, no drawing occurs */
            return VG_LITE_SUCCESS;
        }
        else {
            return error;
        }
    }

    transparency_mode = (source->transparency_mode == VG_LITE_IMAGE_TRANSPARENT ? 0x8000:0);
    width = s_context.tessbuf.tess_w_h & 0xFFFF;
    height = s_context.tessbuf.tess_w_h >> 16;
    get_format_bytes(target->format, &mul, &div, &align);
    dst_align_width = target->stride * div / mul;
    if (width == 0 || height == 0)
        return VG_LITE_NO_CONTEXT;
    if ((dst_align_width <= width) && (target->height <= height) && !s_context.scissor_set)
    {
        ts_is_fullscreen = 1;
        point_min.x = 0;
        point_min.y = 0;
        point_max.x = dst_align_width;
        point_max.y = target->height;
    }

    /* If target is L8 and source is in YUV or RGB (not L8 or A8) then we have to convert RGB into L8. */
    if ((target->format == VG_LITE_L8) && ((source->format != VG_LITE_L8) && (source->format != VG_LITE_A8))) {
        conversion = 0x80000000;
    }

    /* Determine image mode (NORMAL or MULTIPLY) depending on the color. */
    image_mode = (source->image_mode == VG_LITE_NONE_IMAGE_MODE) ? 0 : (source->image_mode == VG_LITE_MULTIPLY_IMAGE_MODE) ? 0x00002000 : 0x00001000;
    tiled_source = (source->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0 ;

    switch (grad->spread_mode) {
        case VG_LITE_GRADIENT_SPREAD_FILL:
            linear_tile = 0x0;
            break;

        case VG_LITE_GRADIENT_SPREAD_PAD:
            linear_tile = 0x1000;
            break;

        case VG_LITE_GRADIENT_SPREAD_REPEAT:
            linear_tile = 0x2000;
            break;

        case VG_LITE_GRADIENT_SPREAD_REFLECT:
            linear_tile = 0x3000;
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

    if (grad->spread_mode == VG_LITE_GRADIENT_SPREAD_FILL)
    {
        vg_lite_uint8_t a,r,g,b;
        a = paint_color >> 24;
        r = paint_color >> 16;
        g = paint_color >> 8;
        b = paint_color;
        paint_color = (a << 24) | (b << 16) | (g << 8) | r;
    }

    /* compute radial gradient paremeters */

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix))
        return VG_LITE_SUCCESS;

    dx = grad->linear_grad.X1 - grad->linear_grad.X0;
    dy = grad->linear_grad.Y1 - grad->linear_grad.Y0;
    dxdx_dydy = dx * dx + dy * dy;

    /*
    **      dx (T(x) - x0) + dy (T(y) - y0)
    **  g = -------------------------------
    **                dx^2 + dy^2
    **
    **  where
    **
    **      dx := x1 - x0
    **      dy := y1 - y1
    **      T(x) := (x + 0.5) m00 + (y + 0.5) m01 + m02
    **            = x m00 + y m01 + 0.5 (m00 + m01) + m02
    **      T(y) := (x + 0.5) m10 + (y + 0.5) m11 + m12
    **            = x m10 + y m11 + 0.5 (m10 + m11) + m12.
    **
    **  We can factor the top line into:
    **
    **      = dx (x m00 + y m01 + 0.5 (m00 + m01) + m02 - x0)
    **      + dy (x m10 + y m11 + 0.5 (m10 + m11) + m12 - y0)
    **
    **      = x (dx m00 + dy m10)
    **      + y (dx m01 + dy m11)
    **      + dx (0.5 (m00 + m01) + m02 - x0)
    **      + dy (0.5 (m10 + m11) + m12 - y0).
    */

    lg_step_x_lin
        = (dx * MAT(&inverse_matrix, 0, 0) + dy * MAT(&inverse_matrix, 1, 0))
        / dxdx_dydy;

    lg_step_y_lin
        = (dx * MAT(&inverse_matrix, 0, 1) + dy * MAT(&inverse_matrix, 1, 1))
        / dxdx_dydy;

    lg_constant_lin =
        (
            (
                0.5f * ( MAT(&inverse_matrix, 0, 0) + MAT(&inverse_matrix, 0, 1) )
                + MAT(&inverse_matrix, 0, 2) - grad->linear_grad.X0
            ) * dx

            +

            (
                0.5f * ( MAT(&inverse_matrix, 1, 0) + MAT(&inverse_matrix, 1, 1) )
                + MAT(&inverse_matrix, 1, 2) - grad->linear_grad.Y0
            ) * dy
        )
        / dxdx_dydy;

    /* Setup the command buffer. */

    /* linear gradient parameters*/
    data = &lg_constant_lin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A04,*(vg_lite_uint32_t*) data));
    data = &lg_step_x_lin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A06,*(vg_lite_uint32_t*) data));
    data = &lg_step_y_lin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A08,*(vg_lite_uint32_t*) data));

    VG_LITE_RETURN_ERROR(set_interpolation_steps(source->width, source->height, matrix, 1, NULL));

    /* enable pre-multiplied in image unit */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A24, convert_source_format(source->format) |
                                                            filter_mode | linear_tile | conversion | src_premultiply_enable));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A26, paint_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A28, source->address));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2A, tiled_source));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2C, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2E, source->width));

    /* Work on path states. */
    matrix = path_matrix;

    if (ts_is_fullscreen == 0){
        if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH){
            vg_lite_float_t add_width = path->stroke->line_width;
            if (path->stroke->join_style == VG_LITE_JOIN_MITER)
                add_width += path->stroke->miter_limit;
            add_width = 1.5f * add_width;
            path->bounding_box[0] -= add_width;
            path->bounding_box[1] -= add_width;
            path->bounding_box[2] += add_width;
            path->bounding_box[3] += add_width;
        }
        
        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[1], matrix);
        point_min = point_max = temp;

        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[1], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        point_min.x = MAX(point_min.x, 0);
        point_min.y = MAX(point_min.y, 0);
        point_max.x = MIN(point_max.x, dst_align_width);
        point_max.y = MIN(point_max.y, target->height);

        if (s_context.scissor_set) {
            point_min.x = MAX(point_min.x, s_context.scissor[0]);
            point_min.y = MAX(point_min.y, s_context.scissor[1]);
            point_max.x = MIN(point_max.x, s_context.scissor[0] + s_context.scissor[2]);
            point_max.y = MIN(point_max.y, s_context.scissor[1] + s_context.scissor[3]);
        }
    }

    if (point_min.x < 0) point_min.x = 0;
    if (point_min.y < 0) point_min.y = 0;
    if (point_max.x > target->width) point_max.x = target->width;
    if (point_max.y > target->height) point_max.x = target->height;

    /* Convert states into hardware values. */
    blend_mode = convert_blend(blend);
    format = convert_path_format(path->format);
    quality = convert_path_quality(path->quality);
    tiling = (s_context.capabilities.cap.tiled == 2) ? 0x2000000 : 0;
    fill = (fill_rule == VG_LITE_FILL_EVEN_ODD) ? 0x10 : 0;
    tessellation_size = s_context.tessbuf.L2_size ? s_context.tessbuf.L2_size : s_context.tessbuf.L1_size;

    /* Setup the command buffer. */
    /* Program color register. */

    /* enable pre-multiplied from VG to VGPE */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x01000002 | s_context.capabilities.cap.tiled | in_premult | image_mode | blend_mode | transparency_mode | s_context.enable_mask | s_context.scissor_enable | s_context.color_transform | s_context.matrix_enable));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000400 | format | quality | tiling | fill));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3B, 0x3F800000));      /* Path tessellation SCALE. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3C, 0x00000000));      /* Path tessellation BIAS.  */
    /* Program matrix. */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A40, (vg_lite_void *) &matrix->m[0][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A41, (vg_lite_void *) &matrix->m[0][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A42, (vg_lite_void *) &matrix->m[0][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A43, (vg_lite_void *) &matrix->m[1][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A44, (vg_lite_void *) &matrix->m[1][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A45, (vg_lite_void *) &matrix->m[1][2]));

    if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
    {
        if (path->path_changed != 0) {
            if (path->uploaded.handle != NULL) {
                free_memory.memory_handle = path->uploaded.handle;
                vg_lite_kernel(VG_LITE_FREE, &free_memory);
                path->uploaded.address = 0;
                path->uploaded.memory = NULL;
                path->uploaded.handle = NULL;
            }
            /* Allocate memory for the path data. */
            memory.bytes = 16 + VG_LITE_ALIGN(path->path_length, 8);
            return_offset = (8 + VG_LITE_ALIGN(path->path_length, 8)) / 4;
            memory.contiguous = 1;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &memory));
            ((uint64_t *) memory.memory)[(path->path_length + 7) / 8] = 0;
            ((vg_lite_uint32_t *) memory.memory)[0] = VG_LITE_DATA((path->path_length + 7) / 8);
            ((vg_lite_uint32_t *) memory.memory)[1] = 0;
            memcpy((vg_lite_uint8_t *) memory.memory + 8, path->path, path->path_length);
            ((vg_lite_uint32_t *) memory.memory)[return_offset] = VG_LITE_RETURN();
            ((vg_lite_uint32_t *) memory.memory)[return_offset + 1] = 0;

            path->uploaded.handle = memory.memory_handle;
            path->uploaded.memory = memory.memory;
            path->uploaded.address = memory.memory_gpu;
            path->uploaded.bytes  = memory.bytes;
            path->path_changed = 0;
        }
    }

    /* Setup tessellation loop. */
    vg_lite_int32_t tem_width = point_max.x - point_min.x;
    vg_lite_int32_t tem_height = point_max.y - point_min.y;
    if (path->path_type == VG_LITE_DRAW_FILL_PATH || path->path_type == VG_LITE_DRAW_ZERO) {
        for (y = point_min.y; y < point_max.y; y += height) {
            for (x = point_min.x; x < point_max.x; x += width) {
                /* Tessellate path. */
                VG_LITE_RETURN_ERROR(push_stall(&s_context, 15));
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(x | (y << 16)), 0, 1));
                VG_LITE_RETURN_ERROR(push_state_tess_path_w_h(tem_width, tem_height, width, height));

                if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
                } 
                else {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
                }
                tem_width -= width;
            }
            tem_width = point_max.x - point_min.x;
            tem_height -= height;
        }
    }
    /* Setup tessellation loop. */
    tem_width = point_max.x - point_min.x;
    tem_height = point_max.y - point_min.y;
    if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {
        vg_lite_int32_t stroke_dx = (vg_lite_int32_t)(((path->stroke->line_width + 1) / 2) * matrix->m[0][0]);
        vg_lite_int32_t stroke_dy = (vg_lite_int32_t)(((path->stroke->line_width + 1) / 2) * matrix->m[1][1]);
        vg_lite_int32_t temp_x = point_min.x - stroke_dx > 0 ? point_min.x - stroke_dx : 0;
        vg_lite_int32_t temp_y = point_min.y - stroke_dy > 0 ? point_min.y - stroke_dy : 0;
        for (y = temp_y; y < point_max.y; y += height) {
            for (x = temp_x; x < point_max.x; x += width) {
                /* Tessellate path. */
                VG_LITE_RETURN_ERROR(push_stall(&s_context, 15));
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(x | (y << 16)), 0, 1));
                format = convert_path_format(VG_LITE_FP32);
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0x0));
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, path->stroke_color));
                VG_LITE_RETURN_ERROR(push_state_tess_path_w_h(tem_width, tem_height, width, height));

                if (VLM_PATH_STROKE_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->stroke->uploaded.address, path->stroke->uploaded.bytes));
                } else {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
                }
                tem_width -= width;
            }
            tem_width = point_max.x - point_min.x;
            tem_height -= height;
        }
    }

    /* Finialize command buffer. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));

    return error;
}

/* GC355/GC255 vg_lite_draw_radial_grad API implementation
 */
vg_lite_error_t vg_lite_draw_radial_grad(vg_lite_buffer_t * target,
                                     vg_lite_path_t * path,
                                     vg_lite_fill_t fill_rule,
                                     vg_lite_matrix_t * path_matrix,
                                     vg_lite_radial_gradient_t *grad,
                                     vg_lite_color_t paint_color,
                                     vg_lite_blend_t blend,
                                     vg_lite_filter_t filter)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t imageMode;
    vg_lite_uint32_t blend_mode;
    vg_lite_uint32_t filter_mode = 0;
    vg_lite_uint32_t conversion = 0;
    vg_lite_uint32_t tiled_source;
    vg_lite_int32_t dst_align_width;
    vg_lite_uint32_t mul, div, align;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_buffer_t * source = &grad->image;
    vg_lite_matrix_t * matrix = &grad->matrix;
    vg_lite_uint32_t rad_tile = 0;
    vg_lite_uint32_t transparency_mode = 0;
    vg_lite_uint32_t in_premult = 0;
    vg_lite_uint32_t src_premultiply_enable = 0;
    vg_lite_uint32_t premul_flag = 0;
    vg_lite_uint32_t prediv_flag = 0;
    vg_lite_void *data;

    /* The following code is from "draw path" */
    vg_lite_uint32_t format, quality, tiling, fill;
    vg_lite_uint32_t tessellation_size;

    vg_lite_kernel_allocate_t memory;
    vg_lite_kernel_free_t free_memory;
    vg_lite_uint32_t return_offset = 0;

    vg_lite_point_t point_min = {0}, point_max = {0}, temp = {0};
    vg_lite_int32_t x, y, width, height;
    vg_lite_uint8_t ts_is_fullscreen = 0;

    vg_lite_float_t radius = grad->radial_grad.r;

    vg_lite_float_t centerX, centerY;
    vg_lite_float_t focalX, focalY;
    vg_lite_float_t fx, fy;
    vg_lite_float_t fxfy_2;
    vg_lite_float_t radius2;
    vg_lite_float_t r2_fx2, r2_fy2;
    vg_lite_float_t r2_fx2_2, r2_fy2_2;
    vg_lite_float_t r2_fx2_fy2;
    vg_lite_float_t r2_fx2_fy2sq;
    vg_lite_float_t cx, cy;

    vg_lite_float_t rgConstantLin, rgStepXLin, rgStepYLin;
    vg_lite_float_t rgConstantRad, rgStepXRad, rgStepYRad;
    vg_lite_float_t rgStepXXRad, rgStepYYRad, rgStepXYRad;

    VG_LITE_TRACE_API("vg_lite_draw_radial_grad %p %p %d %p %p 0x%08X %d %d\n",
        target, path, fill_rule, path_matrix, grad, paint_color, blend, filter);

#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_CHECK_NULL_POINTER2(path, path->path);
    VG_LITE_RETURN_ERROR(feature_check_lvgl_blend_mode(blend));
    VG_LITE_RETURN_ERROR(feature_check_8x_csaa_support(path->quality));
    VG_LITE_RETURN_ERROR(check_draw_pattern_source_format(source->format));
    VG_LITE_RETURN_ERROR(chip_check_target_format(target->format));

    if (radius < 0) {
        return VG_LITE_INVALID_ARGUMENT;
    }
    VG_LITE_RETURN_ERROR(feature_check_compress(source->format, source->compress_mode, source->tiled, source->width, source->height));
#endif /* gcFEATURE_VG_ERROR_CHECK */

    if (!path->path_length) {
        return VG_LITE_SUCCESS;
    }

    if (!path_matrix) {
        path_matrix = &identity_mtx;
    }

#if gcFEATURE_VG_GAMMA
    set_gamma_dest_only(target, VGL_TRUE);
#endif

    /*blend input into context*/
    s_context.blend_mode = blend;

    src_premultiply_enable = 0x01000100;
    if (s_context.color_transform == 0 && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
    else {
        prediv_flag = 1;
    }
    if ((s_context.blend_mode >= OPENVG_BLEND_SRC_OVER && s_context.blend_mode <= OPENVG_BLEND_ADDITIVE) || source->image_mode == VG_LITE_STENCIL_MODE) {
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
    if ((source->format == VG_LITE_A4 || source->format == VG_LITE_A8) && blend >= VG_LITE_BLEND_SRC_OVER && blend <= VG_LITE_BLEND_SUBTRACT) {
        chip_adjust_src_premultiply_enable(&src_premultiply_enable);
        in_premult = 0x00000000;
    }
    if (blend == VG_LITE_BLEND_NORMAL_LVGL) {
        in_premult = 0x00000000;
    }
    if (source->premultiplied == target->premultiplied && premul_flag == 0) {
        target->apply_premult = 1;
    }
    else {
        target->apply_premult = 0;
    }

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        if (error == VG_LITE_NO_CONTEXT) {
            /* If scissoring is enabled and no valid scissoring rectangles
               are present, no drawing occurs */
            return VG_LITE_SUCCESS;
        }
        else {
            return error;
        }
    }

    transparency_mode = (source->transparency_mode == VG_LITE_IMAGE_TRANSPARENT ? 0x8000:0);
    width = s_context.tessbuf.tess_w_h & 0xFFFF;
    height = s_context.tessbuf.tess_w_h >> 16;
    get_format_bytes(target->format, &mul, &div, &align);
    dst_align_width = target->stride * div / mul;
    if (width == 0 || height == 0)
        return VG_LITE_NO_CONTEXT;
    if ((dst_align_width <= width) && (target->height <= height) && !s_context.scissor_set)
    {
        ts_is_fullscreen = 1;
        point_min.x = 0;
        point_min.y = 0;
        point_max.x = dst_align_width;
        point_max.y = target->height;
    }

    /* If target is L8 and source is in YUV or RGB (not L8 or A8) then we have to convert RGB into L8. */
    if ((target->format == VG_LITE_L8) && ((source->format != VG_LITE_L8) && (source->format != VG_LITE_A8))) {
        conversion = 0x80000000;
    }

    /* Determine image mode (NORMAL or MULTIPLY) depending on the color. */
    imageMode = (source->image_mode == VG_LITE_NONE_IMAGE_MODE) ? 0 : (source->image_mode == VG_LITE_MULTIPLY_IMAGE_MODE) ? 0x00002000 : 0x00001000;
    tiled_source = (source->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0 ;

    switch (grad->spread_mode) {
        case VG_LITE_GRADIENT_SPREAD_FILL:
            rad_tile = 0x0;
            break;

        case VG_LITE_GRADIENT_SPREAD_PAD:
            rad_tile = 0x1000;
            break;

        case VG_LITE_GRADIENT_SPREAD_REPEAT:
            rad_tile = 0x2000;
            break;

        case VG_LITE_GRADIENT_SPREAD_REFLECT:
            rad_tile = 0x3000;
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

    if (grad->spread_mode == VG_LITE_GRADIENT_SPREAD_FILL)
    {
        vg_lite_uint8_t a,r,g,b;
        a = paint_color >> 24;
        r = paint_color >> 16;
        g = paint_color >> 8;
        b = paint_color;
        paint_color = (a << 24) | (b << 16) | (g << 8) | r;
    }

    /* compute radial gradient paremeters */

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix))
        return VG_LITE_SUCCESS;

    /* Make shortcuts to the gradient information. */
    centerX = grad->radial_grad.cx;
    centerY = grad->radial_grad.cy;
    focalX  = grad->radial_grad.fx;
    focalY  = grad->radial_grad.fy;

    /* Compute constants of the equation. */
    fx           = focalX - centerX;
    fy           = focalY - centerY;
    radius2      = radius * radius;
    if (fx*fx + fy*fy > radius2)
    {
        /* If the focal point is outside the circle, let's move it 
            to inside the circle. Per vg11 spec pg125 "If (fx, fy) lies outside ... 
            For here, we set it at 0.9 ratio to the center.
        */
        vg_lite_float_t fr = (vg_lite_float_t)sqrt((vg_lite_double_t)(fx*fx + fy*fy));
        fx = radius * fx / fr * 0.9f;
        fy = radius * fy / fr * 0.9f;
        focalX = grad->radial_grad.fx + fx;
        focalY = grad->radial_grad.fy + fy;
    }

    fxfy_2       = 2.0f * fx * fy;
    r2_fx2       = radius2 - fx * fx;
    r2_fy2       = radius2 - fy * fy;
    r2_fx2_2     = 2.0f * r2_fx2;
    r2_fy2_2     = 2.0f * r2_fy2;
    r2_fx2_fy2   = r2_fx2  - fy * fy;
    r2_fx2_fy2sq = r2_fx2_fy2 * r2_fx2_fy2;

    /*                        _____________________________________
    **      dx fx + dy fy + \/r^2 (dx^2 + dy^2) - (dx fy - dy fx)^2
    **  g = -------------------------------------------------------
    **                         r^2 - fx^2 - fy^2
    **
    **  Where
    **
    **      dx := F(x) - focalX
    **      dy := F(y) - focalY
    **      fx := focalX - centerX
    **      fy := focalX - centerY
    **
    **  and
    **
    **      F(x) := (x + 0.5) m00 + (y + 0.5) m01 + m02
    **      F(y) := (x + 0.5) m10 + (y + 0.5) m11 + m12
    **
    **  So, dx can be factored into
    **
    **      dx = (x + 0.5) m00 + (y + 0.5) m01 + m02 - focalX
    **         = x m00 + y m01 + 0.5 m00 + 0.5 m01 + m02 - focalX
    **
    **         = x m00 + y m01 + cx
    **
    **  where
    **
    **      cx := 0.5 m00 + 0.5 m01 + m02 - focalX
    **
    **  The same way we can factor dy into
    **
    **      dy = x m10 + y m11 + cy
    **
    **  where
    **
    **      cy := 0.5 m10 + 0.5 m11 + m12 - focalY.
    **
    **  Now we can rewrite g as
    **                               ______________________________________
    **        dx fx + dy fy         / r^2 (dx^2 + dy^2) - (dx fy - dy fx)^2
    **  g = ----------------- + \  /  -------------------------------------
    **      r^2 - fx^2 - fy^2    \/           (r^2 - fx^2 - fy^2)^2
    **               ____
    **    = gLin + \/gRad
    **
    **  where
    **
    **                dx fx + dy fy
    **      gLin := -----------------
    **              r^2 - fx^2 - fy^2
    **
    **              r^2 (dx^2 + dy^2) - (dx fy - dy fx)^2
    **      gRad := -------------------------------------
    **                      (r^2 - fx^2 - fy^2)^2
    */

    cx
        = 0.5f * ( MAT(&inverse_matrix, 0, 0) + MAT(&inverse_matrix, 0, 1) )
        + MAT(&inverse_matrix, 0, 2)
        - focalX;

    cy
        = 0.5f * ( MAT(&inverse_matrix, 1, 0) + MAT(&inverse_matrix, 1, 1) )
        + MAT(&inverse_matrix, 1, 2)
        - focalY;

    /*
    **            dx fx + dy fy
    **  gLin := -----------------
    **          r^2 - fx^2 - fy^2
    **
    **  We can factor the top half into
    **
    **      = (x m00 + y m01 + cx) fx + (x m10 + y m11 + cy) fy
    **
    **      = x (m00 fx + m10 fy)
    **      + y (m01 fx + m11 fy)
    **      + cx fx + cy fy.
    */

    rgStepXLin
        = ( MAT(&inverse_matrix, 0, 0) * fx + MAT(&inverse_matrix, 1, 0) * fy )
        / r2_fx2_fy2;

    rgStepYLin
        = ( MAT(&inverse_matrix, 0, 1) * fx + MAT(&inverse_matrix, 1, 1) * fy )
        / r2_fx2_fy2;

    rgConstantLin = ( cx * fx  + cy * fy ) / r2_fx2_fy2;

    /*
    **          r^2 (dx^2 + dy^2) - (dx fy - dy fx)^2
    **  gRad := -------------------------------------
    **                  (r^2 - fx^2 - fy^2)^2
    **
    **          r^2 (dx^2 + dy^2) - dx^2 fy^2 - dy^2 fx^2 + 2 dx dy fx fy
    **       := ---------------------------------------------------------
    **                            (r^2 - fx^2 - fy^2)^2
    **
    **          dx^2 (r^2 - fy^2) + dy^2 (r^2 - fx^2) + 2 dx dy fx fy
    **       := -----------------------------------------------------
    **                          (r^2 - fx^2 - fy^2)^2
    **
    **  First, lets factor dx^2 into
    **
    **      dx^2 = (x m00 + y m01 + cx)^2
    **           = x^2 m00^2 + y^2 m01^2 + 2 x y m00 m01
    **           + 2 x m00 cx + 2 y m01 cx + cx^2
    **
    **           = x^2 (m00^2)
    **           + y^2 (m01^2)
    **           + x y (2 m00 m01)
    **           + x (2 m00 cx)
    **           + y (2 m01 cx)
    **           + cx^2.
    **
    **  The same can be done for dy^2:
    **
    **      dy^2 = x^2 (m10^2)
    **           + y^2 (m11^2)
    **           + x y (2 m10 m11)
    **           + x (2 m10 cy)
    **           + y (2 m11 cy)
    **           + cy^2.
    **
    **  Let's also factor dx dy into
    **
    **      dx dy = (x m00 + y m01 + cx) (x m10 + y m11 + cy)
    **            = x^2 m00 m10 + y^2 m01 m11 + x y m00 m11 + x y m01 m10
    **            + x m00 cy + x m10 cx + y m01 cy + y m11 cx + cx cy
    **
    **            = x^2 (m00 m10)
    **            + y^2 (m01 m11)
    **            + x y (m00 m11 + m01 m10)
    **            + x (m00 cy + m10 cx)
    **            + y (m01 cy + m11 cx)
    **            + cx cy.
    **
    **  Now that we have all this, lets look at the top of gRad.
    **
    **      = dx^2 (r^2 - fy^2) + dy^2 (r^2 - fx^2) + 2 dx dy fx fy
    **      = x^2 m00^2 (r^2 - fy^2) + y^2 m01^2 (r^2 - fy^2)
    **      + x y 2 m00 m01 (r^2 - fy^2) + x 2 m00 cx (r^2 - fy^2)
    **      + y 2 m01 cx (r^2 - fy^2) + cx^2 (r^2 - fy^2)
    **      + x^2 m10^2 (r^2 - fx^2) + y^2 m11^2 (r^2 - fx^2)
    **      + x y 2 m10 m11 (r^2 - fx^2) + x 2 m10 cy (r^2 - fx^2)
    **      + y 2 m11 cy (r^2 - fx^2) + cy^2 (r^2 - fx^2)
    **      + x^2 m00 m10 2 fx fy + y^2 m01 m11 2 fx fy
    **      + x y (m00 m11 + m01 m10) 2 fx fy
    **      + x (m00 cy + m10 cx) 2 fx fy + y (m01 cy + m11 cx) 2 fx fy
    **      + cx cy 2 fx fy
    **
    **      = x^2 ( m00^2 (r^2 - fy^2)
    **            + m10^2 (r^2 - fx^2)
    **            + m00 m10 2 fx fy
    **            )
    **      + y^2 ( m01^2 (r^2 - fy^2)
    **            + m11^2 (r^2 - fx^2)
    **            + m01 m11 2 fx fy
    **            )
    **      + x y ( 2 m00 m01 (r^2 - fy^2)
    **            + 2 m10 m11 (r^2 - fx^2)
    **            + (m00 m11 + m01 m10) 2 fx fy
    **            )
    **      + x ( 2 m00 cx (r^2 - fy^2)
    **          + 2 m10 cy (r^2 - fx^2)
    **          + (m00 cy + m10 cx) 2 fx fy
    **          )
    **      + y ( 2 m01 cx (r^2 - fy^2)
    **          + 2 m11 cy (r^2 - fx^2)
    **          + (m01 cy + m11 cx) 2 fx fy
    **          )
    **      + cx^2 (r^2 - fy^2) + cy^2 (r^2 - fx^2) + cx cy 2 fx fy.
    */

    rgStepXXRad =
        (
                MAT(&inverse_matrix, 0, 0) * MAT(&inverse_matrix, 0, 0) * r2_fy2
            + MAT(&inverse_matrix, 1, 0) * MAT(&inverse_matrix, 1, 0) * r2_fx2
            + MAT(&inverse_matrix, 0, 0) * MAT(&inverse_matrix, 1, 0) * fxfy_2
        )
        / r2_fx2_fy2sq;

    rgStepYYRad =
        (
                MAT(&inverse_matrix, 0, 1) * MAT(&inverse_matrix, 0, 1) * r2_fy2
            + MAT(&inverse_matrix, 1, 1) * MAT(&inverse_matrix, 1, 1) * r2_fx2
            + MAT(&inverse_matrix, 0, 1) * MAT(&inverse_matrix, 1, 1) * fxfy_2
        )
        / r2_fx2_fy2sq;

    rgStepXYRad =
        (
                MAT(&inverse_matrix, 0, 0) * MAT(&inverse_matrix, 0, 1) * r2_fy2_2
            + MAT(&inverse_matrix, 1, 0) * MAT(&inverse_matrix, 1, 1) * r2_fx2_2
            + (
                    MAT(&inverse_matrix, 0, 0) * MAT(&inverse_matrix, 1, 1)
                + MAT(&inverse_matrix, 0, 1) * MAT(&inverse_matrix, 1, 0)
                )
                * fxfy_2
        )
        / r2_fx2_fy2sq;

    rgStepXRad =
        (
                MAT(&inverse_matrix, 0, 0) * cx * r2_fy2_2
            + MAT(&inverse_matrix, 1, 0) * cy * r2_fx2_2
            + (
                    MAT(&inverse_matrix, 0, 0) * cy
                + MAT(&inverse_matrix, 1, 0) * cx
                )
                * fxfy_2
        )
        / r2_fx2_fy2sq;

    rgStepYRad =
        (
                MAT(&inverse_matrix, 0, 1) * cx * r2_fy2_2
            + MAT(&inverse_matrix, 1, 1) * cy * r2_fx2_2
            + (
                    MAT(&inverse_matrix, 0, 1) * cy
                + MAT(&inverse_matrix, 1, 1) * cx
                )
                * fxfy_2
        )
        / r2_fx2_fy2sq;

    rgConstantRad =
        (
                cx * cx * r2_fy2
            + cy * cy * r2_fx2
            + cx * cy * fxfy_2
        )
        / r2_fx2_fy2sq;

    /* Setup the command buffer. */
    data = &rgConstantLin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A04,*(vg_lite_uint32_t*) data));
    data = &rgStepXLin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A06,*(vg_lite_uint32_t*) data));
    data = &rgStepYLin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A08,*(vg_lite_uint32_t*) data));
    data = &rgConstantRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A05,*(vg_lite_uint32_t*) data));
    data = &rgStepXRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A07,*(vg_lite_uint32_t*) data));
    data = &rgStepYRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A09,*(vg_lite_uint32_t*) data));
    data = &rgStepXXRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A03,*(vg_lite_uint32_t*) data));
    data = &rgStepYYRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A0A,*(vg_lite_uint32_t*) data));
    data = &rgStepXYRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A0B,*(vg_lite_uint32_t*) data));
    VG_LITE_RETURN_ERROR(set_interpolation_steps(source->width, source->height, matrix, 1, NULL));

    /* enable pre-multiplied in image unit */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A24, convert_source_format(source->format) |
                                                            filter_mode | rad_tile | conversion | src_premultiply_enable));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A26, paint_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A28, source->address));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2A, tiled_source));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2C, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2E, source->width));

    /* Work on path states. */
    matrix = path_matrix;

    if (ts_is_fullscreen == 0){
        if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH){
            vg_lite_float_t add_width = path->stroke->line_width;
            if (path->stroke->join_style == VG_LITE_JOIN_MITER)
                add_width += path->stroke->miter_limit;
            add_width = 1.5f * add_width;
            path->bounding_box[0] -= add_width;
            path->bounding_box[1] -= add_width;
            path->bounding_box[2] += add_width;
            path->bounding_box[3] += add_width;
        }
        
        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[1], matrix);
        point_min = point_max = temp;

        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[1], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        point_min.x = MAX(point_min.x, 0);
        point_min.y = MAX(point_min.y, 0);
        point_max.x = MIN(point_max.x, dst_align_width);
        point_max.y = MIN(point_max.y, target->height);

        if (s_context.scissor_set) {
            point_min.x = MAX(point_min.x, s_context.scissor[0]);
            point_min.y = MAX(point_min.y, s_context.scissor[1]);
            point_max.x = MIN(point_max.x, s_context.scissor[0] + s_context.scissor[2]);
            point_max.y = MIN(point_max.y, s_context.scissor[1] + s_context.scissor[3]);
        }
    }

    if (point_min.x < 0) point_min.x = 0;
    if (point_min.y < 0) point_min.y = 0;
    if (point_max.x > target->width) point_max.x = target->width;
    if (point_max.y > target->height) point_max.x = target->height;

    /* Convert states into hardware values. */
    blend_mode = convert_blend(blend);
    format = convert_path_format(path->format);
    quality = convert_path_quality(path->quality);
    tiling = (s_context.capabilities.cap.tiled == 2) ? 0x2000000 : 0;
    fill = (fill_rule == VG_LITE_FILL_EVEN_ODD) ? 0x10 : 0;
    tessellation_size = s_context.tessbuf.L2_size ? s_context.tessbuf.L2_size : s_context.tessbuf.L1_size;

    /* Setup the command buffer. */
    /* Program color register. */

    /* enable pre-multiplied from VG to VGPE */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x02000002 | s_context.capabilities.cap.tiled | in_premult | imageMode | blend_mode | transparency_mode | s_context.enable_mask | s_context.scissor_enable | s_context.color_transform | s_context.matrix_enable));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000400 | format | quality | tiling | fill));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3B, 0x3F800000));      /* Path tessellation SCALE. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3C, 0x00000000));      /* Path tessellation BIAS.  */
    /* Program matrix. */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A40, (vg_lite_void *) &matrix->m[0][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A41, (vg_lite_void *) &matrix->m[0][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A42, (vg_lite_void *) &matrix->m[0][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A43, (vg_lite_void *) &matrix->m[1][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A44, (vg_lite_void *) &matrix->m[1][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A45, (vg_lite_void *) &matrix->m[1][2]));

    if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
    {
        if (path->path_changed != 0) {
            if (path->uploaded.handle != NULL) {
                free_memory.memory_handle = path->uploaded.handle;
                vg_lite_kernel(VG_LITE_FREE, &free_memory);
                path->uploaded.address = 0;
                path->uploaded.memory = NULL;
                path->uploaded.handle = NULL;
            }
            /* Allocate memory for the path data. */
            memory.bytes = 16 + VG_LITE_ALIGN(path->path_length, 8);
            return_offset = (8 + VG_LITE_ALIGN(path->path_length, 8)) / 4;
            memory.contiguous = 1;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &memory));
            ((uint64_t *) memory.memory)[(path->path_length + 7) / 8] = 0;
            ((vg_lite_uint32_t *) memory.memory)[0] = VG_LITE_DATA((path->path_length + 7) / 8);
            ((vg_lite_uint32_t *) memory.memory)[1] = 0;
            memcpy((vg_lite_uint8_t *) memory.memory + 8, path->path, path->path_length);
            ((vg_lite_uint32_t *) memory.memory)[return_offset] = VG_LITE_RETURN();
            ((vg_lite_uint32_t *) memory.memory)[return_offset + 1] = 0;

            path->uploaded.handle = memory.memory_handle;
            path->uploaded.memory = memory.memory;
            path->uploaded.address = memory.memory_gpu;
            path->uploaded.bytes  = memory.bytes;
            path->path_changed = 0;
        }
    }

    /* Setup tessellation loop. */
    vg_lite_int32_t tem_width = point_max.x - point_min.x;
    vg_lite_int32_t tem_height = point_max.y - point_min.y;
    if (path->path_type == VG_LITE_DRAW_FILL_PATH || path->path_type == VG_LITE_DRAW_ZERO || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {
        for (y = point_min.y; y < point_max.y; y += height) {
            for (x = point_min.x; x < point_max.x; x += width) {
                /* Tessellate path. */
                VG_LITE_RETURN_ERROR(push_stall(&s_context, 15));
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(x | (y << 16)), 0, 1));
                VG_LITE_RETURN_ERROR(push_state_tess_path_w_h(tem_width, tem_height, width, height));

                if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
                }
                else {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
                }
                tem_width -= width;
            }
            tem_width = point_max.x - point_min.x;
            tem_height -= height;
        }
    }
    /* Setup tessellation loop. */
    tem_width = point_max.x - point_min.x;
    tem_height = point_max.y - point_min.y;
    if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {
        for (y = point_min.y; y < point_max.y; y += height) {
            for (x = point_min.x; x < point_max.x; x += width) {
                /* Tessellate path. */
                VG_LITE_RETURN_ERROR(push_stall(&s_context, 15));
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(x | (y << 16)), 0, 1));
                format = convert_path_format(VG_LITE_FP32);
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0x0));
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, path->stroke_color));
                VG_LITE_RETURN_ERROR(push_state_tess_path_w_h(tem_width, tem_height, width, height));

                if (VLM_PATH_STROKE_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->stroke->uploaded.address, path->stroke->uploaded.bytes));
                } else {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
                }
                tem_width -= width;
            }
            tem_width = point_max.x - point_min.x;
            tem_height -= height;
        }
    }

    /* Finialize command buffer. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));

    return error;
}

vg_lite_error_t vg_lite_split_path(vg_lite_uint32_t endis)
{
    return VG_LITE_SUCCESS;
}

#else /* (CHIPID==0x355 || CHIPID==0x255) */

vg_lite_error_t vg_lite_split_path(vg_lite_uint32_t endis)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_SPLIT_PATH_API);

#if gcFEATURE_COMBO_VG_SPLIT_PATH_SUPPORT_BY_SW
    s_context.split_path = endis;
#endif

    return VG_LITE_SUCCESS;
}

vg_lite_error_t push_path_base(vg_lite_path_t* path, vg_lite_uint32_t quality, vg_lite_uint32_t tiling, vg_lite_uint32_t in_premult, vg_lite_uint32_t blend_mode, vg_lite_int8_t func_flag)
{
    vg_lite_error_t error;
    vg_lite_uint32_t format;

    if (path->path_type == VG_LITE_DRAW_FILL_PATH || path->path_type == VG_LITE_DRAW_ZERO || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {
        if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
            VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
        }
        else {
            VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
        }
    }

    if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {

        if (func_flag == 2)
        {
            if (path->stroke_paint_type != VG_LITE_PAINT_LINEAR_GRADIENT)
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000002 | s_context.capabilities.cap.tiled | in_premult | blend_mode | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | s_context.scissor_enable));
        }
        else if (func_flag == 3)
        {
            if (path->stroke_paint_type != VG_LITE_PAINT_RADIAL_GRADIENT)
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000002 | s_context.capabilities.cap.tiled | in_premult | blend_mode | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | s_context.scissor_enable));
        }

        format = convert_path_format(VG_LITE_FP32);
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0x0));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, path->stroke_color));

        if (VLM_PATH_STROKE_GET_UPLOAD_BIT(*path) == 1) {
            VG_LITE_RETURN_ERROR(push_call(&s_context, path->stroke->uploaded.address, path->stroke->uploaded.bytes));
        }
        else {
            VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
        }
    }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t stroke_config(vg_lite_path_t* path, vg_lite_matrix_t* matrix, vg_lite_point_t point_min, vg_lite_int32_t width, vg_lite_int32_t height)
{
    vg_lite_error_t error;
    vg_lite_float_t ts_max_scale_factor;
    vg_lite_float_t temp_sqrt_value = sqrtf(matrix->m[0][0] * matrix->m[0][0] + matrix->m[0][1] * matrix->m[0][1] + matrix->m[1][0] * matrix->m[1][0] + matrix->m[1][1] * matrix->m[1][1]);

    if (matrix->m[2][0] == 0.0 && matrix->m[2][1] == 0.0 && matrix->m[2][2] == 1.0)
    {
        ts_max_scale_factor = temp_sqrt_value;

        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0C14, (void*)&ts_max_scale_factor));
    }
    else
    {
        vg_lite_float_t temp_fabs_value_1 = fabsf(matrix->m[2][0] * point_min.x + matrix->m[2][1] * point_min.y + matrix->m[2][2]);
        vg_lite_float_t temp_fabs_value_2 = fabsf(matrix->m[2][0] * (point_min.x + width) + matrix->m[2][1] * point_min.y + matrix->m[2][2]);
        vg_lite_float_t temp_fabs_value_3 = fabsf(matrix->m[2][0] * point_min.x + matrix->m[2][1] * (point_min.y + height) + matrix->m[2][2]);
        vg_lite_float_t temp_fabs_value_4 = fabsf(matrix->m[2][0] * (point_min.x + width) + matrix->m[2][1] * (point_min.y + height) + matrix->m[2][2]);
        ts_max_scale_factor = temp_sqrt_value / MIN(MIN(temp_fabs_value_1, temp_fabs_value_2), MIN(temp_fabs_value_3, temp_fabs_value_4));

        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0C14, (void*)&ts_max_scale_factor));
    }

    vg_lite_uint32_t cap_style_register_value = 0x000003 & path->stroke->cap_style;
    vg_lite_uint32_t join_style_register_value = (0x000003 & path->stroke->join_style) << 2;
    vg_lite_uint32_t non_scale_register_value = (0x000001 & path->stroke->non_scale_flag) << 11;
    vg_lite_uint32_t stroke_quality_register_value = convert_path_quality(path->stroke->quality) << 25;

    if (path->stroke->non_scale_flag)
    {
        vg_lite_uint32_t int_half_line_width_value = (vg_lite_uint32_t)ceil(path->stroke->half_width);
        int_half_line_width_value = int_half_line_width_value << 12;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C07, stroke_quality_register_value | join_style_register_value | cap_style_register_value | non_scale_register_value | int_half_line_width_value));
    }
    else
    {
        vg_lite_uint32_t int_half_line_width_value = (vg_lite_uint32_t)ceil(path->stroke->half_width * ts_max_scale_factor);
        int_half_line_width_value = int_half_line_width_value << 12;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C07, stroke_quality_register_value | join_style_register_value | cap_style_register_value | non_scale_register_value | int_half_line_width_value));
    }

    vg_lite_float_t half_width_register_value = (vg_lite_float_t)path->stroke->half_width;
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0C08, (void*)&half_width_register_value));

    return VG_LITE_SUCCESS;
}

vg_lite_error_t push_path_advance(vg_lite_path_t* path, vg_lite_uint32_t quality, vg_lite_uint32_t tiling, vg_lite_uint32_t in_premult, vg_lite_uint32_t blend_mode, vg_lite_int8_t func_flag)
{
    vg_lite_error_t error;
    vg_lite_uint32_t format;

    if (path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH)
    {
        if (func_flag == 0)
        {
            if (path->stroke->pattern_count != 0)
            {
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C06, 0x00000000));
                if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
                {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
                }
                else
                {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
                }

                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C06, 0x00000001));
                format = convert_path_format(VG_LITE_FP32);
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0x0));
                VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
            }
            else
            {
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C06, 0x00000002));
                if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
                {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
                }
                else
                {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
                }
            }
        }
        else if(func_flag == 1 || func_flag == 2 || func_flag == 3)
        {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C06, 0x00000000));

            if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
            {
                VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
            }
            else
            {
                VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
            }
            if (func_flag == 1)
            {
                if (path->stroke_paint_type != VG_LITE_PAINT_PATTERN)
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000002 | s_context.capabilities.cap.tiled | in_premult | blend_mode | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | s_context.scissor_enable));
            }
            if (func_flag == 2)
            {
                if (path->stroke_paint_type != VG_LITE_PAINT_LINEAR_GRADIENT)
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000002 | s_context.capabilities.cap.tiled | in_premult | blend_mode | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | s_context.scissor_enable));
            }
            if (func_flag == 3)
            {
                if (path->stroke_paint_type != VG_LITE_PAINT_RADIAL_GRADIENT)
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000002 | s_context.capabilities.cap.tiled | in_premult | blend_mode | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | s_context.scissor_enable));
            }
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C06, 0x00000001));

            if (path->stroke->pattern_count != 0)
            {
                format = convert_path_format(VG_LITE_FP32);
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0X0));
                VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
            }
            else
            {
                if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
                {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
                }
                else
                {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
                }

            }
        }

    }
    else if (path->path_type == VG_LITE_DRAW_STROKE_PATH)
    {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C06, 0x00000001));

        if (func_flag == 1)
        {
            if (path->stroke_paint_type != VG_LITE_PAINT_PATTERN)
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000002 | s_context.capabilities.cap.tiled | in_premult | blend_mode | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | s_context.scissor_enable));
        }
        if (func_flag == 2)
        {
            if (path->stroke_paint_type != VG_LITE_PAINT_LINEAR_GRADIENT)
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000002 | s_context.capabilities.cap.tiled | in_premult | blend_mode | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | s_context.scissor_enable));
        }
        if (func_flag == 3)
        {
            if (path->stroke_paint_type != VG_LITE_PAINT_RADIAL_GRADIENT)
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x00000002 | s_context.capabilities.cap.tiled | in_premult | blend_mode | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | s_context.scissor_enable));
        }

        if (path->stroke->pattern_count != 0)
        {
            format = convert_path_format(VG_LITE_FP32);
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0x0));
            VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
        }
        else
        {
            if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
            {
                VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
            }
            else
            {
                VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
            }
        }
    }
    else
    {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C06, 0x00000000));
        if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
        {
            VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
        }
        else
        {
            VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
        }
    }
    
    return VG_LITE_SUCCESS; 
}
   
/* GC555 vg_lite_draw API implementation
 */
vg_lite_error_t vg_lite_draw(vg_lite_buffer_t* target,
                            vg_lite_path_t* path,
                            vg_lite_fill_t fill_rule,
                            vg_lite_matrix_t* matrix,
                            vg_lite_blend_t blend,
                            vg_lite_color_t color)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DRAW_API);
    DUMP_API_CALL(vg_lite_draw, target, path, fill_rule, matrix, blend, color);

    vg_lite_uint32_t blend_mode;
    vg_lite_uint32_t format, quality, tiling, fill;
    vg_lite_uint32_t tessellation_size;
    vg_lite_error_t error;
    vg_lite_point_t point_min = { 0 }, point_max = { 0 }, temp = { 0 };
    vg_lite_int32_t width, height;
    vg_lite_uint8_t ts_is_fullscreen = 0;
    vg_lite_uint32_t return_offset = 0;
    vg_lite_kernel_free_t free_memory;
    vg_lite_kernel_allocate_t memory;
    vg_lite_float_t new_matrix[6];
    vg_lite_float_t scale, bias;
    vg_lite_uint32_t tile_setting = 0;
    vg_lite_uint32_t in_premult = 0;
    vg_lite_uint32_t premul_flag = 0;

#if gcFEATURE_VG_NEW_FACTOR
    vg_factor_config_t factor_config;
    factor_config.factor_src_alpha = 0x0;
    factor_config.factor_src_color = 0x0;
    factor_config.factor_dst_alpha = 0x3;
    factor_config.factor_dst_color = 0x5;
    factor_config.final_equation_opcode = 0x0;
    factor_config.dstchannelmode = 0x0;
    factor_config.srcchannelmode = 0x0;
    vg_lite_porter_duff_config_t porter_duff_config;
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

    VG_LITE_TRACE_API("vg_lite_draw %p %p %d %p %d 0x%08X\n", target, path, fill_rule, matrix, blend, color);
    VG_LITE_TRACE_API("    path_type %d, path_length %d, stroke_size %d\n", path->path_type, path->path_length, path->stroke_size);

#if gcFEATURE_VG_FLEXA
    if (s_context.sync_mode)
    {
        printf("When Flexa is enabled vg_lite_draw is not support.\n");
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_CHECK_NULL_POINTER2(path, path->path);
    VG_LITE_RETURN_ERROR(feature_check_8x_csaa_support(path->quality));
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_new_blend_mode(blend));
    VG_LITE_RETURN_ERROR(feature_check_a124_a8l8_target_format(target->format));
#endif /* gcFEATURE_VG_ERROR_CHECK */

    if (!path->path_length) {
        return VG_LITE_SUCCESS;
    }

    if (!matrix) {
        matrix = &identity_mtx;
    }

#if gcFEATURE_VG_GAMMA
    set_gamma_dest_only(target, VGL_FALSE);
#endif

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_GLOBAL));
#endif

    /*blend input into context*/
    s_context.blend_mode = blend;

    /* Adjust premultiply setting according to openvg condition */
    target->apply_premult = 0;
    premul_flag = (s_context.blend_mode >= OPENVG_BLEND_SRC_OVER && s_context.blend_mode <= OPENVG_BLEND_ADDITIVE)
                  || is_lvgl_blend_mode(s_context.blend_mode)
                  ;

    if (target->premultiplied == 0 && premul_flag == 0) {
        in_premult = 0x10000000;
        target->apply_premult = 1;
    }
    else if ((target->premultiplied == 1) ||
             (target->premultiplied == 0 && premul_flag == 1)) {
        in_premult = 0x00000000;
    }
    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }

    if (is_packed_yuy_format(target->format) && path->quality != VG_LITE_LOW) {
        path->quality = VG_LITE_LOW;
        printf("If target is YUV group , the path qulity should use VG_LITE_LOW.\n");
    }

    width = target->width;
    height = target->height;
    if (s_context.scissor_set) {
        width = s_context.scissor[2] - s_context.scissor[0];
        height = s_context.scissor[3] - s_context.scissor[1];
    }
    if (width == 0 || height == 0)
        return VG_LITE_NO_CONTEXT;
    if ((target->width <= width) && (target->height <= height) && (!s_context.scissor_set))
    {
        ts_is_fullscreen = 1;
        point_min.x = 0;
        point_min.y = 0;
        point_max.x = target->width;
        point_max.y = target->height;
    }

    if (ts_is_fullscreen == 0) {
        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[1], matrix);
        point_min = point_max = temp;

        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[1], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        if (point_min.x < 0) point_min.x = 0;
        if (point_min.y < 0) point_min.y = 0;
        if (point_max.x > target->width) point_max.x = target->width;
        if (point_max.y > target->height) point_max.y = target->height;

        if (s_context.scissor_set) {
            point_min.x = MAX(point_min.x, s_context.scissor[0]);
            point_min.y = MAX(point_min.y, s_context.scissor[1]);
            point_max.x = MIN(point_max.x, s_context.scissor[2]);
            point_max.y = MIN(point_max.y, s_context.scissor[3]);
        }
    }

    width = point_max.x - point_min.x;
    height = point_max.y - point_min.y;
    scale = 1.0f;
    bias = 0.0f;
    new_matrix[0] = matrix->m[0][0] * scale;
    new_matrix[1] = matrix->m[0][1] * scale;
    new_matrix[2] = (matrix->m[0][0] + matrix->m[0][1]) * bias + matrix->m[0][2];
    new_matrix[3] = matrix->m[1][0] * scale;
    new_matrix[4] = matrix->m[1][1] * scale;
    new_matrix[5] = (matrix->m[1][0] + matrix->m[1][1]) * bias + matrix->m[1][2];

    /* Convert states into hardware values. */
    blend_mode = convert_blend(blend);
    format = convert_path_format(path->format);
    quality = convert_path_quality(path->quality);
    tiling = (s_context.capabilities.cap.tiled == 2) ? 0x2000000 : 0;
    fill = (fill_rule == VG_LITE_FILL_EVEN_ODD) ? 0x10 : 0;
    tessellation_size = s_context.tessbuf.tessbuf_size;

    VG_LITE_RETURN_ERROR(chip_set_tes_tile(target, &tile_setting));

    /* Setup the command buffer. */
    /* Program color register. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, in_premult | s_context.capabilities.cap.tiled | blend_mode | tile_setting | s_context.enable_mask | s_context.scissor_enable | s_context.color_transform | s_context.matrix_enable));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color));
    /* Program tessellation control: for TS module. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000000 | format | quality | tiling | fill));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3B, 0x3F800000));      /* Path tessellation SCALE. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3C, 0x00000000));      /* Path tessellation BIAS.  */
    /* Program matrix. */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A40, (vg_lite_pointer)&new_matrix[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A41, (vg_lite_pointer)&new_matrix[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A42, (vg_lite_pointer)&new_matrix[2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A43, (vg_lite_pointer)&new_matrix[3]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A44, (vg_lite_pointer)&new_matrix[4]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A45, (vg_lite_pointer)&new_matrix[5]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0ACD, (vg_lite_pointer)&matrix->m[0][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0ACE, (vg_lite_pointer)&matrix->m[1][2]));

    /* DDRLess does not support uploading path data. */
    if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
    {
        if (path->path_changed != 0) {
            if (path->uploaded.handle != NULL) {
                free_memory.memory_handle = path->uploaded.handle;
                VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free_memory));
                path->uploaded.address = 0;
                path->uploaded.memory = NULL;
                path->uploaded.handle = NULL;
            }
            /* Allocate memory for the path data. */
            memory.bytes = 16 + VG_LITE_ALIGN(path->path_length, 8);
            return_offset = (8 + VG_LITE_ALIGN(path->path_length, 8)) / 4;
            memory.contiguous = 1;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &memory));
            ((uint64_t*)memory.memory)[(path->path_length + 7) / 8] = 0;
            ((vg_lite_uint32_t*)memory.memory)[0] = VG_LITE_DATA((path->path_length + 7) / 8);
            ((vg_lite_uint32_t*)memory.memory)[1] = 0;
            memcpy((vg_lite_uint8_t*)memory.memory + 8, path->path, path->path_length);
            ((vg_lite_uint32_t*)memory.memory)[return_offset] = VG_LITE_RETURN();
            ((vg_lite_uint32_t*)memory.memory)[return_offset + 1] = 0;

            path->uploaded.handle = memory.memory_handle;
            path->uploaded.memory = memory.memory;
            path->uploaded.address = memory.memory_gpu;
            path->uploaded.bytes = memory.bytes;
            path->path_changed = 0;
        }
    }

#if DUMP_CAPTURE
    if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
        vglitemDUMP_BUFFER("path", (size_t)path->uploaded.address, (vg_lite_uint8_t*)(path->uploaded.memory), 0, path->uploaded.bytes);
    }
    vglitemDUMP("@[memory 0x%08X 0x%08X]", s_context.tessbuf.physical_addr, s_context.tessbuf.tessbuf_size);
#endif

    if (width + point_min.x > target->width) {
        width = target->width - point_min.x;
    }

#if gcFEATURE_VG_NEW_FACTOR
    porter_duff_config = s_context.porter_duff_config;
    config_factor_parameter(blend, porter_duff_config, &factor_config);

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF6, factor_config.srcchannelmode | (factor_config.dstchannelmode << 8)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF8, factor_config.factor_src_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF9, factor_config.factor_src_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFA, factor_config.factor_dst_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFB, factor_config.factor_dst_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF7, factor_config.final_equation_opcode));
#endif

#if gcFEATURE_COMBO_VG_SPLIT_PATH_SUPPORT_BY_SW
  if (s_context.split_path)
  {
    vg_lite_int32_t y = 0;
    vg_lite_uint32_t par_height = 0;
    vg_lite_int32_t next_boundary = 0;
#if (!gcFEATURE_VG_PARALLEL_PATHS_DISABLE)
    vg_lite_uint32_t parallel_workpaths1 = 2;
    vg_lite_uint32_t parallel_workpaths2 = 2;
#endif

    s_context.tessbuf.tess_w_h = width | (height << 16);
    s_context.tessbuf.tess_x_y = point_min.x | (point_min.y << 16);
    if (path->path_type == VG_LITE_DRAW_FILL_PATH || path->path_type == VG_LITE_DRAW_ZERO || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {
#if !gcFEATURE_VG_PARALLEL_PATHS_DISABLE
        if (height <= 128)
            parallel_workpaths1 = 4;
        else
            parallel_workpaths1 = height * 128 / 4096 - 1;

        if (parallel_workpaths1 > parallel_workpaths2)
            parallel_workpaths1 = parallel_workpaths2;
#endif
        for (y = point_min.y; y < point_max.y; y += par_height) {
#if (gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE && gcFEATURE_VG_512_HALF_SPLIT_DISABLE && !gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE)
            next_boundary = (y + 512) & 0xfffffe00;
#elif (gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE && !gcFEATURE_VG_512_HALF_SPLIT_DISABLE)
            if (height > 512)
                next_boundary = (y + 256);
            else
                next_boundary = (y + (height + 1) / 2);
#elif (!gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE)
            next_boundary = (y + 32) & 0xffffffe0;
#else
            next_boundary = (y + 16) & 0xfffffff0;
#endif
            par_height = ((next_boundary < point_max.y) ? next_boundary - y : (point_max.y - y));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, in_premult | s_context.capabilities.cap.tiled | blend_mode | tile_setting | s_context.enable_mask | s_context.scissor_enable | s_context.color_transform | s_context.matrix_enable));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color));
            /* Program tessellation control: for TS module. */
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000000 | format | quality | tiling | fill));
            VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                (vg_lite_uint32_t)(point_min.x | (y << 16)), (vg_lite_uint32_t)(width | (par_height << 16)), 0));

            if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
                VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
            }
            else {
                VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000101));
            }
#if !gcFEATURE_VG_PARALLEL_PATHS_DISABLE
            s_context.path_counter++;
            if (parallel_workpaths1 == s_context.path_counter) {
                VG_LITE_RETURN_ERROR(push_stall(&s_context, 7));
                s_context.path_counter = 0;
            }
#elif !gcFEATURE_VG_512_HALF_SPLIT_DISABLE && gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE
            VG_LITE_RETURN_ERROR(push_stall(&s_context, 7));
#endif
        }
    }
    if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {
#if !gcFEATURE_VG_PARALLEL_PATHS_DISABLE
        if (height <= 128)
            parallel_workpaths1 = 4;
        else
            parallel_workpaths1 = height * 128 / 4096 - 1;

        if (parallel_workpaths1 > parallel_workpaths2)
            parallel_workpaths1 = parallel_workpaths2;
#endif
        for (y = point_min.y; y < point_max.y; y += par_height) {
#if (gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE && gcFEATURE_VG_512_HALF_SPLIT_DISABLE && !gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE)
            next_boundary = (y + 512) & 0xfffffe00;
#elif (gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE && !gcFEATURE_VG_512_HALF_SPLIT_DISABLE)
            if (height > 512)
                next_boundary = (y + 256);
            else
                next_boundary = (y + (height + 1) / 2);
#elif (!gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE)
            next_boundary = (y + 32) & 0xffffffe0;
#else           
            next_boundary = (y + 16) & 0xfffffff0;
#endif
            par_height = ((next_boundary < point_max.y) ? next_boundary - y : (point_max.y - y));

            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, in_premult | s_context.capabilities.cap.tiled | blend_mode | tile_setting | s_context.enable_mask | s_context.scissor_enable | s_context.color_transform | s_context.matrix_enable));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
            VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                (vg_lite_uint32_t)(point_min.x | (y << 16)),
                (vg_lite_uint32_t)(width | (par_height << 16)), 0));

            format = convert_path_format(VG_LITE_FP32);
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0x0));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, path->stroke_color));

            if (VLM_PATH_STROKE_GET_UPLOAD_BIT(*path) == 1) {
                VG_LITE_RETURN_ERROR(push_call(&s_context, path->stroke->uploaded.address, path->stroke->uploaded.bytes));
            }
            else {
                VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
                
#if (CHIPID == 0x555)               
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000101));
#endif
                
            }
#if !gcFEATURE_VG_PARALLEL_PATHS_DISABLE
            s_context.path_counter++;
            if (parallel_workpaths1 == s_context.path_counter) {
                VG_LITE_RETURN_ERROR(push_stall(&s_context, 7));
                s_context.path_counter = 0;
            }
#elif !gcFEATURE_VG_512_HALF_SPLIT_DISABLE && gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE
            VG_LITE_RETURN_ERROR(push_stall(&s_context, 7));
#endif          
        }
    }
  }
  else
#endif /* gcFEATURE_COMBO_VG_SPLIT_PATH_SUPPORT_BY_SW */
#if gcFEATURE_VG_HW_STROKE
  {
        s_context.tessbuf.tess_w_h = width | (height << 16);
        s_context.tessbuf.tess_x_y = point_min.x | (point_min.y << 16);
        VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
            s_context.tessbuf.tess_x_y, s_context.tessbuf.tess_w_h, 0));

        vg_lite_uint16_t  no_use_hw_stroke_flag = 0;
        
        if(path->stroke)
            no_use_hw_stroke_flag = path->stroke->join_style == VG_LITE_JOIN_MITER || path->stroke->cap_style == VG_LITE_CAP_SQUARE || path->stroke->dash_phase != 0;
        
        if (!no_use_hw_stroke_flag)
        {
            if (path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH || path->path_type == VG_LITE_DRAW_STROKE_PATH)
            {
                VG_LITE_RETURN_ERROR(stroke_config(path, matrix, point_min, width, height));
            }
            VG_LITE_RETURN_ERROR(push_path_advance(path, quality, tiling,in_premult,  blend_mode, 0));
        }
        else
        {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C06, 0x00000000));
            VG_LITE_RETURN_ERROR(push_path_base(path, quality, tiling, 0, 0, 0));
        }
  }
#else
  {
        s_context.tessbuf.tess_w_h = width | (height << 16);
        s_context.tessbuf.tess_x_y = point_min.x | (point_min.y << 16);
        VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
            s_context.tessbuf.tess_x_y, s_context.tessbuf.tess_w_h, 0));
        VG_LITE_RETURN_ERROR(push_path_base(path, quality, tiling, 0, 0, 0));
  }
#endif

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_NORMAL));
#endif

#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((target->format >= VG_LITE_ABGR8565) && (target->format <= VG_LITE_RGBA5658))
    {
        if (target->sw24bit_planar_buffer)
            target = target->sw24bit_planar_buffer;
    }
#endif
    return error;
}

/* GC555 vg_lite_draw_pattern API implementation
 */
vg_lite_error_t vg_lite_draw_pattern(vg_lite_buffer_t *target,
                                    vg_lite_path_t *path,
                                    vg_lite_fill_t fill_rule,
                                    vg_lite_matrix_t *path_matrix,
                                    vg_lite_buffer_t *source,
                                    vg_lite_matrix_t *pattern_matrix,
                                    vg_lite_blend_t blend,
                                    vg_lite_pattern_mode_t pattern_mode,
                                    vg_lite_color_t  pattern_color,
                                    vg_lite_color_t  color,
                                    vg_lite_filter_t filter)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DRAW_PATTERN_API);
    DUMP_API_CALL(vg_lite_draw_pattern, target, path, fill_rule, path_matrix, source, pattern_matrix, blend, pattern_mode, pattern_color, color, filter);

#if gcFEATURE_VG_IM_INPUT
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_float_t x_step[3];
    vg_lite_float_t y_step[3];
    vg_lite_float_t c_step[3];
    vg_lite_uint32_t imageMode = 0;
    vg_lite_uint32_t blend_mode;
    vg_lite_uint32_t filter_mode = 0;
    vg_lite_int32_t stride;
    vg_lite_uint32_t conversion = 0;
    vg_lite_uint32_t tiled_source;
    vg_lite_matrix_t matrix;
    vg_lite_uint32_t pattern_tile = 0;
    vg_lite_uint32_t transparency_mode = 0;
    vg_lite_uint32_t tile_setting = 0;
    vg_lite_uint32_t yuv2rgb = 0;
    vg_lite_uint32_t uv_swiz = 0;
    /* The following code is from "draw path" */
    vg_lite_uint32_t format, quality, tiling, fill;
    vg_lite_uint32_t tessellation_size;

    vg_lite_kernel_allocate_t memory;
    vg_lite_kernel_free_t free_memory;
    vg_lite_uint32_t return_offset = 0;

    vg_lite_point_t point_min = { 0 }, point_max = { 0 }, temp = { 0 };
    vg_lite_int32_t width, height;
    vg_lite_uint8_t ts_is_fullscreen = 0;

    vg_lite_float_t new_matrix[6];
    vg_lite_float_t Scale, Bias;

    vg_lite_uint32_t compress_mode;
    vg_lite_uint32_t src_premultiply_enable = 0;
    vg_lite_uint32_t index_endian = 0;
    vg_lite_uint32_t in_premult = 0;
    vg_lite_uint32_t paintType = 0;
    vg_lite_uint32_t premul_flag = 0;
    vg_lite_uint32_t prediv_flag = 0;
#if gcFEATURE_VG_NEW_FACTOR
    vg_factor_config_t factor_config;
    factor_config.factor_src_alpha = 0x0;
    factor_config.factor_src_color = 0x0;
    factor_config.factor_dst_alpha = 0x3;
    factor_config.factor_dst_color = 0x5;
    factor_config.final_equation_opcode = 0x0;
    factor_config.dstchannelmode = 0x0;
    factor_config.srcchannelmode = 0x0;
    vg_lite_porter_duff_config_t porter_duff_config;
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

    VG_LITE_TRACE_API("vg_lite_draw_pattern %p %p %d %p %p %p %d %d 0x%08X %d\n",
        target, path, fill_rule, path_matrix, source, pattern_matrix, blend, pattern_mode, pattern_color, filter);

#if gcFEATURE_VG_FLEXA
    if (s_context.sync_mode)
    {
        printf("When Flexa is enabled vg_lite_draw_pattern is not support.\n");
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_CHECK_NULL_POINTER2(path, path->path);
    VG_LITE_RETURN_ERROR(feature_check_8x_csaa_support(path->quality));
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
    VG_LITE_RETURN_ERROR(feature_check_stencil_image_mode(source->image_mode));
    VG_LITE_RETURN_ERROR(feature_check_lvgl_recolor_image_mode(source->image_mode));
    VG_LITE_RETURN_ERROR(feature_check_mesh_blt_sw_lvgl_blend(blend, s_context.mesh_mode));
    VG_LITE_RETURN_ERROR(feature_check_new_blend_mode(blend));
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

    if (!path->path_length) {
        return VG_LITE_SUCCESS;
    }

    if (!path_matrix) {
        path_matrix = &identity_mtx;
    }
    if (!pattern_matrix) {
        pattern_matrix = &identity_mtx;
    }

    /* Work on pattern states. */
    matrix = *pattern_matrix;
    if (source->paintType == VG_LITE_PAINT_PATTERN)
    {
        matrix.m[2][0] = 0;
        matrix.m[2][1] = 0;
        matrix.m[2][2] = 1;
        source->image_mode = VG_LITE_NONE_IMAGE_MODE;
    }

    chip_get_source_index_endian_bits(source->format, source->index_endian, &index_endian);

#if gcFEATURE_VG_GAMMA
    save_st_gamma_src_dest(source, target);
#endif

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_GLOBAL));
#endif

    /*blend input into context*/
    s_context.blend_mode = blend;
    in_premult = 0x00000000;

    /* Adjust premultiply setting according to openvg condition */
    src_premultiply_enable = 0x01000100;
    if (s_context.color_transform == 0 && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
    else {
        prediv_flag = 1;
    }
    if ((s_context.blend_mode >= OPENVG_BLEND_SRC && s_context.blend_mode <= OPENVG_BLEND_ADDITIVE) || source->image_mode == VG_LITE_STENCIL_MODE
        || is_lvgl_blend_mode(s_context.blend_mode)
        )
    {
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

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }

    if (is_packed_yuy_format(target->format) && path->quality != VG_LITE_LOW) {
        path->quality = VG_LITE_LOW;
        printf("If target is YUV group , the path qulity should use VG_LITE_LOW.\n");
    }

    transparency_mode = (source->transparency_mode == VG_LITE_IMAGE_TRANSPARENT ? 0x8000:0);
    width = target->width;
    height = target->height;

    if (s_context.scissor_set) {
        width = s_context.scissor[2] - s_context.scissor[0];
        height = s_context.scissor[3] - s_context.scissor[1];
    }
    if (width == 0 || height == 0)
        return VG_LITE_NO_CONTEXT;
    if ((target->width <= width) && (target->height <= height) && (!s_context.scissor_set))
    {
        ts_is_fullscreen = 1;
        point_min.x = 0;
        point_min.y = 0;
        point_max.x = target->width;
        point_max.y = target->height;
    }

    conversion = feature_a124_a8l8_l8_conversion(target->format, source->format);

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, &matrix))
        return VG_LITE_SUCCESS;

    /* Compute step values. */
    calculate_step_value(filter, &inverse_matrix, source->width, source->height, x_step, y_step, c_step);

    /* Determine image mode (NORMAL, NONE , MULTIPLY or STENCIL) depending on the color. */
    switch (source->image_mode) {
        case VG_LITE_NONE_IMAGE_MODE:
            imageMode = 0x0;
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

    tiled_source = (source->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0 ;
    compress_mode = (vg_lite_uint32_t)source->compress_mode << 25;

    if (pattern_mode == VG_LITE_PATTERN_COLOR)
    {
        vg_lite_uint8_t a,r,g,b;
        pattern_tile = 0;
        a = pattern_color >> 24;
        r = pattern_color >> 16;
        g = pattern_color >> 8;
        b = pattern_color;
        pattern_color = (a << 24) | (b << 16) | (g << 8) | r;
    }
    else if (pattern_mode == VG_LITE_PATTERN_PAD)
    {
        pattern_tile = 0x1000;
    }
#if gcFEATURE_VG_IM_REPEAT_REFLECT
    else if (pattern_mode == VG_LITE_PATTERN_REPEAT)
    {
        pattern_tile = 0x2000;
    }
    else if (pattern_mode == VG_LITE_PATTERN_REFLECT)
    {
        pattern_tile = 0x3000;
    }
#endif
    else
    {
        return VG_LITE_INVALID_ARGUMENT;
    }

    if (source->paintType == VG_LITE_PAINT_PATTERN)
    {
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A04, (vg_lite_void *) &c_step[0]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A05, (vg_lite_void *) &c_step[1]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A06, (vg_lite_void *) &x_step[0]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A07, (vg_lite_void *) &x_step[1]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A08, (vg_lite_void *) &y_step[0]));
        VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A09, (vg_lite_void *) &y_step[1]));
    }
    
    /* Setup the command buffer. */
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
    blend_mode = convert_blend(blend);

#if gcFEATURE_VG_NEW_FACTOR
    porter_duff_config = s_context.porter_duff_config;
    config_factor_parameter(blend, porter_duff_config, &factor_config);

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF6, factor_config.srcchannelmode | (factor_config.dstchannelmode << 8)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF8, factor_config.factor_src_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF9, factor_config.factor_src_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFA, factor_config.factor_dst_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFB, factor_config.factor_dst_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF7, factor_config.final_equation_opcode));
#endif

    if (source->paintType == VG_LITE_PAINT_PATTERN)
    {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A24, convert_source_format(source->format) | filter_mode | pattern_tile | uv_swiz | yuv2rgb | conversion | compress_mode | src_premultiply_enable | index_endian));
        /* 24bit format stride configured to 4bpp. */
        if (source->format >= VG_LITE_RGB888 && source->format <= VG_LITE_RGBA5658) {
            stride = source->stride / 3 * 4;
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2A, stride | tiled_source));
        }
        else {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2A, source->stride | tiled_source));
        }
        if (source->yuv.uv_planar) {
            /* Program u plane address if necessary. */
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A50, source->yuv.uv_planar));
        }
        if (source->yuv.v_planar) {
            /* Program v plane address if necessary. */
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A52, source->yuv.v_planar));
        }

        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A26, pattern_color));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A28, source->address));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2C, 0));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2E, source->width | (source->height << 16)));
    }
    else
    {
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

        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A27, pattern_color));

#if !gcFEATURE_VG_LVGL_SUPPORT
        if (lvgl_sw_blend) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source->lvgl_buffer->address));
        }
        else
#endif
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source->address));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2D, 0));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2F, source->width | (source->height << 16)));
    }

    /* Work on path states. */
    matrix = *path_matrix;

    if (ts_is_fullscreen == 0) {
        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[1], &matrix);
        point_min = point_max = temp;
    
        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[1], &matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;
    
        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[3], &matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;
    
        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[3], &matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;
    
        point_min.x = MAX(point_min.x, 0);
        point_min.y = MAX(point_min.y, 0);
        point_max.x = MIN(point_max.x, target->width);
        point_max.y = MIN(point_max.y, target->height);

        if (s_context.scissor_set) {
            point_min.x = MAX(point_min.x, s_context.scissor[0]);
            point_min.y = MAX(point_min.y, s_context.scissor[1]);
            point_max.x = MIN(point_max.x, s_context.scissor[2]);
            point_max.y = MIN(point_max.y, s_context.scissor[3]);
        }
    }

    width = point_max.x - point_min.x;
    height = point_max.y - point_min.y;
    Scale = 1.0f;
    Bias = 0.0f;
    new_matrix[0] = matrix.m[0][0] * Scale;
    new_matrix[1] = matrix.m[0][1] * Scale;
    new_matrix[2] = (matrix.m[0][0] + matrix.m[0][1]) * Bias + matrix.m[0][2];
    new_matrix[3] = matrix.m[1][0] * Scale;
    new_matrix[4] = matrix.m[1][1] * Scale;
    new_matrix[5] = (matrix.m[1][0] + matrix.m[1][1]) * Bias + matrix.m[1][2];

    /* Convert states into hardware values. */
    format = convert_path_format(path->format);
    quality = convert_path_quality(path->quality);
    tiling = (s_context.capabilities.cap.tiled == 2) ? 0x2000000 : 0;
    fill = (fill_rule == VG_LITE_FILL_EVEN_ODD) ? 0x10 : 0;
    tessellation_size = s_context.tessbuf.tessbuf_size;

    VG_LITE_RETURN_ERROR(chip_set_tes_tile(target, &tile_setting));

    if (source->paintType == VG_LITE_PAINT_PATTERN) {
        paintType = 1 << 24 | 1 << 25;
    }

    /* Setup the command buffer. */
    /* Program color register. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, in_premult | paintType |s_context.capabilities.cap.tiled | imageMode | blend_mode | transparency_mode | tile_setting | s_context.enable_mask | s_context.scissor_enable | s_context.color_transform | s_context.matrix_enable | 0x2));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000000 | format | quality | tiling | fill));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3B, 0x3F800000));      /* Path tessellation SCALE. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3C, 0x00000000));      /* Path tessellation BIAS.  */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color));
    /* Program matrix. */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A40, (vg_lite_void *) &new_matrix[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A41, (vg_lite_void *) &new_matrix[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A42, (vg_lite_void *) &new_matrix[2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A43, (vg_lite_void *) &new_matrix[3]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A44, (vg_lite_void *) &new_matrix[4]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A45, (vg_lite_void *) &new_matrix[5]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0ACD, (vg_lite_void *) &matrix.m[0][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0ACE, (vg_lite_void *) &matrix.m[1][2]));

    if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
    {
        if (path->path_changed != 0) {
            if (path->uploaded.handle != NULL) {
                free_memory.memory_handle = path->uploaded.handle;
                vg_lite_kernel(VG_LITE_FREE, &free_memory);
                path->uploaded.address = 0;
                path->uploaded.memory = NULL;
                path->uploaded.handle = NULL;
            }
            /* Allocate memory for the path data. */
            memory.bytes = 16 + VG_LITE_ALIGN(path->path_length, 8);
            return_offset = (8 + VG_LITE_ALIGN(path->path_length, 8)) / 4;
            memory.contiguous = 1;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &memory));
            ((uint64_t *) memory.memory)[(path->path_length + 7) / 8] = 0;
            ((vg_lite_uint32_t *) memory.memory)[0] = VG_LITE_DATA((path->path_length + 7) / 8);
            ((vg_lite_uint32_t *) memory.memory)[1] = 0;
            memcpy((vg_lite_uint8_t *) memory.memory + 8, path->path, path->path_length);
            ((vg_lite_uint32_t *) memory.memory)[return_offset] = VG_LITE_RETURN();
            ((vg_lite_uint32_t *) memory.memory)[return_offset + 1] = 0;

            path->uploaded.handle = memory.memory_handle;
            path->uploaded.memory = memory.memory;
            path->uploaded.address = memory.memory_gpu;
            path->uploaded.bytes  = memory.bytes;
            path->path_changed = 0;
        }
    }


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
    if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
        vglitemDUMP_BUFFER("path", (size_t)path->uploaded.address, (vg_lite_uint8_t *)(path->uploaded.memory), 0, path->uploaded.bytes);
    }
    vglitemDUMP("@[memory 0x%08X 0x%08X]", s_context.tessbuf.physical_addr, s_context.tessbuf.tessbuf_size);
#endif

    if (width + point_min.x > target->width) {
        width = target->width - point_min.x;
    }

#if gcFEATURE_COMBO_VG_SPLIT_PATH_SUPPORT_BY_SW
  if (s_context.split_path)
  {
    vg_lite_int32_t y = 0;
    vg_lite_uint32_t par_height = 0;
    vg_lite_int32_t next_boundary = 0;
#if (!gcFEATURE_VG_PARALLEL_PATHS_DISABLE)
    vg_lite_uint32_t parallel_workpaths1 = 2;
    vg_lite_uint32_t parallel_workpaths2 = 2;
#endif

    s_context.tessbuf.tess_w_h = width | (height << 16);
    s_context.tessbuf.tess_x_y = point_min.x | (point_min.y << 16);

#if !gcFEATURE_VG_PARALLEL_PATHS_DISABLE
    if (height <= 128)
        parallel_workpaths1 = 4;
    else
        parallel_workpaths1 = height * 128 / 4096 - 1;

    if (parallel_workpaths1 > parallel_workpaths2)
        parallel_workpaths1 = parallel_workpaths2;
#endif 
    for (y = point_min.y; y < point_max.y; y += par_height) {
#if (gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE && gcFEATURE_VG_512_HALF_SPLIT_DISABLE && !gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE)
        next_boundary = (y + 512) & 0xfffffe00;
#elif (gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE && !gcFEATURE_VG_512_HALF_SPLIT_DISABLE)
        if (height > 512)
            next_boundary = (y + 256);
        else
            next_boundary = (y + (height + 1) / 2);
#elif (!gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE)
        next_boundary = (y + 32) & 0xffffffe0;
#else   
        next_boundary = (y + 16) & 0xfffffff0;
#endif
        par_height = ((next_boundary < point_max.y) ? next_boundary - y : (point_max.y - y));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, in_premult | paintType | s_context.capabilities.cap.tiled | imageMode | blend_mode | transparency_mode | tile_setting | s_context.enable_mask | s_context.scissor_enable | s_context.color_transform | s_context.matrix_enable | 0x2));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000000 | format | quality | tiling | fill));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color));;
        VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
            (vg_lite_uint32_t)(point_min.x | (y << 16)),
            (vg_lite_uint32_t)(width | (par_height << 16)), 0));

        if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
            VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
        }
        else {
            if (path->path_type == VG_LITE_DRAW_FILL_PATH || path->path_type == VG_LITE_DRAW_ZERO)
                VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
            if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {
                format = convert_path_format(VG_LITE_FP32);
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0x0));
                VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, path->stroke_color));
                VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
            }
        }
#if !gcFEATURE_VG_PARALLEL_PATHS_DISABLE
        s_context.path_counter++;
        if (parallel_workpaths1 == s_context.path_counter) {
            VG_LITE_RETURN_ERROR(push_stall(&s_context, 7));
            s_context.path_counter = 0;
        }
#elif !gcFEATURE_VG_512_HALF_SPLIT_DISABLE && gcFEATURE_VG_PARALLEL_PATHS_DISABLE && gcFEATURE_VG_SPLIT_PATH_DISABLE
        VG_LITE_RETURN_ERROR(push_stall(&s_context, 7));
#endif          
    }
  }
  else
#endif /* gcFEATURE_COMBO_VG_SPLIT_PATH_SUPPORT_BY_SW */
#if gcFEATURE_VG_HW_STROKE
  {
    /* Tessellate path. */
    s_context.tessbuf.tess_w_h = width | (height << 16);
    s_context.tessbuf.tess_x_y = point_min.x | (point_min.y << 16);
    VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
        s_context.tessbuf.tess_x_y, s_context.tessbuf.tess_w_h, 0));

    vg_lite_uint16_t  no_use_hw_stroke_flag = 0;

    if (path->stroke)
        no_use_hw_stroke_flag = path->stroke->join_style == VG_LITE_JOIN_MITER || path->stroke->cap_style == VG_LITE_CAP_SQUARE || path->stroke->dash_phase != 0;

    if (!no_use_hw_stroke_flag)
    {
        if (path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH || path->path_type == VG_LITE_DRAW_STROKE_PATH)
        {
            VG_LITE_RETURN_ERROR(stroke_config(path, path_matrix,  point_min, width, height));
        }
        push_path_advance(path, quality, tiling, in_premult, blend_mode, 1);
    }
    else
    {
        /* Tessellate path. */
        s_context.tessbuf.tess_w_h = width | (height << 16);
        s_context.tessbuf.tess_x_y = point_min.x | (point_min.y << 16);
        VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
            s_context.tessbuf.tess_x_y, s_context.tessbuf.tess_w_h, 0));
        VG_LITE_RETURN_ERROR(push_path_base(path, quality, tiling, 0, 0, 1));
    }
  }
#else
  {
    /* Tessellate path. */
    s_context.tessbuf.tess_w_h = width | (height << 16);
    s_context.tessbuf.tess_x_y = point_min.x | (point_min.y << 16);
    VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
        s_context.tessbuf.tess_x_y, s_context.tessbuf.tess_w_h, 0));
    VG_LITE_RETURN_ERROR(push_path_base(path, quality, tiling, 0, 0, 1));
  }
#endif

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_NORMAL));
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

/* GC555 vg_lite_draw_linear_grad API implementation
 */
vg_lite_error_t vg_lite_draw_linear_grad(vg_lite_buffer_t* target,
                                        vg_lite_path_t* path,
                                        vg_lite_fill_t fill_rule,
                                        vg_lite_matrix_t* path_matrix,
                                        vg_lite_ext_linear_gradient_t* grad,
                                        vg_lite_color_t paint_color,
                                        vg_lite_blend_t blend,
                                        vg_lite_filter_t filter)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DRAW_LINEAR_GRAD_API);
    DUMP_API_CALL(vg_lite_draw_linear_grad, target, path, fill_rule, path_matrix, grad, paint_color, blend, filter);

#if gcFEATURE_VG_LINEAR_GRADIENT_EXT && gcFEATURE_VG_IM_INPUT
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t image_mode = 0;
    vg_lite_uint32_t blend_mode;
    vg_lite_uint32_t filter_mode = 0;
    vg_lite_uint32_t conversion = 0;
    vg_lite_uint32_t tiled_source;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_float_t x_step[3];
    vg_lite_float_t y_step[3];
    vg_lite_float_t c_step[3];
    vg_lite_buffer_t* source = &grad->image;
    vg_lite_matrix_t* matrix = &grad->matrix;
    vg_lite_uint32_t linear_tile = 0;
    vg_lite_uint32_t transparency_mode = 0;
    vg_lite_uint32_t yuv2rgb = 0;
    vg_lite_uint32_t uv_swiz = 0;
    vg_lite_uint32_t in_premult = 0;
    vg_lite_uint32_t src_premultiply_enable = 0;
    vg_lite_uint32_t premul_flag = 0;
    vg_lite_uint32_t prediv_flag = 0;
    vg_lite_pointer data;
    vg_lite_uint32_t tile_setting = 0;

    /* The following code is from "draw path" */
    vg_lite_uint32_t format, quality, tiling, fill;
    vg_lite_uint32_t tessellation_size;

    vg_lite_kernel_allocate_t memory;
    vg_lite_kernel_free_t free_memory;
    vg_lite_uint32_t return_offset = 0;

    vg_lite_point_t point_min = { 0 }, point_max = { 0 }, temp = { 0 };
    vg_lite_int32_t width, height;
    vg_lite_uint8_t ts_is_fullscreen = 0;
    vg_lite_float_t new_matrix[6];
    vg_lite_float_t Scale, Bias;

    vg_lite_float_t dx, dy, dxdx_dydy;
    vg_lite_float_t lg_step_x_lin, lg_step_y_lin, lg_constant_lin;
#if gcFEATURE_VG_NEW_FACTOR
    vg_factor_config_t factor_config;
    factor_config.factor_src_alpha = 0x0;
    factor_config.factor_src_color = 0x0;
    factor_config.factor_dst_alpha = 0x3;
    factor_config.factor_dst_color = 0x5;
    factor_config.final_equation_opcode = 0x0;
    factor_config.dstchannelmode = 0x0;
    factor_config.srcchannelmode = 0x0;
    vg_lite_porter_duff_config_t porter_duff_config;
#endif
#if DUMP_CAPTURE
    vg_lite_float_t ratio = 1;
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

    VG_LITE_TRACE_API("vg_lite_draw_linear_grad %p %p %d %p %p 0x%08X %d %d\n",
        target, path, fill_rule, path_matrix, grad, paint_color, blend, filter);

#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_CHECK_NULL_POINTER2(path, path->path);
    VG_LITE_RETURN_ERROR(feature_check_8x_csaa_support(path->quality));
    VG_LITE_RETURN_ERROR(feature_check_lvgl_blend_mode(blend));
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(target->format));

    VG_LITE_RETURN_ERROR(feature_check_stencil_image_mode(source->image_mode));
    VG_LITE_RETURN_ERROR(feature_check_new_blend_mode(blend));
    VG_LITE_RETURN_ERROR(feature_check_grad_spread_mode(grad->spread_mode));
#endif /* gcFEATURE_VG_ERROR_CHECK */

    if (!path->path_length) {
        return VG_LITE_SUCCESS;
    }

    if (!path_matrix) {
        path_matrix = &identity_mtx;
    }

#if gcFEATURE_VG_GAMMA
    set_gamma_dest_only(target, VGL_TRUE);
#endif

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_GLOBAL));
#endif

    /*blend input into context*/
    s_context.blend_mode = blend;

    src_premultiply_enable = 0x01000100;
    if (s_context.color_transform == 0 && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
    else {
        prediv_flag = 1;
    }
    if ((s_context.blend_mode >= OPENVG_BLEND_SRC && s_context.blend_mode <= OPENVG_BLEND_ADDITIVE) || source->image_mode == VG_LITE_STENCIL_MODE
        || is_lvgl_blend_mode(s_context.blend_mode)
        )
    {
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
        in_premult = 0x00000000;
    }
    if (source->premultiplied == target->premultiplied && premul_flag == 0) {
        target->apply_premult = 1;
    }
    else {
        target->apply_premult = 0;
    }

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        if (error == VG_LITE_NO_CONTEXT) {
            /* If scissoring is enabled and no valid scissoring rectangles
               are present, no drawing occurs */
            return VG_LITE_SUCCESS;
        }
        else {
            return error;
        }
    }

    transparency_mode = (source->transparency_mode == VG_LITE_IMAGE_TRANSPARENT ? 0x8000:0);

    width = target->width;
    height = target->height;
    if (s_context.scissor_set) {
        width = s_context.scissor[2] - s_context.scissor[0];
        height = s_context.scissor[3] - s_context.scissor[1];
    }
    if (width == 0 || height == 0)
        return VG_LITE_NO_CONTEXT;
    if ((target->width <= width) && (target->height <= height) && (!s_context.scissor_set))
    {
        ts_is_fullscreen = 1;
        point_min.x = 0;
        point_min.y = 0;
        point_max.x = target->width;
        point_max.y = target->height;
    }

    conversion = feature_a124_a8l8_l8_conversion(target->format, source->format);

#if gcFEATURE_VG_NEW_FACTOR
    porter_duff_config = s_context.porter_duff_config;
    config_factor_parameter(blend, porter_duff_config, &factor_config);

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF6, factor_config.srcchannelmode | (factor_config.dstchannelmode << 8)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF8, factor_config.factor_src_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF9, factor_config.factor_src_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFA, factor_config.factor_dst_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFB, factor_config.factor_dst_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF7, factor_config.final_equation_opcode));
#endif

    /* Determine image mode (NORMAL, NONE , MULTIPLY or STENCIL) depending on the color. */
    switch (source->image_mode) {
        case VG_LITE_NONE_IMAGE_MODE:
            image_mode = 0x0;
            break;

        case VG_LITE_MULTIPLY_IMAGE_MODE:
            return VG_LITE_INVALID_ARGUMENT;

        case VG_LITE_NORMAL_IMAGE_MODE:
        case VG_LITE_ZERO:
            image_mode = 0x00001000;
            break;

        case VG_LITE_STENCIL_MODE:
            image_mode = 0x00003000;
            break;

        case VG_LITE_RECOLOR_MODE:
            image_mode = 0x00006000;
            break;
    }
    tiled_source = (source->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0 ;

    switch (grad->spread_mode) {
        case VG_LITE_GRADIENT_SPREAD_FILL:
            linear_tile = 0x0;
            break;

        case VG_LITE_GRADIENT_SPREAD_PAD:
            linear_tile = 0x1000;
            break;

        case VG_LITE_GRADIENT_SPREAD_REPEAT:
            linear_tile = 0x2000;
            break;

        case VG_LITE_GRADIENT_SPREAD_REFLECT:
            linear_tile = 0x3000;
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

    if (grad->spread_mode == VG_LITE_GRADIENT_SPREAD_FILL)
    {
        vg_lite_uint8_t a,r,g,b;
        a = paint_color >> 24;
        r = paint_color >> 16;
        g = paint_color >> 8;
        b = paint_color;
        paint_color = (a << 24) | (b << 16) | (g << 8) | r;
    }

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix))
        return VG_LITE_SUCCESS;

    /* compute linear gradient paremeters */

    dx = grad->linear_grad.X1 - grad->linear_grad.X0;
    dy = grad->linear_grad.Y1 - grad->linear_grad.Y0;
#if gcFEATURE_VG_MATH_PRECISION_FIX_DISABLE
    //dxdx_dydy = (vg_lite_float_t)((dx * dx + dy * dy) / sqrt((dx + 1) * (dx + 1) + (dy + 1) * (dy + 1)));
    dxdx_dydy = (vg_lite_float_t)(sqrt((vg_lite_double_t)dx * (vg_lite_double_t)dx + (vg_lite_double_t)dy * (vg_lite_double_t)dy));
#else
    dxdx_dydy = dx * dx + dy * dy;
#endif

    /*
    **      dx (T(x) - x0) + dy (T(y) - y0)
    **  g = -------------------------------
    **                dx^2 + dy^2
    **
    **  where
    **
    **      dx := x1 - x0
    **      dy := y1 - y0
    **      T(x) := (x + 0.5) m00 + (y + 0.5) m01 + m02
    **            = x m00 + y m01 + 0.5 (m00 + m01) + m02
    **      T(y) := (x + 0.5) m10 + (y + 0.5) m11 + m12
    **            = x m10 + y m11 + 0.5 (m10 + m11) + m12.
    **
    **  We can factor the top line into:
    **
    **      = dx (x m00 + y m01 + 0.5 (m00 + m01) + m02 - x0)
    **      + dy (x m10 + y m11 + 0.5 (m10 + m11) + m12 - y0)
    **
    **      = x (dx m00 + dy m10)
    **      + y (dx m01 + dy m11)
    **      + dx (0.5 (m00 + m01) + m02 - x0)
    **      + dy (0.5 (m10 + m11) + m12 - y0).
    */

    lg_step_x_lin
        = (dx * MAT(&inverse_matrix, 0, 0) + dy * MAT(&inverse_matrix, 1, 0))
        / dxdx_dydy;

    lg_step_y_lin
        = (dx * MAT(&inverse_matrix, 0, 1) + dy * MAT(&inverse_matrix, 1, 1))
        / dxdx_dydy;

    lg_constant_lin =
        (
            (
                0.5f * ( MAT(&inverse_matrix, 0, 0) + MAT(&inverse_matrix, 0, 1) )
                + MAT(&inverse_matrix, 0, 2) - grad->linear_grad.X0
            ) * dx

            +

            (
                0.5f * ( MAT(&inverse_matrix, 1, 0) + MAT(&inverse_matrix, 1, 1) )
                + MAT(&inverse_matrix, 1, 2) - grad->linear_grad.Y0
            ) * dy
        )
        / dxdx_dydy;

#if gcFEATURE_VG_MATH_PRECISION_FIX_DISABLE
    // Preprocess for REPEAT and REFLECT mode to avoid overflow (max value of S16.6 is 32768).
    // Need driver add condition judgment for REPEAT and REFLECT mode here:
    if ((grad->spread_mode == VG_LITE_GRADIENT_SPREAD_REPEAT) || (grad->spread_mode == VG_LITE_GRADIENT_SPREAD_REFLECT))
        lg_constant_lin -= (((vg_lite_int32_t)lg_constant_lin) / ((vg_lite_int32_t)(dx * 2))) * dx * 2;
#endif

    /* Setup the command buffer. */

    /* linear gradient parameters*/
    data = &lg_constant_lin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A04,*(vg_lite_uint32_t*) data));
    data = &lg_step_x_lin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A06,*(vg_lite_uint32_t*) data));
    data = &lg_step_y_lin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A08,*(vg_lite_uint32_t*) data));

    /* Compute step values. */
    calculate_step_value(filter, &inverse_matrix, source->width, source->height, x_step, y_step, c_step);

    /* Setup the command buffer. */
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

    if (source->yuv.uv_planar) {
        /* Program u plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A51, source->yuv.uv_planar));
    }
    if (source->yuv.v_planar) {
        /* Program v plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source->yuv.v_planar));
    }

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A24, convert_source_format(source->format) |
                                                        filter_mode | uv_swiz | yuv2rgb | linear_tile | conversion | src_premultiply_enable));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A26, paint_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A28, source->address));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2A, tiled_source));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2C, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2E, source->width | (source->height << 16)));

    /* Work on path states. */
    matrix = path_matrix;

    if (ts_is_fullscreen == 0) {
        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[1], matrix);
        point_min = point_max = temp;

        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[1], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        point_min.x = MAX(point_min.x, 0);
        point_min.y = MAX(point_min.y, 0);
        point_max.x = MIN(point_max.x, target->width);
        point_max.y = MIN(point_max.y, target->height);

        if (s_context.scissor_set) {
            point_min.x = MAX(point_min.x, s_context.scissor[0]);
            point_min.y = MAX(point_min.y, s_context.scissor[1]);
            point_max.x = MIN(point_max.x, s_context.scissor[2]);
            point_max.y = MIN(point_max.y, s_context.scissor[3]);
        }
    }

    width = point_max.x - point_min.x;
    height = point_max.y - point_min.y;
    Scale = 1.0f;
    Bias = 0.0f;
    new_matrix[0] = matrix->m[0][0] * Scale;
    new_matrix[1] = matrix->m[0][1] * Scale;
    new_matrix[2] = (matrix->m[0][0] + matrix->m[0][1]) * Bias + matrix->m[0][2];
    new_matrix[3] = matrix->m[1][0] * Scale;
    new_matrix[4] = matrix->m[1][1] * Scale;
    new_matrix[5] = (matrix->m[1][0] + matrix->m[1][1]) * Bias + matrix->m[1][2];

    /* Convert states into hardware values. */
    blend_mode = convert_blend(blend);
    format = convert_path_format(path->format);
    quality = convert_path_quality(path->quality);
    tiling = (s_context.capabilities.cap.tiled == 2) ? 0x2000000 : 0;
    fill = (fill_rule == VG_LITE_FILL_EVEN_ODD) ? 0x10 : 0;
    tessellation_size = s_context.tessbuf.tessbuf_size;

    VG_LITE_RETURN_ERROR(chip_set_tes_tile(target, &tile_setting));

    /* Setup the command buffer. */
    /* Program color register. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x01000002 | s_context.capabilities.cap.tiled | in_premult | image_mode | blend_mode | tile_setting | transparency_mode | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | s_context.scissor_enable));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000400 | format | quality | tiling | fill));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3B, 0x3F800000));      /* Path tessellation SCALE. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3C, 0x00000000));      /* Path tessellation BIAS.  */
    /* Program matrix. */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A40, (vg_lite_void *) &new_matrix[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A41, (vg_lite_void *) &new_matrix[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A42, (vg_lite_void *) &new_matrix[2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A43, (vg_lite_void *) &new_matrix[3]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A44, (vg_lite_void *) &new_matrix[4]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A45, (vg_lite_void *) &new_matrix[5]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0ACD, (vg_lite_void *) &matrix->m[0][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0ACE, (vg_lite_void *) &matrix->m[1][2]));

    if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
    {
        if (path->path_changed != 0) {
            if (path->uploaded.handle != NULL) {
                free_memory.memory_handle = path->uploaded.handle;
                vg_lite_kernel(VG_LITE_FREE, &free_memory);
                path->uploaded.address = 0;
                path->uploaded.memory = NULL;
                path->uploaded.handle = NULL;
            }
            /* Allocate memory for the path data. */
            memory.bytes = 16 + VG_LITE_ALIGN(path->path_length, 8);
            return_offset = (8 + VG_LITE_ALIGN(path->path_length, 8)) / 4;
            memory.contiguous = 1;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &memory));
            ((uint64_t *) memory.memory)[(path->path_length + 7) / 8] = 0;
            ((vg_lite_uint32_t *) memory.memory)[0] = VG_LITE_DATA((path->path_length + 7) / 8);
            ((vg_lite_uint32_t *) memory.memory)[1] = 0;
            memcpy((vg_lite_uint8_t *) memory.memory + 8, path->path, path->path_length);
            ((vg_lite_uint32_t *) memory.memory)[return_offset] = VG_LITE_RETURN();
            ((vg_lite_uint32_t *) memory.memory)[return_offset + 1] = 0;

            path->uploaded.handle = memory.memory_handle;
            path->uploaded.memory = memory.memory;
            path->uploaded.address = memory.memory_gpu;
            path->uploaded.bytes  = memory.bytes;
            path->path_changed = 0;
        }
    }

#if DUMP_CAPTURE
    if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
        vglitemDUMP_BUFFER("path", (size_t)path->uploaded.address, (vg_lite_uint8_t *)(path->uploaded.memory), 0, path->uploaded.bytes);
    }
    vglitemDUMP("@[memory 0x%08X 0x%08X]", s_context.tessbuf.physical_addr, s_context.tessbuf.tessbuf_size);
#endif

    if (width + point_min.x > target->width) {
        width = target->width - point_min.x;
    }

#if gcFEATURE_COMBO_VG_SPLIT_PATH_SUPPORT_BY_SW
    if (s_context.split_path)
    {
#if gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE
        vg_lite_uint32_t parallel_workpaths1 = 2;
        vg_lite_uint32_t parallel_workpaths2 = 2;
#endif

        vg_lite_int32_t y;
        vg_lite_int32_t temp_height = 0;
        height = s_context.tessbuf.tess_w_h >> 16;
        if (path->path_type == VG_LITE_DRAW_FILL_PATH || path->path_type == VG_LITE_DRAW_ZERO) {
#if gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE
            if (height <= 128)
                parallel_workpaths1 = 4;
            else 
                parallel_workpaths1 = height * 128 / 4096 - 1;

            if (parallel_workpaths1 > parallel_workpaths2)
                parallel_workpaths1 = parallel_workpaths2;
#endif
            for (y = point_min.y; y < point_max.y; y += height) {
                vg_lite_uint32_t tile_wh;

                if (y + height > target->height) {
                    temp_height = target->height - y;
                    tile_wh = (vg_lite_uint32_t)(width | (temp_height << 16));
                } else {
                    tile_wh = (vg_lite_uint32_t)(width | (height << 16));
                }
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(point_min.x | (y << 16)), tile_wh, 0));

                if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
                } else {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
                }
#if gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE
                s_context.path_counter ++;
                if (parallel_workpaths1 == s_context.path_counter) {
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0E02, 0x10 | (0x7 << 8)));
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0F00, 0x10 | (0x7 << 8)));
                    s_context.path_counter = 0;
                }
#endif
            }
        }

        if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {
            for (y = point_min.y; y < point_max.y; y += height) {
                vg_lite_uint32_t tile_wh;

                if (y + height > target->height) {
                    temp_height = target->height - y;
                    tile_wh = (vg_lite_uint32_t)(width | (temp_height << 16));
                } else {
                    tile_wh = (vg_lite_uint32_t)(width | (height << 16));
                }
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(point_min.x | (y << 16)), tile_wh, 0));

                if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
                } else {
                    format = convert_path_format(VG_LITE_FP32);
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0x0));
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, path->stroke_color));
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
                }
#if gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE
                s_context.path_counter ++;
                if (parallel_workpaths1 == s_context.path_counter) {
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0E02, 0x10 | (0x7 << 8)));
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0F00, 0x10 | (0x7 << 8)));
                    s_context.path_counter = 0;
                }
#endif
            }
        }
    }
    else
#else
#if gcFEATURE_VG_HW_STROKE
    {
        /* Tessellate path. */
        s_context.tessbuf.tess_w_h = width | (height << 16);
        s_context.tessbuf.tess_x_y = point_min.x | (point_min.y << 16);
        VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
            s_context.tessbuf.tess_x_y, s_context.tessbuf.tess_w_h, 0));

        vg_lite_uint16_t  no_use_hw_stroke_flag = 0;

        if (path->stroke)
            no_use_hw_stroke_flag = path->stroke->join_style == VG_LITE_JOIN_MITER || path->stroke->cap_style == VG_LITE_CAP_SQUARE  || path->stroke->dash_phase != 0;
        
        if (!no_use_hw_stroke_flag)
        {
            if (path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH || path->path_type == VG_LITE_DRAW_STROKE_PATH)
            {
                VG_LITE_RETURN_ERROR(stroke_config(path, path_matrix, point_min, width, height));
            }
            push_path_advance(path, quality, tiling, in_premult, blend_mode, 2);
        }
        else
        {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C06, 0x00000000));
            VG_LITE_RETURN_ERROR(push_path_base(path, quality, tiling, in_premult, blend_mode, 2));            
        }
    }
#else
    {
        /* Tessellate path. */
        s_context.tessbuf.tess_w_h = width | (height << 16);
        s_context.tessbuf.tess_x_y = point_min.x | (point_min.y << 16);
        VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
            s_context.tessbuf.tess_x_y, s_context.tessbuf.tess_w_h, 0));
        VG_LITE_RETURN_ERROR(push_path_base(path, quality, tiling, in_premult, blend_mode, 2));
    }
#endif
#endif

    /* Finialize command buffer. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_NORMAL));
#endif

#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((target->format >= VG_LITE_ABGR8565) && (target->format <= VG_LITE_RGBA5658))
    {
        if (target->sw24bit_planar_buffer)
            target = target->sw24bit_planar_buffer;
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

/* GC555 vg_lite_draw_radial_grad API implementation
 */
vg_lite_error_t vg_lite_draw_radial_grad(vg_lite_buffer_t* target,
                                        vg_lite_path_t* path,
                                        vg_lite_fill_t fill_rule,
                                        vg_lite_matrix_t* path_matrix,
                                        vg_lite_radial_gradient_t* grad,
                                        vg_lite_color_t paint_color,
                                        vg_lite_blend_t blend,
                                        vg_lite_filter_t filter)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DRAW_RADIAL_GRAD_API);
    DUMP_API_CALL(vg_lite_draw_radial_grad, target, path, fill_rule, path_matrix, grad, paint_color, blend, filter);

#if gcFEATURE_COMBO_VG_SUPPORT_RADIAL_GRADIENT
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_uint32_t imageMode = 0;
    vg_lite_uint32_t blend_mode;
    vg_lite_uint32_t filter_mode = 0;
    vg_lite_uint32_t conversion = 0;
    vg_lite_uint32_t tiled_source;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_float_t x_step[3];
    vg_lite_float_t y_step[3];
    vg_lite_float_t c_step[3];
    vg_lite_buffer_t* source = &grad->image;
    vg_lite_matrix_t* matrix = &grad->matrix;
    vg_lite_uint32_t rad_tile = 0;
    vg_lite_uint32_t transparency_mode = 0;
    vg_lite_uint32_t yuv2rgb = 0;
    vg_lite_uint32_t uv_swiz = 0;
    vg_lite_pointer data;
    vg_lite_uint32_t compress_mode;
    vg_lite_uint32_t in_premult = 0;
    vg_lite_uint32_t src_premultiply_enable = 0;
    vg_lite_uint32_t premul_flag = 0;
    vg_lite_uint32_t prediv_flag = 0;
    vg_lite_uint32_t tile_setting = 0;

#if gcFEATURE_VG_NEW_FACTOR
    vg_factor_config_t factor_config;
    factor_config.factor_src_alpha = 0x0;
    factor_config.factor_src_color = 0x0;
    factor_config.factor_dst_alpha = 0x3;
    factor_config.factor_dst_color = 0x5;
    factor_config.final_equation_opcode = 0x0;
    factor_config.dstchannelmode = 0x0;
    factor_config.srcchannelmode = 0x0;
    vg_lite_porter_duff_config_t porter_duff_config;
#endif

    /* The following code is from "draw path" */
    vg_lite_uint32_t format, quality, tiling, fill;
    vg_lite_uint32_t tessellation_size;

    vg_lite_kernel_allocate_t memory;
    vg_lite_kernel_free_t free_memory;
    vg_lite_uint32_t return_offset = 0;

    vg_lite_point_t point_min = { 0 }, point_max = { 0 }, temp = { 0 };
    vg_lite_int32_t width, height;
    vg_lite_uint8_t ts_is_fullscreen = 0;
    vg_lite_float_t new_matrix[6];
    vg_lite_float_t Scale, Bias;

    vg_lite_float_t radius = grad->radial_grad.r;

    vg_lite_float_t centerX, centerY;
    vg_lite_float_t focalX, focalY;
    vg_lite_float_t fx, fy;
    vg_lite_float_t fxfy_2;
    vg_lite_float_t radius2;
    vg_lite_float_t r2_fx2, r2_fy2;
    vg_lite_float_t r2_fx2_2, r2_fy2_2;
    vg_lite_float_t r2_fx2_fy2;
    vg_lite_float_t r2_fx2_fy2sq;
    vg_lite_float_t cx, cy;

    vg_lite_float_t rgConstantLin, rgStepXLin, rgStepYLin;
    vg_lite_float_t rgConstantRad, rgStepXRad, rgStepYRad;
    vg_lite_float_t rgStepXXRad, rgStepYYRad, rgStepXYRad;

#if DUMP_CAPTURE
    vg_lite_float_t ratio = 1;
#endif
#if !gcFEATURE_VG_PARALLEL_PATHS_DISABLE
    vg_lite_uint32_t parallel_workpaths1 = 2;
    vg_lite_uint32_t parallel_workpaths2 = 2;
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

    VG_LITE_TRACE_API("vg_lite_draw_radial_grad %p %p %d %p %p 0x%08X %d %d\n",
        target, path, fill_rule, path_matrix, grad, paint_color, blend, filter);

#if gcFEATURE_VG_ERROR_CHECK
    VG_LITE_CHECK_NULL_POINTER2(path, path->path);
    VG_LITE_RETURN_ERROR(feature_check_8x_csaa_support(path->quality));
    VG_LITE_RETURN_ERROR(feature_check_lvgl_blend_mode(blend));
    VG_LITE_RETURN_ERROR(feature_check_24bit_packed_format(target->format));
    VG_LITE_RETURN_ERROR(feature_check_stencil_image_mode(source->image_mode));
    VG_LITE_RETURN_ERROR(feature_check_new_blend_mode(blend));
    VG_LITE_RETURN_ERROR(feature_check_grad_spread_mode(grad->spread_mode));

    if (radius < 0) {
        return VG_LITE_INVALID_ARGUMENT;
    }
    VG_LITE_RETURN_ERROR(feature_check_compress(source->format, source->compress_mode, source->tiled, source->width, source->height));
#endif /* gcFEATURE_VG_ERROR_CHECK */

    if (!path->path_length) {
        return VG_LITE_SUCCESS;
    }

    if (!path_matrix) {
        path_matrix = &identity_mtx;
    }

#if gcFEATURE_VG_GAMMA
    set_gamma_dest_only(target, VGL_TRUE);
#endif

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_GLOBAL));
#endif

    /*blend input into context*/
    s_context.blend_mode = blend;

    src_premultiply_enable = 0x01000100;
    if (s_context.color_transform == 0 && s_context.gamma_dst == s_context.gamma_src && s_context.matrix_enable == 0 && s_context.dst_alpha_mode == 0 && s_context.src_alpha_mode == 0 &&
        (source->image_mode == VG_LITE_NORMAL_IMAGE_MODE || source->image_mode == 0)) {
        prediv_flag = 0;
    }
    else {
        prediv_flag = 1;
    }
    if ((s_context.blend_mode >= OPENVG_BLEND_SRC && s_context.blend_mode <= OPENVG_BLEND_ADDITIVE) || source->image_mode == VG_LITE_STENCIL_MODE
        || is_lvgl_blend_mode(s_context.blend_mode)
        )
    {
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
        in_premult = 0x00000000;
    }
    if (source->premultiplied == target->premultiplied && premul_flag == 0) {
        target->apply_premult = 1;
    }
    else {
        target->apply_premult = 0;
    }

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        if (error == VG_LITE_NO_CONTEXT) {
            /* If scissoring is enabled and no valid scissoring rectangles
               are present, no drawing occurs */
            return VG_LITE_SUCCESS;
        }
        else {
            return error;
        }
    }

    if (is_packed_yuy_format(target->format) && path->quality != VG_LITE_LOW) {
        path->quality = VG_LITE_LOW;
        printf("If target is YUV group , the path qulity should use VG_LITE_LOW.\n");
    }

    transparency_mode = (source->transparency_mode == VG_LITE_IMAGE_TRANSPARENT ? 0x8000:0);

    width = target->width;
    height = target->height;
    if (s_context.scissor_set) {
        width = s_context.scissor[2] - s_context.scissor[0];
        height = s_context.scissor[3] - s_context.scissor[1];
    }
    if (width == 0 || height == 0)
        return VG_LITE_NO_CONTEXT;
    if ((target->width <= width) && (target->height <= height) && (!s_context.scissor_set))
    {
        ts_is_fullscreen = 1;
        point_min.x = 0;
        point_min.y = 0;
        point_max.x = target->width;
        point_max.y = target->height;
    }


    conversion = feature_a124_a8l8_l8_conversion(target->format, source->format);

    /* Determine image mode (NORMAL, NONE , MULTIPLY or STENCIL) depending on the color. */
    switch (source->image_mode) {
        case VG_LITE_NONE_IMAGE_MODE:
            imageMode = 0x0;
            break;

        case VG_LITE_MULTIPLY_IMAGE_MODE:
            return VG_LITE_INVALID_ARGUMENT;

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

    tiled_source = (source->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0 ;

    switch (grad->spread_mode) {
        case VG_LITE_GRADIENT_SPREAD_FILL:
            rad_tile = 0x0;
            break;

        case VG_LITE_GRADIENT_SPREAD_PAD:
            rad_tile = 0x1000;
            break;

        case VG_LITE_GRADIENT_SPREAD_REPEAT:
            rad_tile = 0x2000;
            break;

        case VG_LITE_GRADIENT_SPREAD_REFLECT:
            rad_tile = 0x3000;
            break;
    }

    compress_mode = (vg_lite_uint32_t)source->compress_mode << 25;

    if (grad->spread_mode == VG_LITE_GRADIENT_SPREAD_FILL)
    {
        vg_lite_uint8_t a,r,g,b;
        a = paint_color >> 24;
        r = paint_color >> 16;
        g = paint_color >> 8;
        b = paint_color;
        paint_color = (a << 24) | (b << 16) | (g << 8) | r;
    }

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix))
        return VG_LITE_SUCCESS;

    /* compute radial gradient paremeters */

    /* Make shortcuts to the gradient information. */
    centerX = grad->radial_grad.cx;
    centerY = grad->radial_grad.cy;
    focalX  = grad->radial_grad.fx;
    focalY  = grad->radial_grad.fy;

    /* Compute constants of the equation. */
    fx           = focalX - centerX;
    fy           = focalY - centerY;
    radius2      = radius * radius;
    if (fx*fx + fy*fy > radius2)
    {
        /* If the focal point is outside the circle, let's move it 
            to inside the circle. Per vg11 spec pg125 "If (fx, fy) lies outside ... 
            For here, we set it at 0.9 ratio to the center.
        */
        vg_lite_float_t fr = (vg_lite_float_t)sqrt((vg_lite_double_t)((vg_lite_double_t)fx* (vg_lite_double_t)fx + (vg_lite_double_t)fy*(vg_lite_double_t)fy));
        fx = radius * fx / fr * 0.9f;
        fy = radius * fy / fr * 0.9f;
        focalX = grad->radial_grad.fx + fx;
        focalY = grad->radial_grad.fy + fy;
    }

    fxfy_2       = 2.0f * fx * fy;
    r2_fx2       = radius2 - fx * fx;
    r2_fy2       = radius2 - fy * fy;
    r2_fx2_2     = 2.0f * r2_fx2;
    r2_fy2_2     = 2.0f * r2_fy2;
#if gcFEATURE_VG_MATH_PRECISION_FIX_DISABLE
    r2_fx2_fy2   = (r2_fx2  - fy * fy) / source->width;
    r2_fx2_fy2sq = (r2_fx2_fy2 * r2_fx2_fy2);
#else
    r2_fx2_fy2   = r2_fx2  - fy * fy;
    r2_fx2_fy2sq = r2_fx2_fy2 * r2_fx2_fy2;
#endif

    /*                        _____________________________________
    **      dx fx + dy fy + \/r^2 (dx^2 + dy^2) - (dx fy - dy fx)^2
    **  g = -------------------------------------------------------
    **                         r^2 - fx^2 - fy^2
    **
    **  Where
    **
    **      dx := F(x) - focalX
    **      dy := F(y) - focalY
    **      fx := focalX - centerX
    **      fy := focalX - centerY
    **
    **  and
    **
    **      F(x) := (x + 0.5) m00 + (y + 0.5) m01 + m02
    **      F(y) := (x + 0.5) m10 + (y + 0.5) m11 + m12
    **
    **  So, dx can be factored into
    **
    **      dx = (x + 0.5) m00 + (y + 0.5) m01 + m02 - focalX
    **         = x m00 + y m01 + 0.5 m00 + 0.5 m01 + m02 - focalX
    **
    **         = x m00 + y m01 + cx
    **
    **  where
    **
    **      cx := 0.5 m00 + 0.5 m01 + m02 - focalX
    **
    **  The same way we can factor dy into
    **
    **      dy = x m10 + y m11 + cy
    **
    **  where
    **
    **      cy := 0.5 m10 + 0.5 m11 + m12 - focalY.
    **
    **  Now we can rewrite g as
    **                               ______________________________________
    **        dx fx + dy fy         / r^2 (dx^2 + dy^2) - (dx fy - dy fx)^2
    **  g = ----------------- + \  /  -------------------------------------
    **      r^2 - fx^2 - fy^2    \/           (r^2 - fx^2 - fy^2)^2
    **               ____
    **    = gLin + \/gRad
    **
    **  where
    **
    **                dx fx + dy fy
    **      gLin := -----------------
    **              r^2 - fx^2 - fy^2
    **
    **              r^2 (dx^2 + dy^2) - (dx fy - dy fx)^2
    **      gRad := -------------------------------------
    **                      (r^2 - fx^2 - fy^2)^2
    */

    cx
        = 0.5f * ( MAT(&inverse_matrix, 0, 0) + MAT(&inverse_matrix, 0, 1) )
        + MAT(&inverse_matrix, 0, 2)
        - focalX;

    cy
        = 0.5f * ( MAT(&inverse_matrix, 1, 0) + MAT(&inverse_matrix, 1, 1) )
        + MAT(&inverse_matrix, 1, 2)
        - focalY;

    /*
    **            dx fx + dy fy
    **  gLin := -----------------
    **          r^2 - fx^2 - fy^2
    **
    **  We can factor the top half into
    **
    **      = (x m00 + y m01 + cx) fx + (x m10 + y m11 + cy) fy
    **
    **      = x (m00 fx + m10 fy)
    **      + y (m01 fx + m11 fy)
    **      + cx fx + cy fy.
    */

    rgStepXLin
        = ( MAT(&inverse_matrix, 0, 0) * fx + MAT(&inverse_matrix, 1, 0) * fy )
        / r2_fx2_fy2;

    rgStepYLin
        = ( MAT(&inverse_matrix, 0, 1) * fx + MAT(&inverse_matrix, 1, 1) * fy )
        / r2_fx2_fy2;

    rgConstantLin = ( cx * fx  + cy * fy ) / r2_fx2_fy2;

    /*
    **          r^2 (dx^2 + dy^2) - (dx fy - dy fx)^2
    **  gRad := -------------------------------------
    **                  (r^2 - fx^2 - fy^2)^2
    **
    **          r^2 (dx^2 + dy^2) - dx^2 fy^2 - dy^2 fx^2 + 2 dx dy fx fy
    **       := ---------------------------------------------------------
    **                            (r^2 - fx^2 - fy^2)^2
    **
    **          dx^2 (r^2 - fy^2) + dy^2 (r^2 - fx^2) + 2 dx dy fx fy
    **       := -----------------------------------------------------
    **                          (r^2 - fx^2 - fy^2)^2
    **
    **  First, lets factor dx^2 into
    **
    **      dx^2 = (x m00 + y m01 + cx)^2
    **           = x^2 m00^2 + y^2 m01^2 + 2 x y m00 m01
    **           + 2 x m00 cx + 2 y m01 cx + cx^2
    **
    **           = x^2 (m00^2)
    **           + y^2 (m01^2)
    **           + x y (2 m00 m01)
    **           + x (2 m00 cx)
    **           + y (2 m01 cx)
    **           + cx^2.
    **
    **  The same can be done for dy^2:
    **
    **      dy^2 = x^2 (m10^2)
    **           + y^2 (m11^2)
    **           + x y (2 m10 m11)
    **           + x (2 m10 cy)
    **           + y (2 m11 cy)
    **           + cy^2.
    **
    **  Let's also factor dx dy into
    **
    **      dx dy = (x m00 + y m01 + cx) (x m10 + y m11 + cy)
    **            = x^2 m00 m10 + y^2 m01 m11 + x y m00 m11 + x y m01 m10
    **            + x m00 cy + x m10 cx + y m01 cy + y m11 cx + cx cy
    **
    **            = x^2 (m00 m10)
    **            + y^2 (m01 m11)
    **            + x y (m00 m11 + m01 m10)
    **            + x (m00 cy + m10 cx)
    **            + y (m01 cy + m11 cx)
    **            + cx cy.
    **
    **  Now that we have all this, lets look at the top of gRad.
    **
    **      = dx^2 (r^2 - fy^2) + dy^2 (r^2 - fx^2) + 2 dx dy fx fy
    **      = x^2 m00^2 (r^2 - fy^2) + y^2 m01^2 (r^2 - fy^2)
    **      + x y 2 m00 m01 (r^2 - fy^2) + x 2 m00 cx (r^2 - fy^2)
    **      + y 2 m01 cx (r^2 - fy^2) + cx^2 (r^2 - fy^2)
    **      + x^2 m10^2 (r^2 - fx^2) + y^2 m11^2 (r^2 - fx^2)
    **      + x y 2 m10 m11 (r^2 - fx^2) + x 2 m10 cy (r^2 - fx^2)
    **      + y 2 m11 cy (r^2 - fx^2) + cy^2 (r^2 - fx^2)
    **      + x^2 m00 m10 2 fx fy + y^2 m01 m11 2 fx fy
    **      + x y (m00 m11 + m01 m10) 2 fx fy
    **      + x (m00 cy + m10 cx) 2 fx fy + y (m01 cy + m11 cx) 2 fx fy
    **      + cx cy 2 fx fy
    **
    **      = x^2 ( m00^2 (r^2 - fy^2)
    **            + m10^2 (r^2 - fx^2)
    **            + m00 m10 2 fx fy
    **            )
    **      + y^2 ( m01^2 (r^2 - fy^2)
    **            + m11^2 (r^2 - fx^2)
    **            + m01 m11 2 fx fy
    **            )
    **      + x y ( 2 m00 m01 (r^2 - fy^2)
    **            + 2 m10 m11 (r^2 - fx^2)
    **            + (m00 m11 + m01 m10) 2 fx fy
    **            )
    **      + x ( 2 m00 cx (r^2 - fy^2)
    **          + 2 m10 cy (r^2 - fx^2)
    **          + (m00 cy + m10 cx) 2 fx fy
    **          )
    **      + y ( 2 m01 cx (r^2 - fy^2)
    **          + 2 m11 cy (r^2 - fx^2)
    **          + (m01 cy + m11 cx) 2 fx fy
    **          )
    **      + cx^2 (r^2 - fy^2) + cy^2 (r^2 - fx^2) + cx cy 2 fx fy.
    */

    rgStepXXRad =
        (
                MAT(&inverse_matrix, 0, 0) * MAT(&inverse_matrix, 0, 0) * r2_fy2
            + MAT(&inverse_matrix, 1, 0) * MAT(&inverse_matrix, 1, 0) * r2_fx2
            + MAT(&inverse_matrix, 0, 0) * MAT(&inverse_matrix, 1, 0) * fxfy_2
        )
        / r2_fx2_fy2sq;

    rgStepYYRad =
        (
                MAT(&inverse_matrix, 0, 1) * MAT(&inverse_matrix, 0, 1) * r2_fy2
            + MAT(&inverse_matrix, 1, 1) * MAT(&inverse_matrix, 1, 1) * r2_fx2
            + MAT(&inverse_matrix, 0, 1) * MAT(&inverse_matrix, 1, 1) * fxfy_2
        )
        / r2_fx2_fy2sq;

    rgStepXYRad =
        (
                MAT(&inverse_matrix, 0, 0) * MAT(&inverse_matrix, 0, 1) * r2_fy2_2
            + MAT(&inverse_matrix, 1, 0) * MAT(&inverse_matrix, 1, 1) * r2_fx2_2
            + (
                    MAT(&inverse_matrix, 0, 0) * MAT(&inverse_matrix, 1, 1)
                + MAT(&inverse_matrix, 0, 1) * MAT(&inverse_matrix, 1, 0)
                )
                * fxfy_2
        )
        / r2_fx2_fy2sq;

    rgStepXRad =
        (
                MAT(&inverse_matrix, 0, 0) * cx * r2_fy2_2
            + MAT(&inverse_matrix, 1, 0) * cy * r2_fx2_2
            + (
                    MAT(&inverse_matrix, 0, 0) * cy
                + MAT(&inverse_matrix, 1, 0) * cx
                )
                * fxfy_2
        )
        / r2_fx2_fy2sq;

    rgStepYRad =
        (
                MAT(&inverse_matrix, 0, 1) * cx * r2_fy2_2
            + MAT(&inverse_matrix, 1, 1) * cy * r2_fx2_2
            + (
                    MAT(&inverse_matrix, 0, 1) * cy
                + MAT(&inverse_matrix, 1, 1) * cx
                )
                * fxfy_2
        )
        / r2_fx2_fy2sq;

    rgConstantRad =
        (
                cx * cx * r2_fy2
            + cy * cy * r2_fx2
            + cx * cy * fxfy_2
        )
        / r2_fx2_fy2sq;

    /* Setup the command buffer. */

    /* rad gradient parameters*/
    data = &rgConstantLin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A04,*(vg_lite_uint32_t*) data));
    data = &rgStepXLin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A06,*(vg_lite_uint32_t*) data));
    data = &rgStepYLin;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A08,*(vg_lite_uint32_t*) data));
    data = &rgConstantRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A05,*(vg_lite_uint32_t*) data));
    data = &rgStepXRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A07,*(vg_lite_uint32_t*) data));
    data = &rgStepYRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A09,*(vg_lite_uint32_t*) data));
    data = &rgStepXXRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A03,*(vg_lite_uint32_t*) data));
    data = &rgStepYYRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A0A,*(vg_lite_uint32_t*) data));
    data = &rgStepXYRad;
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A0B,*(vg_lite_uint32_t*) data));

    /* Compute step values. */
    calculate_step_value(filter, &inverse_matrix, source->width, source->height, x_step, y_step, c_step);

#if gcFEATURE_VG_NEW_FACTOR
    porter_duff_config = s_context.porter_duff_config;
    config_factor_parameter(blend, porter_duff_config, &factor_config);

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF6, factor_config.srcchannelmode | (factor_config.dstchannelmode << 8)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF8, factor_config.factor_src_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF9, factor_config.factor_src_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFA, factor_config.factor_dst_alpha));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AFB, factor_config.factor_dst_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AF7, factor_config.final_equation_opcode));
#endif

    /* Setup the command buffer. */
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

    if (source->yuv.uv_planar) {
        /* Program u plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A51, source->yuv.uv_planar));
    }
    if (source->yuv.v_planar) {
        /* Program v plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source->yuv.v_planar));
    }

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A24, convert_source_format(source->format) |
                                                        filter_mode | uv_swiz | yuv2rgb | rad_tile | conversion | src_premultiply_enable | compress_mode));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A26, paint_color));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A28, source->address));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2A, tiled_source));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2C, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2E, source->width | (source->height << 16)));

    /* Work on path states. */
    matrix = path_matrix;

    if (ts_is_fullscreen == 0) {
        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[1], matrix);
        point_min = point_max = temp;

        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[1], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        transform(&temp, (vg_lite_float_t)path->bounding_box[2], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        transform(&temp, (vg_lite_float_t)path->bounding_box[0], (vg_lite_float_t)path->bounding_box[3], matrix);
        if (temp.x < point_min.x) point_min.x = temp.x;
        if (temp.y < point_min.y) point_min.y = temp.y;
        if (temp.x > point_max.x) point_max.x = temp.x;
        if (temp.y > point_max.y) point_max.y = temp.y;

        point_min.x = MAX(point_min.x, 0);
        point_min.y = MAX(point_min.y, 0);
        point_max.x = MIN(point_max.x, target->width);
        point_max.y = MIN(point_max.y, target->height);

        if (s_context.scissor_set) {
            point_min.x = MAX(point_min.x, s_context.scissor[0]);
            point_min.y = MAX(point_min.y, s_context.scissor[1]);
            point_max.x = MIN(point_max.x, s_context.scissor[2]);
            point_max.y = MIN(point_max.y, s_context.scissor[3]);
        }
    }

    width = point_max.x - point_min.x;
    height = point_max.y - point_min.y;
    Scale = 1.0f;
    Bias = 0.0f;
    new_matrix[0] = matrix->m[0][0] * Scale;
    new_matrix[1] = matrix->m[0][1] * Scale;
    new_matrix[2] = (matrix->m[0][0] + matrix->m[0][1]) * Bias + matrix->m[0][2];
    new_matrix[3] = matrix->m[1][0] * Scale;
    new_matrix[4] = matrix->m[1][1] * Scale;
    new_matrix[5] = (matrix->m[1][0] + matrix->m[1][1]) * Bias + matrix->m[1][2];

    /* Convert states into hardware values. */
    blend_mode = convert_blend(blend);
    format = convert_path_format(path->format);
    quality = convert_path_quality(path->quality);
    tiling = (s_context.capabilities.cap.tiled == 2) ? 0x2000000 : 0;
    fill = (fill_rule == VG_LITE_FILL_EVEN_ODD) ? 0x10 : 0;
    tessellation_size = s_context.tessbuf.tessbuf_size;

    VG_LITE_RETURN_ERROR(chip_set_tes_tile(target, &tile_setting));
    /* Setup the command buffer. */
    /* Program color register. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x02000002 | s_context.capabilities.cap.tiled | in_premult | imageMode | blend_mode | tile_setting | transparency_mode | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | s_context.scissor_enable));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A01, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000000 | format | quality | tiling | fill));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3B, 0x3F800000));      /* Path tessellation SCALE. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A3C, 0x00000000));      /* Path tessellation BIAS.  */
    /* Program matrix. */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A40, (vg_lite_void *) &new_matrix[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A41, (vg_lite_void *) &new_matrix[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A42, (vg_lite_void *) &new_matrix[2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A43, (vg_lite_void *) &new_matrix[3]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A44, (vg_lite_void *) &new_matrix[4]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A45, (vg_lite_void *) &new_matrix[5]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0ACD, (vg_lite_void *) &matrix->m[0][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0ACE, (vg_lite_void *) &matrix->m[1][2]));

    if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1)
    {
        if (path->path_changed != 0) {
            if (path->uploaded.handle != NULL) {
                free_memory.memory_handle = path->uploaded.handle;
                vg_lite_kernel(VG_LITE_FREE, &free_memory);
                path->uploaded.address = 0;
                path->uploaded.memory = NULL;
                path->uploaded.handle = NULL;
            }
            /* Allocate memory for the path data. */
            memory.bytes = 16 + VG_LITE_ALIGN(path->path_length, 8);
            return_offset = (8 + VG_LITE_ALIGN(path->path_length, 8)) / 4;
            memory.contiguous = 1;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &memory));
            ((uint64_t *) memory.memory)[(path->path_length + 7) / 8] = 0;
            ((vg_lite_uint32_t *) memory.memory)[0] = VG_LITE_DATA((path->path_length + 7) / 8);
            ((vg_lite_uint32_t *) memory.memory)[1] = 0;
            memcpy((vg_lite_uint8_t *) memory.memory + 8, path->path, path->path_length);
            ((vg_lite_uint32_t *) memory.memory)[return_offset] = VG_LITE_RETURN();
            ((vg_lite_uint32_t *) memory.memory)[return_offset + 1] = 0;

            path->uploaded.handle = memory.memory_handle;
            path->uploaded.memory = memory.memory;
            path->uploaded.address = memory.memory_gpu;
            path->uploaded.bytes  = memory.bytes;
            path->path_changed = 0;
        }
    }

#if DUMP_CAPTURE
    if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
        vglitemDUMP_BUFFER("path", (size_t)path->uploaded.address, (vg_lite_uint8_t *)(path->uploaded.memory), 0, path->uploaded.bytes);
    }
    vglitemDUMP("@[memory 0x%08X 0x%08X]", s_context.tessbuf.physical_addr, s_context.tessbuf.tessbuf_size);
#endif

    if (width + point_min.x > target->width) {
        width = target->width - point_min.x;
    }

#if gcFEATURE_COMBO_VG_SPLIT_PATH_SUPPORT_BY_SW
    if (s_context.split_path)   
    {
        vg_lite_int32_t y;
        vg_lite_int32_t temp_height = 0;
        height = s_context.tessbuf.tess_w_h >> 16;
        if (path->path_type == VG_LITE_DRAW_FILL_PATH || path->path_type == VG_LITE_DRAW_ZERO) {
#if gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE
            if (height <= 128)
                parallel_workpaths1 = 4;
            else 
                parallel_workpaths1 = height * 128 / 4096 - 1;

            if (parallel_workpaths1 > parallel_workpaths2)
                parallel_workpaths1 = parallel_workpaths2;
#endif

            for (y = point_min.y; y < point_max.y; y += height) {
                vg_lite_uint32_t tile_wh;

                if (y + height > target->height) {
                    temp_height = target->height - y;
                    tile_wh = (vg_lite_uint32_t)(width | (temp_height << 16));
                } else {
                    tile_wh = (vg_lite_uint32_t)(width | (height << 16));
                }
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(point_min.x | (y << 16)), tile_wh, 0));

                if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
                } else {
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->path_length, path->path));
                }
#if gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE
                s_context.path_counter ++;
                if (parallel_workpaths1 == s_context.path_counter) {
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0E02, 0x10 | (0x7 << 8)));
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0F00, 0x10 | (0x7 << 8)));
                    s_context.path_counter = 0;
                }
#endif
            }
        }

        if (path->path_type == VG_LITE_DRAW_STROKE_PATH || path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH) {
            for (y = point_min.y; y < point_max.y; y += height) {
                vg_lite_uint32_t tile_wh;

                if (y + height > target->height) {
                    temp_height = target->height - y;
                    tile_wh = (vg_lite_uint32_t)(width | (temp_height << 16));
                } else {
                    tile_wh = (vg_lite_uint32_t)(width | (height << 16));
                }
                VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
                    (vg_lite_uint32_t)(point_min.x | (y << 16)), tile_wh, 0));

                if (VLM_PATH_GET_UPLOAD_BIT(*path) == 1) {
                    VG_LITE_RETURN_ERROR(push_call(&s_context, path->uploaded.address, path->uploaded.bytes));
                } else {
                    format = convert_path_format(VG_LITE_FP32);
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0x01000200 | format | quality | tiling | 0x0));
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, path->stroke_color));
                    VG_LITE_RETURN_ERROR(push_data(&s_context, path->stroke_size, path->stroke_path));
                }
#if gcFEATURE_VG_512_PARALLEL_PATHS_DISABLE
                s_context.path_counter ++;
                if (parallel_workpaths1 == s_context.path_counter) {
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0E02, 0x10 | (0x7 << 8)));
                    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0F00, 0x10 | (0x7 << 8)));
                    s_context.path_counter = 0;
                }
#endif
            }
        }
    }
    else
#else
#if gcFEATURE_VG_HW_STROKE
    {
        /* Tessellate path. */
        s_context.tessbuf.tess_w_h = width | (height << 16);
        s_context.tessbuf.tess_x_y = point_min.x | (point_min.y << 16);
        VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
            s_context.tessbuf.tess_x_y, s_context.tessbuf.tess_w_h, 0));

        vg_lite_uint16_t  no_use_hw_stroke_flag = 0;

        if (path->stroke)
            no_use_hw_stroke_flag = path->stroke->join_style == VG_LITE_JOIN_MITER || path->stroke->cap_style == VG_LITE_CAP_SQUARE  || path->stroke->dash_phase != 0;
        
        if (!no_use_hw_stroke_flag)
        {
            if (path->path_type == VG_LITE_DRAW_FILL_STROKE_PATH || path->path_type == VG_LITE_DRAW_STROKE_PATH)
            {
                VG_LITE_RETURN_ERROR(stroke_config(path, path_matrix, point_min, width, height));
            }
            VG_LITE_RETURN_ERROR(push_path_advance(path, quality, tiling, in_premult, blend_mode, 3));
        }
        else
        {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0C06, 0x00000000));
            VG_LITE_RETURN_ERROR(push_path_base(path, quality, tiling, in_premult, blend_mode, 3));        
        }
    }
#else   
    {
        /* Tessellate path. */
        s_context.tessbuf.tess_w_h = width | (height << 16);
        s_context.tessbuf.tess_x_y = point_min.x | (point_min.y << 16);
        VG_LITE_RETURN_ERROR(push_state_tess_path_ts_regs(tessellation_size,
            s_context.tessbuf.tess_x_y, s_context.tessbuf.tess_w_h, 0));
        VG_LITE_RETURN_ERROR(push_path_base(path, quality, tiling, in_premult, blend_mode, 3));
    }
#endif
#endif

    /* Finialize command buffer. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));

#if !gcFEATURE_VG_NEW_FACTOR
    VG_LITE_RETURN_ERROR(feature_war_legacy_lvgl_blend(blend, VG_LITE_NORMAL));
#endif

#if gcFEATURE_COMBO_VG_24BIT_PLANAR_BY_SW
    if ((target->format >= VG_LITE_ABGR8565) && (target->format <= VG_LITE_RGBA5658))
    {
        if (target->sw24bit_planar_buffer)
            target = target->sw24bit_planar_buffer;
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

#endif /* (CHIPID==0x355 || CHIPID==0x255) */

/* GC555/GC355/GC255 vg_lite_draw_grad API implementation
 */
vg_lite_error_t vg_lite_draw_grad(vg_lite_buffer_t* target,
                                vg_lite_path_t* path,
                                vg_lite_fill_t fill_rule,
                                vg_lite_matrix_t* matrix,
                                vg_lite_linear_gradient_t* grad,
                                vg_lite_blend_t blend)
{
    DUMP_LAST_FRAME_CAPTURE(VG_LITE_DRAW_GRAD_API);
    DUMP_API_CALL(vg_lite_draw_grad, target, path, fill_rule, matrix, grad, blend);

    return vg_lite_draw_pattern(target, path, fill_rule, matrix,
        &grad->image, &grad->matrix, blend, VG_LITE_PATTERN_PAD, 0, 0, VG_LITE_FILTER_LINEAR);
}