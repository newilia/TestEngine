@echo off
REM Configure build-ninja/ for clangd (compile_commands.json). Main build uses build/ + Visual Studio.
setlocal

cd /d "%~dp0"

if not exist "build-ninja" mkdir "build-ninja"

call "%~dp0cmake\WinNinjaEnv.cmd"
if errorlevel 1 goto :fail

cmake -S . -B build-ninja -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM="%CMAKE_MAKE_PROGRAM%"
if errorlevel 1 goto :fail

copy /Y "%~dp0build-ninja\compile_commands.json" "%~dp0compile_commands.json" >nul

echo compile_commands.json: %~dp0build-ninja\compile_commands.json
echo (copy for clangd watcher: %~dp0compile_commands.json)
exit /b 0

:fail
set "EC=%errorlevel%"
pause
exit /b %EC%
