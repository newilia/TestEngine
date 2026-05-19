@echo off
REM MSVC + Ninja for _cmake_ninja.cmd (clangd compile_commands only).
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
	echo vswhere not found. Install Visual Studio with the C++ workload.
	exit /b 1
)

for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL (
	echo Visual Studio with C++ tools not found.
	exit /b 1
)

set "CMAKE_MAKE_PROGRAM=%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
if not exist "%CMAKE_MAKE_PROGRAM%" (
	echo Ninja not found: %CMAKE_MAKE_PROGRAM%
	exit /b 1
)

call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
