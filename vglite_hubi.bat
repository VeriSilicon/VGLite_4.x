
set CMODEL=%cd%\..\..\..\HW\projects.HuBi\arch\XAQ2
set SAMPLES=%cd%\..\..\..\TEST\SW\VGLite\Conformance\windows

mkdir arch
mklink /d /j arch\GC350  %CMODEL%
mklink /d /j tools  ..\..\..\TOOLS

set AQROOT=%cd%
set AQARCH=%AQROOT%\arch\GC350\
set AQARCH2=%AQROOT%\arch\GC350\
set ROOT255=%AQROOT%
set VIVANTE_SDK_DIR=%AQROOT%\BUILD_vgHuBi
set path=%VIVANTE_SDK_DIR%\bin;%path%

sleep 5
"C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\devenv" %AQROOT%\VGLite.sln
sleep 5
"C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\devenv" %SAMPLES%\VGLite_Samples.sln
