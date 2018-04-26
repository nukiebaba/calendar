@echo off

set platform=x64

set projectDirectory=%~dp0

REM choco install vcbuildtools
set buildToolsPath="C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat"

REM set buildToolsPath="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"

set debugToolsPath="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"

call %buildToolsPath% %platform%
call %debugToolsPath%

cd /d %projectDirectory%
if not exist build\windows (
   mkdir build\windows
)

pushd build\windows
cl -Zi -FC ..\..\src\win32_calendar.cpp user32.lib gdi32.lib /link /subsystem:windows
popd
