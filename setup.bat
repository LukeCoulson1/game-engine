@echo off
echo Setting up 2D Game Engine Development Environment...
echo.

REM Check if vcpkg directory exists
if exist "C:\vcpkg" (
    echo vcpkg found at C:\vcpkg
    set "VCPKG_ROOT=C:\vcpkg"
    goto :install_deps
)

if exist "vcpkg" (
    echo vcpkg found in current directory
    set "VCPKG_ROOT=%CD%\vcpkg"
    goto :install_deps
)

echo vcpkg not found. Installing vcpkg...
echo.

REM Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
if errorlevel 1 (
    echo Failed to clone vcpkg. Please ensure git is installed.
    pause
    exit /b 1
)

cd vcpkg

REM Bootstrap vcpkg
echo Bootstrapping vcpkg...
call bootstrap-vcpkg.bat
if errorlevel 1 (
    echo Failed to bootstrap vcpkg.
    pause
    exit /b 1
)

REM Integrate vcpkg with Visual Studio
echo Integrating vcpkg with Visual Studio...
vcpkg integrate install

cd ..
set "VCPKG_ROOT=%CD%\vcpkg"

:install_deps
echo.
echo Installing required packages...
echo This may take several minutes...
echo.

REM Install required packages
"%VCPKG_ROOT%\vcpkg" install sdl2:x64-windows
"%VCPKG_ROOT%\vcpkg" install sdl2-image:x64-windows
"%VCPKG_ROOT%\vcpkg" install sdl2-mixer:x64-windows
"%VCPKG_ROOT%\vcpkg" install sdl2-ttf:x64-windows
"%VCPKG_ROOT%\vcpkg" install glm:x64-windows
"%VCPKG_ROOT%\vcpkg" install nlohmann-json:x64-windows

echo.
echo Setting up ImGui...

REM Create external directory if it doesn't exist
if not exist "external" mkdir external
if not exist "external\imgui" mkdir "external\imgui"

REM Check if ImGui is already downloaded
if exist "external\imgui\imgui.h" (
    echo ImGui already exists.
) else (
    echo Please download Dear ImGui from: https://github.com/ocornut/imgui/releases
    echo Extract the contents to: external\imgui\
    echo.
    echo Required files:
    echo - imgui.h, imgui.cpp
    echo - imgui_demo.cpp, imgui_draw.cpp
    echo - imgui_tables.cpp, imgui_widgets.cpp
    echo - backends\imgui_impl_sdl2.h/cpp
    echo - backends\imgui_impl_sdlrenderer2.h/cpp
    echo.
    pause
)

echo.
echo Setting environment variable...
setx VCPKG_ROOT "%VCPKG_ROOT%" /M

echo.
echo Setup complete!
echo.
echo VCPKG_ROOT has been set to: %VCPKG_ROOT%
echo.
echo Please restart your command prompt and run: build.bat
echo.
pause
