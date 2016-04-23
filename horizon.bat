@ECHO OFF
setlocal enableextensions enabledelayedexpansion
:horizon_path
    set HORIZON_OPTIONS=-glog.txt
    if NOT DEFINED set HORIZON_HOME=$HOME\My Games\Red Eclipse Horizon
    if DEFINED HORIZON_PATH goto horizon_init
    pushd "%~dp0"
    set HORIZON_PATH=%CD%
    popd
:horizon_init
    set HORIZON_SCRIPT=%~dp0\%~0
    for %%a in ("%HORIZON_SCRIPT%") do set HORIZON_SCRIPT_TIME=%%~ta
    if NOT DEFINED HORIZON_BINARY set HORIZON_BINARY=horizon
    set HORIZON_SUFFIX=.exe
    set HORIZON_MAKE=mingw32-make
:horizon_setup
    if DEFINED HORIZON_ARCH goto horizon_branch
    set HORIZON_ARCH=x86
    if DEFINED PROCESSOR_ARCHITEW6432 (
        set HORIZON_MACHINE=%PROCESSOR_ARCHITEW6432%
    ) else (
        set HORIZON_MACHINE=%PROCESSOR_ARCHITECTURE%
    )
    if /i "%HORIZON_MACHINE%" == "amd64" set HORIZON_ARCH=amd64
:horizon_branch
    if EXIST "%HORIZON_PATH%\branch.txt" set /p HORIZON_BRANCH_CURRENT=< "%HORIZON_PATH%\branch.txt"
    if NOT DEFINED HORIZON_BRANCH (
        if DEFINED HORIZON_BRANCH_CURRENT (
            set HORIZON_BRANCH=%HORIZON_BRANCH_CURRENT%
        ) else if EXIST .git (
            set HORIZON_BRANCH=devel
        ) else (
            set HORIZON_BRANCH=stable
        )
    )
    if NOT DEFINED HORIZON_HOME if NOT "%HORIZON_BRANCH%" == "stable" if NOT "%HORIZON_BRANCH%" == "inplace" set HORIZON_HOME=home
    if DEFINED HORIZON_HOME set HORIZON_OPTIONS="-u%HORIZON_HOME%" %HORIZON_OPTIONS%
:horizon_check
    if NOT "%HORIZON_BRANCH%" == "source" goto horizon_notsource
    %HORIZON_MAKE% -C src all install
    goto horizon_runit
:horizon_notsource
    if "%HORIZON_BRANCH%" == "inplace" goto horizon_runit
    echo.
    echo Checking for updates to "%HORIZON_BRANCH%". To disable: set HORIZON_BRANCH=inplace
    echo.
:horizon_begin
    set HORIZON_RETRY=false
    goto horizon_update
:horizon_retry
    if "%HORIZON_RETRY%" == "true" goto horizon_runit
    set HORIZON_RETRY=true
    echo Retrying...
:horizon_update
    call "%HORIZON_PATH%\bin\update.bat" && (
        for %%a in ("%HORIZON_SCRIPT%") do set HORIZON_SCRIPT_NOW=%%~ta
        if NOT "!HORIZON_SCRIPT_NOW!" == "!HORIZON_SCRIPT_TIME!" (
            call :horizon_runit "%HORIZON_SCRIPT%" %*
            exit /b 0
        )
        goto horizon_runit
    ) || (
        for %%a in ("%HORIZON_SCRIPT%") do set HORIZON_SCRIPT_NOW=%%~ta
        if NOT "!HORIZON_SCRIPT_NOW!" == "!HORIZON_SCRIPT_TIME!" (
            call :horizon_retry "%HORIZON_SCRIPT%" %*
            exit /b 0
        )
        goto horizon_retry
    )
:horizon_runit
    if EXIST "%HORIZON_PATH%\bin\%HORIZON_ARCH%\%HORIZON_BINARY%%HORIZON_SUFFIX%" (
        pushd "%HORIZON_PATH%" || goto horizon_error
        start bin\%HORIZON_ARCH%\%HORIZON_BINARY%%HORIZON_SUFFIX% %HORIZON_OPTIONS% %* || (
            popd
            goto horizon_error
        )
        popd
        exit /b 0
    ) else (
        if "%HORIZON_BRANCH%" == "source" (
            %HORIZON_MAKE% -C src all install && goto horizon_runit
            set HORIZON_BRANCH=devel
        )
        if NOT "%HORIZON_BRANCH%" == "inplace" if NOT "%HORIZON_TRYUPDATE%" == "true" (
            set HORIZON_TRYUPDATE=true
            goto horizon_begin
        )
        if NOT "%HORIZON_ARCH%" == "x86" (
            set HORIZON_ARCH=x86
            goto horizon_runit
        )
        echo Unable to find a working binary.
    )
:horizon_error
    echo There was an error running Red Eclipse Horizon.
    pause
    exit /b 1
