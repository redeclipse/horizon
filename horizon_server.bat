@ECHO OFF
setlocal enableextensions enabledelayedexpansion
:path
    if DEFINED HORIZON_PATH goto init
    pushd %~dp0
    set HORIZON_PATH=%CD%
    popd
:init
    set HORIZON_BINARY=horizon_server
:start
    call "%HORIZON_PATH%\horizon.bat" %*
