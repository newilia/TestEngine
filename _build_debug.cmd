@echo off
setlocal
cd /d "%~dp0"
cmake --build build --config Debug %*
set "EC=%errorlevel%"
if %EC% neq 0 pause
exit /b %EC%
