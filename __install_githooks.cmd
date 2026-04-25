@echo off
REM Point Git at versioned hooks in githooks\ (run once per clone).
cd /d "%~dp0"
git config core.hooksPath githooks
if errorlevel 1 exit /b 1
echo core.hooksPath=githooks
