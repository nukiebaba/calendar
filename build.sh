#!/bin/sh
error_code=0
directory=$(dirname $(readlink -f "$0"))
echo Working directory: $directory

if [ -d $directory ]; then
    if uname -a | grep 'CYGWIN_NT'; then
        buildDirectory="$directory/build/cygwin"
        $directory/resources/ctime.exe -begin calendar.ctm
        clang $directory/src/calendar.cpp -o $buildDirectory/calendar
        error_code=$?
        $directory/resources/ctime.exe -end calendar.ctm "${error_code}"
    else
        buildDirectory="$directory/build/linux"
        $directory/resources/ctime -begin calendar.ctm
        clang $directory/src/calendar.cpp -o $buildDirectory/calendar -lX11 -lm
        error_code=$?
        $directory/resources/ctime -end calendar.ctm "${error_code}"
    fi
fi

exit $error_code
