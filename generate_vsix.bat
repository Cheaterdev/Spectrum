@echo off
REM Builds the Prism language server (Profile) and packs it with the generated highlighting into
REM bin\editor\prism.vsix (double-click that file to install it).
REM
REM   generate_vsix.bat                 build Prism, then bin\editor\prism.vsix
REM   generate_vsix.bat --install-user  also copy the highlighting-only extension into
REM                                     %USERPROFILE%\.vs\Extensions\Prism (no VSIX needed)
REM
REM Needs Visual Studio with the "Visual Studio extension development" workload, Python, and
REM projects\spectrum.sln (generate_project.bat creates it).
setlocal
cd /d "%~dp0"

if not exist projects\spectrum.sln (
    echo projects\spectrum.sln is missing -- run generate_project.bat first.
    exit /b 1
)

set MSBUILD=
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set MSBUILD=%%i
if not defined MSBUILD (
    echo MSBuild not found.
    exit /b 1
)

REM The extension carries a copy of bin\profile\prismc.exe as its language server, so build it first.
"%MSBUILD%" projects\spectrum.sln /t:Prism /p:Configuration=Profile-D3D12 /p:Platform=x64 /m -nodeReuse:false /nologo /v:minimal
set BUILD_RESULT=%ERRORLEVEL%

REM A lingering MSBuild or compiler server locks files and blocks Visual Studio installers.
powershell -NoProfile -Command "Get-Process MSBuild,VBCSCompiler -ErrorAction SilentlyContinue | Stop-Process -Force"

if not %BUILD_RESULT%==0 (
    echo Prism build failed.
    exit /b %BUILD_RESULT%
)

python sources\Prism\editor\gen_vs_extension.py %*
exit /b %ERRORLEVEL%
