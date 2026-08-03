@echo off
setlocal
cd /d "%~dp0\.."
set "ROOT=%CD%"
set "PROTO_DIR=%ROOT%\proto"
set "PY_OUT=%ROOT%\server\billiards_server\generated"
set "CPP_OUT=%ROOT%\src\proto_generated"

if not exist "%PY_OUT%" mkdir "%PY_OUT%"

set "PROTOC_CPP="
for %%P in (
	"%ROOT%\build\bin\Release\protoc.exe"
	"%ROOT%\build\bin\Debug\protoc.exe"
) do (
	if exist %%P (
		set "PROTOC_CPP=%%~P"
		goto :found_cpp_protoc
	)
)

where protoc >nul 2>&1
if %errorlevel% equ 0 (
	for /f "delims=" %%P in ('where protoc') do (
		set "PROTOC_CPP=%%P"
		goto :found_cpp_protoc
	)
)

echo C++ protoc not found. Build Release once or install protoc on PATH.
exit /b 1

:found_cpp_protoc
echo Using C++ protoc: %PROTOC_CPP%
if not exist "%CPP_OUT%" mkdir "%CPP_OUT%"
"%PROTOC_CPP%" -I "%PROTO_DIR%" --cpp_out="%CPP_OUT%" "%PROTO_DIR%\BilliardSession.proto"
if errorlevel 1 exit /b %errorlevel%

echo Generating Python via grpc_tools.protoc (matches pip protobuf runtime)...
python -m pip install "grpcio-tools>=1.68.0" -q
python -m grpc_tools.protoc -I "%PROTO_DIR%" --python_out="%PY_OUT%" "%PROTO_DIR%\BilliardSession.proto"
if errorlevel 1 exit /b %errorlevel%

echo Generated:
echo   %PY_OUT%\BilliardSession_pb2.py
echo   %CPP_OUT%\BilliardSession.pb.h / BilliardSession.pb.cc
exit /b 0
