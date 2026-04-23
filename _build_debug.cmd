@echo off
setlocal
cd /d "%~dp0"
cmake --build build --config Debug %*
exit /b %errorlevel%
