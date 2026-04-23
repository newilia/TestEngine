@echo off
setlocal
cd /d "%~dp0"
set "EXE=build\bin\Debug\TestEngine.exe"
if not exist "%EXE%" (
  echo Не найден: %EXE%
  echo Сначала соберите Debug: cmake --build build --config Debug
  pause
  exit /b 1
)
"%EXE%" %*
set "EC=%errorlevel%"
if %EC% neq 0 pause
exit /b %EC%
