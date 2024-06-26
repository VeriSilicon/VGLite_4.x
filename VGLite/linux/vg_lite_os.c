/****************************************************************************
*
*    Copyright 2012 - 2023 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

#include "stdlib.h"
#include "stdio.h"

void* vg_lite_os_malloc(size_t size)
{
    return malloc(size);
}

void vg_lite_os_free(void* memory)
{
    free(memory);
}

int vg_lite_os_fseek(FILE* Stream, long Offset, int Origin)
{
    int value = 0;
    value = fseek(Stream, Offset, Origin);
    return value;
}

FILE* vg_lite_os_fopen(char const* FileName, char const* Mode) 
{
    FILE* file;
    file = fopen(FileName, Mode);
    return file;
}

long vg_lite_os_ftell(FILE* Stream)
{
    long value;
    value = ftell(Stream);
    return value;
}

size_t vg_lite_os_fread(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream)
{
    size_t value;
    value = fread(Buffer, ElementSize, ElementCount, Stream);
    return value;
}

size_t vg_lite_os_fwrite(void const* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream)
{
    size_t value;
    value = fwrite(Buffer, ElementSize, ElementCount, Stream);
    return value;
}

int vg_lite_os_close(FILE* Stream)
{
    int value;
    value = fclose(Stream);
    return value;
}

int vg_lite_os_fflush(FILE* Stream)
{
    int value;
    value = fflush(Stream);
    return value;
}
