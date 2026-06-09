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

SDK_DIR ?= ./build
.phony: all install doc

all:
ifneq ($(KERNEL_DIR),)
	@$(MAKE) -C VGLiteKernel
endif
	@$(MAKE) -C VGLite

clean:
	@rm -rf bin
ifneq ($(KERNEL_DIR),)
	@$(MAKE) -C VGLiteKernel clean
endif
	@$(MAKE) -C VGLite clean

install: all $(SDK_DIR)
	@cp -rf bin/* $(SDK_DIR)/drivers
	@cp -f inc/* $(SDK_DIR)/inc
ifeq ($(dumpAPI), 1)
	@cp -f ../Dump_api/bin/* $(SDK_DIR)/drivers
endif


$(SDK_DIR):
	@mkdir -p $(SDK_DIR)/drivers
	@mkdir -p $(SDK_DIR)/inc