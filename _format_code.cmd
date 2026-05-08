@echo off
setlocal EnableExtensions
cd /d "%~dp0"

if "%CLANG_FORMAT%"=="" set "CLANG_FORMAT=clang-format"
where "%CLANG_FORMAT%" >nul 2>&1
if errorlevel 1 (
	echo error: %CLANG_FORMAT% not found in PATH
	set "EC=1"
	goto :end
)

set "EC=0"
for /r "src" %%F in (*.c *.h *.cc *.cpp *.cxx *.cppm *.hpp *.hh *.hxx *.inl *.inc) do if exist "%%F" (
	"%CLANG_FORMAT%" -i --style=file "%%F"
	if errorlevel 1 (
		echo error: clang-format failed for %%F
		set "EC=1"
		goto :end
	)
)

:end
if %EC% neq 0 pause
exit /b %EC%
