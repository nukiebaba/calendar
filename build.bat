@echo off

set platform=x64

set projectDirectory=%~dp0
set buildToolsPath="C:\Program Files (x86)\Microsoft Visual C++ Build Tools"

call %buildToolsPath%\vcbuildtools.bat %platform%

cd /d %projectDirectory%
if not exist build\windows (
   mkdir build\windows
)
pushd build\windows
cl -Zi ..\..\src\win32_calendar.cpp user32.lib gdi32.lib
popd
