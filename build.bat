@echo off
echo Building 2D Game Engine...

REM Check if VCPKG_ROOT is set
if "%VCPKG_ROOT%"=="" (
    echo ERROR: VCPKG_ROOT environment variable is not set!
    echo Please set it to your vcpkg installation directory.
    echo Example: set VCPKG_ROOT=C:\vcpkg
    pause
    exit /b 1
)

REM Check if vcpkg toolchain file exists
if not exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
    echo ERROR: vcpkg toolchain file not found at: %VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
    echo Please verify your VCPKG_ROOT path and that vcpkg is properly installed.
    pause
    exit /b 1
)

echo Using vcpkg at: %VCPKG_ROOT%

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"

if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release

if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build complete!
echo.
echo To run the editor: bin\Release\GameEditor.exe
echo To run the runtime: bin\Release\GameRuntime.exe

pause
