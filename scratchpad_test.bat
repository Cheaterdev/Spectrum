@echo off
setlocal
set SRC=%~dp0vcpkg_installed\x64-windows\x64-windows\share\nrd\Shaders_test
set DST=%~dp0workdir\shaders\nrd\3rdparty_synctest

if not exist "%SRC%" (
    echo [sync_nrd_shaders] "%SRC%" not found -- nrd vcpkg package not installed yet.
    exit /b 0
)

if not exist "%DST%" mkdir "%DST%"

robocopy "%SRC%" "%DST%" /E /NFL /NDL /NJH /NJS /NP >nul
if %ERRORLEVEL% GEQ 8 (
    echo [sync_nrd_shaders] robocopy failed with code %ERRORLEVEL%
    exit /b 1
)
echo OK exit 0
exit /b 0
