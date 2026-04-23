@echo off
setlocal
cd /d "%~dp0"
if not exist "build" exit /b 0
rmdir /s /q "build"
if exist "build" (
  echo.
  echo Не удалось удалить build: файлы заняты ^(закройте Visual Studio и остальные, что держат build^).
  pause
  exit /b 1
)
exit /b 0
