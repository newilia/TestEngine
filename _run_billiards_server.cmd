@echo off
setlocal
cd /d "%~dp0"
set "PYTHONPATH=%CD%\server"
python -m pip install -r server\requirements.txt -q
if errorlevel 1 (
	echo pip install failed
	pause
	exit /b 1
)
python -m billiards_server --host 127.0.0.1 --port 7777 %*
set "EC=%errorlevel%"
if %EC% neq 0 pause
exit /b %EC%
