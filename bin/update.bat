@ECHO OFF
setlocal enableextensions enabledelayedexpansion
:horizon_update_path
    if DEFINED HORIZON_PATH goto horizon_update_init
    pushd "%~dp0\.."
    set HORIZON_PATH=%CD%
    popd
:horizon_update_init
    if NOT "%HORIZON_DEPLOY%" == "true" set HORIZON_DEPLOY=false
    if NOT DEFINED HORIZON_UPDATER set HORIZON_UPDATER=%~dp0\%~0
    if NOT DEFINED HORIZON_SOURCE set HORIZON_SOURCE=http://redeclipse.net/horizon
    if NOT DEFINED HORIZON_GITHUB set HORIZON_GITHUB=https://github.com/red-eclipse
    if DEFINED HORIZON_CACHE goto horizon_update_setup
    for /f "tokens=3* delims= " %%a in ('reg query "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" /v "Personal"') do set HORIZON_WINDOCS=%%a
    if EXIST "%HORIZON_WINDOCS%" (
        set HORIZON_CACHE=%HORIZON_WINDOCS%\My Games\Red Eclipse Horizon\cache
    ) else if EXIST "%HORIZON_HOME%" (
        set HORIZON_CACHE=%HORIZON_HOME%\cache
    ) else (
        set HORIZON_CACHE=cache
    )
:horizon_update_setup
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
    set HORIZON_UPDATE=%HORIZON_BRANCH%
    set HORIZON_TEMP=%HORIZON_CACHE%\%HORIZON_BRANCH%
:horizon_update_branch
    echo branch: %HORIZON_UPDATE%
    echo folder: %HORIZON_PATH%
    echo cached: %HORIZON_TEMP%
    if NOT EXIST "%HORIZON_PATH%\bin\tools\curl.exe" (
        echo Unable to find curl.exe, are you sure it is in tools?
        exit /b 0
    )
    set HORIZON_DOWNLOADER="%HORIZON_PATH%\bin\tools\curl.exe" -L -k -f -A "horizon-%HORIZON_UPDATE%" -o
    if NOT EXIST "%HORIZON_PATH%\bin\tools\unzip.exe" (
        echo Unable to find unzip.exe, are you sure it is in tools?
        exit /b 0
    )
    set HORIZON_UNZIP="%HORIZON_PATH%\bin\tools\unzip.exe" -o
    if NOT EXIST "%HORIZON_TEMP%" mkdir "%HORIZON_TEMP%"
    echo @ECHO OFF> "%HORIZON_TEMP%\install.bat"
    echo setlocal enableextensions>> "%HORIZON_TEMP%\install.bat"
    if "%HORIZON_BRANCH%" == "devel" goto horizon_update_bins_run
:horizon_update_module
    echo modules: Updating..
    %HORIZON_DOWNLOADER% "%HORIZON_TEMP%\mods.txt" "%HORIZON_SOURCE%/%HORIZON_UPDATE%/mods.txt"
    if NOT EXIST "%HORIZON_TEMP%\mods.txt" (
        echo modules: Failed to retrieve update information.
        goto horizon_update_bins_run
    )
    set /p HORIZON_MODULE_LIST=< "%HORIZON_TEMP%\mods.txt"
    if "%HORIZON_MODULE_LIST%" == "" (
        echo modules: Failed to get list, continuing..
        goto horizon_update_bins_run
    )
    if EXIST "%HORIZON_TEMP%\data.txt" del /f /q "%HORIZON_TEMP%\data.txt"
    if EXIST "%HORIZON_TEMP%\data.zip" del /f /q "%HORIZON_TEMP%\data.zip"
    echo modules: Prefetching versions..
    set HORIZON_MODULE_PREFETCH=
    for %%a in (%HORIZON_MODULE_LIST%) do (
        del /f /q "%HORIZON_TEMP%\%%a.txt"
        if NOT "!HORIZON_MODULE_PREFETCH!" == "" (
            set HORIZON_MODULE_PREFETCH=!HORIZON_MODULE_PREFETCH!,%%a
        ) else (set HORIZON_MODULE_PREFETCH=%%a)
    )
    if "%HORIZON_MODULE_PREFETCH" == "" (
        echo modules: Failed to get version information, continuing..
        goto horizon_update_bins_run
    )
    %HORIZON_DOWNLOADER% "%HORIZON_TEMP%\#1.txt" "%HORIZON_SOURCE%/%HORIZON_UPDATE%/{%HORIZON_MODULE_PREFETCH%}.txt"
    for %%a in (%HORIZON_MODULE_LIST%) do (
        set HORIZON_MODULE_RUN=%%a
        if NOT "!HORIZON_MODULE_RUN!" == "" (
            call :horizon_update_module_run "%HORIZON_UPDATER%" || (echo !HORIZON_MODULE_RUN!: There was an error updating the module, continuing..)
        )
    )
    goto horizon_update_bins_run
:horizon_update_module_run
    if "%HORIZON_MODULE_RUN%" == "horizon" (set HORIZON_MODULE_DIR=) else (set HORIZON_MODULE_DIR=\data\%HORIZON_MODULE_RUN%)
    if EXIST "%HORIZON_PATH%%HORIZON_MODULE_DIR%\version.txt" goto horizon_update_module_ver
    echo %HORIZON_MODULE_RUN%: Unable to find version.txt. Will start from scratch.
    set HORIZON_MODULE_INSTALLED=none
    echo mkdir "%HORIZON_PATH%%HORIZON_MODULE_DIR%">> "%HORIZON_TEMP%\install.bat"
    goto horizon_update_module_get
:horizon_update_module_ver
    if EXIST "%HORIZON_PATH%%HORIZON_MODULE_DIR%\version.txt" set /p HORIZON_MODULE_INSTALLED=< "%HORIZON_PATH%%HORIZON_MODULE_DIR%\version.txt"
    if "%HORIZON_MODULE_INSTALLED%" == "" set HORIZON_MODULE_INSTALLED=none
    echo %HORIZON_MODULE_RUN%: %HORIZON_MODULE_INSTALLED% is installed.
:horizon_update_module_get
    if NOT EXIST "%HORIZON_TEMP%\%HORIZON_MODULE_RUN%.txt" (
        echo %HORIZON_MODULE_RUN%: Failed to retrieve update information.
        exit /b 1
    )
    set /p HORIZON_MODULE_REMOTE=< "%HORIZON_TEMP%\%HORIZON_MODULE_RUN%.txt"
    if "%HORIZON_MODULE_REMOTE%" == "" (
        echo %HORIZON_MODULE_RUN%: Failed to read update information.
        exit /b 1
    )
    echo %HORIZON_MODULE_RUN%: %HORIZON_MODULE_REMOTE% is the current version.
    if "%HORIZON_MODULE_REMOTE%" == "%HORIZON_MODULE_INSTALLED%" (
        echo echo %HORIZON_MODULE_RUN%: already up to date.>> "%HORIZON_TEMP%\install.bat"
        exit /b 0
    )
:horizon_update_module_blob
    if EXIST "%HORIZON_TEMP%\%HORIZON_MODULE_RUN%.zip" (
        del /f /q "%HORIZON_TEMP%\%HORIZON_MODULE_RUN%.zip"
    )
    echo %HORIZON_MODULE_RUN%: Downloading %HORIZON_GITHUB%/%HORIZON_MODULE_RUN%/zipball/%HORIZON_MODULE_REMOTE%
    %HORIZON_DOWNLOADER% "%HORIZON_TEMP%\%HORIZON_MODULE_RUN%.zip" "%HORIZON_GITHUB%/%HORIZON_MODULE_RUN%/zipball/%HORIZON_MODULE_REMOTE%"
    if NOT EXIST "%HORIZON_TEMP%\%HORIZON_MODULE_RUN%.zip" (
        echo %HORIZON_MODULE_RUN%: Failed to retrieve update package.
        exit /b 1
    )
:horizon_update_module_blob_deploy
    echo echo %HORIZON_MODULE_RUN%: deploying blob.>> "%HORIZON_TEMP%\install.bat"
    echo %HORIZON_UNZIP% "%HORIZON_TEMP%\%HORIZON_MODULE_RUN%.zip" -d "%HORIZON_TEMP%" ^&^& ^(>> "%HORIZON_TEMP%\install.bat"
    if "%HORIZON_MODULE_RUN%" == "horizon" goto horizon_update_module_blob_deploy_ext
    echo    rmdir /s /q "%HORIZON_PATH%%HORIZON_MODULE_DIR%">> "%HORIZON_TEMP%\install.bat"
    echo    mkdir "%HORIZON_PATH%%HORIZON_MODULE_DIR%">> "%HORIZON_TEMP%\install.bat"
:horizon_update_module_blob_deploy_ext
    echo    xcopy /e /c /i /f /h /y "%HORIZON_TEMP%\red-eclipse-%HORIZON_MODULE_RUN%-%HORIZON_MODULE_REMOTE:~0,7%\*" "%HORIZON_PATH%%HORIZON_MODULE_DIR%">> "%HORIZON_TEMP%\install.bat"
    echo    rmdir /s /q "%HORIZON_TEMP%\red-eclipse-%HORIZON_MODULE_RUN%-%HORIZON_MODULE_REMOTE:~0,7%">> "%HORIZON_TEMP%\install.bat"
    echo    ^(echo %HORIZON_MODULE_REMOTE%^)^> "%HORIZON_PATH%%HORIZON_MODULE_DIR%\version.txt">> "%HORIZON_TEMP%\install.bat"
    echo ^) ^|^| ^(>> "%HORIZON_TEMP%\install.bat"
    echo     del /f /q "%HORIZON_TEMP%\%HORIZON_MODULE_RUN%.txt">> "%HORIZON_TEMP%\install.bat"
    echo     exit 1>> "%HORIZON_TEMP%\install.bat"
    echo ^)>> "%HORIZON_TEMP%\install.bat"
    set HORIZON_DEPLOY=true
    exit /b 0
:horizon_update_bins_run
    echo bins: Updating..
    del /f /q "%HORIZON_TEMP%\bins.txt"
    %HORIZON_DOWNLOADER% "%HORIZON_TEMP%\bins.txt" "%HORIZON_SOURCE%/%HORIZON_UPDATE%/bins.txt"
    if EXIST "%HORIZON_PATH%\bin\version.txt" set /p HORIZON_BINS=< "%HORIZON_PATH%\bin\version.txt"
    if "%HORIZON_BINS%" == "" set HORIZON_BINS=none
    echo bins: %HORIZON_BINS% is installed.
:horizon_update_bins_get
    if NOT EXIST "%HORIZON_TEMP%\bins.txt" (
        echo bins: Failed to retrieve update information.
        goto horizon_update_deploy
    )
    set /p HORIZON_BINS_REMOTE=< "%HORIZON_TEMP%\bins.txt"
    if "%HORIZON_BINS_REMOTE%" == "" (
        echo bins: Failed to read update information.
        goto horizon_update_deploy
    )
    echo bins: %HORIZON_BINS_REMOTE% is the current version.
    if NOT "%HORIZON_TRYUPDATE%" == "true" if "%HORIZON_BINS_REMOTE%" == "%HORIZON_BINS%" (
        echo echo bins: already up to date.>> "%HORIZON_TEMP%\install.bat"
        goto horizon_update_deploy
    )
:horizon_update_bins_blob
    if EXIST "%HORIZON_TEMP%\windows.zip" (
        del /f /q "%HORIZON_TEMP%\windows.zip"
    )
    echo bins: Downloading %HORIZON_SOURCE%/%HORIZON_UPDATE%/windows.zip
    %HORIZON_DOWNLOADER% "%HORIZON_TEMP%\windows.zip" "%HORIZON_SOURCE%/%HORIZON_UPDATE%/windows.zip"
    if NOT EXIST "%HORIZON_TEMP%\windows.zip" (
        echo bins: Failed to retrieve update package.
        goto horizon_update_deploy
    )
:horizon_update_bins_deploy
    echo echo bins: deploying blob.>> "%HORIZON_TEMP%\install.bat"
    echo %HORIZON_UNZIP% "%HORIZON_TEMP%\windows.zip" -d "%HORIZON_PATH%" ^&^& ^(>> "%HORIZON_TEMP%\install.bat"
    echo     ^(echo %HORIZON_BINS_REMOTE%^)^> "%HORIZON_PATH%\bin\version.txt">> "%HORIZON_TEMP%\install.bat"
    echo ^) ^|^| ^(>> "%HORIZON_TEMP%\install.bat"
    echo     del /f /q "%HORIZON_TEMP%\bins.txt">> "%HORIZON_TEMP%\install.bat"
    echo     exit 1>> "%HORIZON_TEMP%\install.bat"
    echo ^)>> "%HORIZON_TEMP%\install.bat"
    set HORIZON_DEPLOY=true
:horizon_update_deploy
    if NOT "%HORIZON_DEPLOY%" == "true" exit /b 0
    echo deploy: %HORIZON_TEMP%\install.bat
    set HORIZON_INSTALL=call
    copy /y nul test.tmp> nul 2>&1 && (
        del /f /q test.tmp
        goto horizon_update_unpack
    )
    echo Administrator permissions are required to deploy the files.
    if NOT EXIST "%HORIZON_PATH%\bin\tools\elevate.exe" (
        echo Unable to find elevate.exe, are you sure it is in tools?
        echo There was an error deploying the files.
        exit /b 1
    )
    set HORIZON_INSTALL="%HORIZON_PATH%\bin\tools\elevate.exe" -wait
:horizon_update_unpack
%HORIZON_INSTALL% "%HORIZON_TEMP%\install.bat" && (
    (echo %HORIZON_BRANCH%)> "%HORIZON_PATH%\branch.txt"
    exit /b 0
) || (
    echo There was an error deploying the files.
    exit /b 1
)
