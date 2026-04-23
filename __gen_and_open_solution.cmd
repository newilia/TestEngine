@echo off

setlocal

cd /d "%~dp0"

if not exist "build" mkdir "build"

cmake -S . -B build -A x64
if errorlevel 1 exit /b %errorlevel%

if exist "build\TestEngine.slnx" start "" "%~dp0build\TestEngine.slnx"

exit /b 0

