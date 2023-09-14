/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <zephyr.h>
#include <fs/fs.h>

extern void * vg_lite_os_malloc(size_t size);
extern void vg_lite_os_free(void *memory);

void * vg_lite_os_fopen(const char *__restrict path, const char *__restrict mode)
{
    struct fs_file_t *zfp = NULL;
    fs_mode_t flags = 0;
    int rc;

    for (; *mode != 0; mode++) {
        switch (*mode) {
        case 'r':
            flags |= FS_O_READ;
            break;
        case '+':
            flags |= FS_O_RDWR;
            break;
        case 'w':
            flags |= FS_O_WRITE | FS_O_CREATE;
            break;
        case 'a':
            flags |= FS_O_WRITE | FS_O_CREATE | FS_O_APPEND;
            break;
        default:
            break;
        }
    }

    if (flags == 0)
        return NULL;

    zfp = vg_lite_os_malloc(sizeof(*zfp));
    if (zfp == NULL)
        return NULL;

    fs_file_t_init(zfp);
    rc = fs_open(zfp, path, flags);
    if (rc != 0) {
        vg_lite_os_free(zfp);
        return NULL;
    }

    return zfp;
}

int vg_lite_os_fclose(void * fp)
{
    int rc = -EINVAL;

    if (fp) {
        rc = fs_close(fp);
        vg_lite_os_free(fp);
    }

    return rc;
}

size_t vg_lite_os_fread(void *__restrict ptr, size_t size, size_t nmemb, void *__restrict fp)
{
    ssize_t num = 0;

    if (fp) {
        num = fs_read(fp, ptr, size * nmemb);
    }

    return (num <= 0) ? 0 : (num / size);
}

size_t vg_lite_os_fwrite(const void *__restrict ptr, size_t size, size_t nmemb, void * fp)
{
    ssize_t num = 0;

    if (fp) {
        num = fs_write(fp, ptr, size * nmemb);
    }

    return (num <= 0) ? 0 : (num / size);
}

int vg_lite_os_fseek(void * fp, long offset, int whence)
{
    if (fp == NULL)
        return -EINVAL;

    switch (whence) {
    case SEEK_SET:
        whence = FS_SEEK_SET;
        break;
    case SEEK_CUR:
        whence = FS_SEEK_CUR;
        break;
    case SEEK_END:
        whence = FS_SEEK_END;
        break;
    default:
        return -EINVAL;
    }

    return fs_seek(fp, offset, whence);
}

int vg_lite_os_fprintf(void *__restrict fp, const char *__restrict format, ...)
{
    static char buffer[1024];

    va_list ap;
    int len;

    if (fp == NULL)
        return -EINVAL;

    va_start(ap, format);
    len = vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);

    return fs_write(fp, buffer, len);
}

int vg_lite_os_getpid(void)
{
    return (intptr_t)k_current_get();
}
