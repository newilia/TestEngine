@echo off
setlocal
cd /d "%~dp0"
set "EXE=build\bin\Debug\TestEngine.exe"
if not exist "%EXE%" (
  echo Не найден: %EXE%
  echo Сначала соберите Debug: cmake --build build --config Debug
  exit /b 1
)
"%EXE%" %*
exit /b %errorlevel%
