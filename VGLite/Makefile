##############################################################################
#
#    The MIT License (MIT)
#
#    Copyright (c) 2014 - 2026 Vivante Corporation
#
#    Permission is hereby granted, free of charge, to any person obtaining a
#    copy of this software and associated documentation files (the "Software"),
#    to deal in the Software without restriction, including without limitation
#    the rights to use, copy, modify, merge, publish, distribute, sublicense,
#    and/or sell copies of the Software, and to permit persons to whom the
#    Software is furnished to do so, subject to the following conditions:
#
#    The above copyright notice and this permission notice shall be included in
#    all copies or substantial portions of the Software.
#
#    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#    DEALINGS IN THE SOFTWARE.
#
##############################################################################

include ../Makefile.def

OUT = ../bin
NAME = libvg_lite.so

SOURCES = vg_lite_dump.c vg_lite.c vg_lite_path.c vg_lite_stroke.c vg_lite_matrix.c vg_lite_image.c vg_lite_chip.c ./linux/vg_lite_os.c ./linux/vg_lite_ioctl.c

CFLAGS += -I../inc -I../VGLiteKernel -I. -I./linux

LDFLAGS += -fPIC -shared -Wl,-z,defs
LDFLAGS += -Wl,--version-script=vglite.map
LDFLAGS += -lpthread -lpng -lm

ifeq ($(dumpAPI), 1)
LDFLAGS += -L../../Dump_api/bin
LDLIBS  += -ldump_api
endif

all : $(OUT)/$(NAME)

clean:
	@rm -f $(OUT)/$(NAME)

$(OUT):
	@mkdir $(OUT)

$(OUT)/$(NAME) : $(OUT) $(SOURCES)
	@echo $(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
	@$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
