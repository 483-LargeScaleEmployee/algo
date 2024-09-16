@echo off
setlocal

:: Navigate to the project root
cd /d "%~dp0\.."

:: Create build directory if it doesn't exist
if not exist "build\windows" mkdir "build\windows"

:: Navigate to build directory
cd "build\windows"

:: Generate Visual Studio solution
cmake -G "Visual Studio 16 2019" -A x64 ..\..

:: Build the project
cmake --build . --config Release

:: Check if build was successful
if %ERRORLEVEL% EQU 0 (
    echo Build successful! Executable is in the build\windows\Release directory.
) else (
    echo Build failed. Please check the error messages above.
)

endlocal
