@echo off
setlocal
cd /d "%~dp0"
cmake --build build --config Release %*
set "EC=%errorlevel%"
if %EC% neq 0 pause
exit /b %EC%
