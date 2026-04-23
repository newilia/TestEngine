@echo off
setlocal
cd /d "%~dp0"
set "EXE=build\bin\Release\TestEngine.exe"
if not exist "%EXE%" (
  echo Не найден: %EXE%
  echo Сначала соберите Release: cmake --build build --config Release
  pause
  exit /b 1
)
"%EXE%" %*
set "EC=%errorlevel%"
if %EC% neq 0 pause
exit /b %EC%
