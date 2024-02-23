This is a VGlite tool to dump all the APIs which are used when running a case.

TO enable the functionality:
a) In workspace/SW/VGLite/Hubi.dev/VGLite/vg_lite_context.h, set macro DUMP_API to 1;
b) In workspace/SW/VGLite/Hubi.dev/build_linux.sh, set variable dumpAPI to 1;
c) In workspace/TEST/SW/VGLite/Conformance/samples/util/vg_lite_util.h, set macro DUMP_API to 1.

Then run the case, dump file will be generated.