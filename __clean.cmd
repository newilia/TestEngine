@echo off
setlocal
cd /d "%~dp0"

if exist "build" (
  rmdir /s /q "build"
  if exist "build" (
    echo.
    echo Не удалось удалить build: файлы заняты ^(закройте Visual Studio и остальные, что держат build^).
    pause
    exit /b 1
  )
)

if exist "src\Codegen" (
  rmdir /s /q "src\Codegen"
  if exist "src\Codegen" (
    echo.
    echo Не удалось удалить src\Codegen: файлы заняты.
    pause
    exit /b 1
  )
)

exit /b 0
