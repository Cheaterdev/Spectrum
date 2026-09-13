@echo off
rem Copies NVIDIA NRD's vendored shader source (Shaders/*.hlsl, *.hlsli, plus
rem MathLib's ml.hlsli -- see custom-overlay/nrd/portfile.cmake's own
rem "share/nrd/Shaders" install step) from the vcpkg package into
rem workdir/shaders/nrd/3rdparty/, which is gitignored on purpose (proprietary
rem NVIDIA source, never committed -- see .gitignore's comment there).
rem
rem Run automatically as Spectrum's PostBuildEvent (main.sharpmake.cs) so a
rem fresh clone/machine gets these files with zero extra manual steps -- the
rem first build restores the nrd vcpkg package (headers+lib+this Shaders
rem folder), and this step then syncs them into workdir right after. Safe to
rem re-run any time: copy-only (never deletes), so it can't clobber anything
rem hand-edited in workdir/shaders/nrd/ outside 3rdparty/.
setlocal

set ROOT=%~dp0
set SRC=%ROOT%vcpkg_installed\x64-windows\x64-windows\share\nrd\Shaders
set DST=%ROOT%workdir\shaders\nrd\3rdparty

if not exist "%SRC%" (
    echo [sync_nrd_shaders] "%SRC%" not found -- nrd vcpkg package not installed yet.
    echo [sync_nrd_shaders] Build once to let vcpkg restore it, then rebuild to sync shaders.
    exit /b 0
)

if not exist "%DST%" mkdir "%DST%"

robocopy "%SRC%" "%DST%" /E /NFL /NDL /NJH /NJS /NP >nul
if %ERRORLEVEL% GEQ 8 (
    echo [sync_nrd_shaders] robocopy failed with code %ERRORLEVEL% copying "%SRC%" -^> "%DST%"
    exit /b 1
)

exit /b 0
