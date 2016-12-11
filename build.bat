@echo off

set platform=x64

set projectDirectory=%~dp0
set buildToolsPath="C:\Program Files (x86)\Microsoft Visual C++ Build Tools"

call %buildToolsPath%\vcbuildtools.bat %platform%
cd /d %projectDirectory%
pushd build\windows
cl -Zi ..\..\src\calendar.cpp user32.lib
popd
