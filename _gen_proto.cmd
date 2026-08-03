@echo off
setlocal
cd /d "%~dp0"
call tools\gen_proto.cmd
set "EC=%errorlevel%"
if %EC% neq 0 pause
exit /b %EC%
