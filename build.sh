#!/bin/bash

error_code=0
directory=$(dirname $(readlink -f "$0"))
echo Working directory: $directory

CLANG_PATH="$HOME/clang/5.0/cc-toolchain/build/bin/clang"

if [ -d $directory ]; then
    if uname -a | grep 'CYGWIN_NT'; then
        buildDirectory="$directory/build/cygwin"
        if [ ! -d "$buildDirectory" ]; then
            mkdir -p "$buildDirectory"
        fi

        $directory/resources/ctime.exe -begin $directory/calendar.ctm
        $CLANG_PATH $directory/src/linux_calendar.cpp -o $buildDirectory/calendar -g
        error_code=$?
        $directory/resources/ctime.exe -end $directory/calendar.ctm "${error_code}"
    else
        buildDirectory="$directory/build/linux"
        if [ ! -d "$buildDirectory" ]; then
            mkdir -p "$buildDirectory"
        fi

        case "$1" in
            --release )
                if [ -f $buildDirectory/calendar ]; then
                    $buildDirectory/calendar
                    exit $?
                fi
                break
                ;;
        esac

        $directory/resources/ctime -begin $directory/calendar.ctm
        $CLANG_PATH $directory/src/linux_calendar.cpp -o $buildDirectory/calendar -lX11 -lm -g
        error_code=$?
        $directory/resources/ctime -end $directory/calendar.ctm "${error_code}"
    fi
fi

exit $error_code
