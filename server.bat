@ECHO OFF

set HORIZON_BIN=bin

IF /I "%PROCESSOR_ARCHITECTURE%" == "amd64" (
    set HORIZON_BIN=bin64
)
IF /I "%PROCESSOR_ARCHITEW6432%" == "amd64" (
    set HORIZON_BIN=bin64
)

start %HORIZON_BIN%\horizon.exe "-u$HOME\My Games\Red Eclipse Horizon" -gserver-log.txt -d %*
