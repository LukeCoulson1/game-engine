@echo off
echo Building 2D Game Engine (Manual Dependencies)...
echo.
echo This build script assumes you have manually installed:
echo - SDL2 development libraries
echo - Visual Studio 2022
echo.

REM Create build directory
if not exist build mkdir build
cd build

REM Try to configure without vcpkg first
echo Attempting to configure without vcpkg...
cmake .. -G "Visual Studio 17 2022"

if errorlevel 1 (
    echo.
    echo Manual build failed. This likely means SDL2 is not installed or not found.
    echo.
    echo Please either:
    echo 1. Run setup.bat to install vcpkg and dependencies automatically
    echo 2. Manually install SDL2 development libraries and ensure they're in your PATH
    echo 3. Use vcpkg by setting VCPKG_ROOT and running build.bat
    echo.
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
