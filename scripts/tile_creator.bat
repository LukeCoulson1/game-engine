@echo off
setlocal enabledelayedexpansion

echo 🎨 Tile Image Creation Helper
echo =============================
echo.

if "%1"=="create" goto :create_tile
if "%1"=="list" goto :list_tiles
if "%1"=="validate" goto :validate_tiles

:menu
echo Choose an option:
echo [1] Create a new tile image
echo [2] List existing tiles  
echo [3] Validate tile assets
echo [4] Show tile requirements
echo [5] Exit
echo.
set /p choice="Enter choice (1-5): "

if "%choice%"=="1" goto :create_tile
if "%choice%"=="2" goto :list_tiles
if "%choice%"=="3" goto :validate_tiles
if "%choice%"=="4" goto :show_requirements
if "%choice%"=="5" goto :end
goto :menu

:create_tile
echo.
echo 🎨 Creating a new tile image...
echo.
echo Available tile types:
echo [1] Wall       [8] Tree
echo [2] Floor      [9] Building  
echo [3] Door       [10] Road
echo [4] Water      [11] House
echo [5] Grass      [12] Shop
echo [6] Stone      [13] Entrance
echo [7] Tree       [14] Exit
echo.
set /p tiletype="Enter tile type number (1-14): "

set "tilenames[1]=wall"
set "tilenames[2]=floor"
set "tilenames[3]=door"
set "tilenames[4]=water"
set "tilenames[5]=grass"
set "tilenames[6]=stone"
set "tilenames[7]=tree"
set "tilenames[8]=tree"
set "tilenames[9]=building"
set "tilenames[10]=road"
set "tilenames[11]=house"
set "tilenames[12]=shop"
set "tilenames[13]=entrance"
set "tilenames[14]=exit"

set tilename=!tilenames[%tiletype%]!

if "%tilename%"=="" (
    echo Invalid tile type!
    goto :create_tile
)

echo.
echo Theme prefixes:
echo [1] None (generic)
echo [2] dungeon_
echo [3] city_
echo [4] terrain_
echo [5] custom_
echo.
set /p themenum="Enter theme number (1-5): "

set "themes[1]="
set "themes[2]=dungeon_"
set "themes[3]=city_"
set "themes[4]=terrain_"
set "themes[5]=custom_"

set themeprefix=!themes[%themenum%]!

set filename=%themeprefix%%tilename%.png

echo.
echo Creating template for: %filename%
echo.

if not exist "assets\tiles" mkdir "assets\tiles"

powershell -Command "
Add-Type -AssemblyName System.Drawing
$bitmap = New-Object System.Drawing.Bitmap(32, 32)
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)

# Create a simple colored square based on tile type
$color = switch('%tilename%') {
    'wall' { [System.Drawing.Color]::Gray }
    'floor' { [System.Drawing.Color]::LightGray }
    'door' { [System.Drawing.Color]::Brown }
    'water' { [System.Drawing.Color]::Blue }
    'grass' { [System.Drawing.Color]::Green }
    'stone' { [System.Drawing.Color]::DarkGray }
    'tree' { [System.Drawing.Color]::DarkGreen }
    'building' { [System.Drawing.Color]::LightBlue }
    'road' { [System.Drawing.Color]::DarkSlateGray }
    'house' { [System.Drawing.Color]::SandyBrown }
    'shop' { [System.Drawing.Color]::Orange }
    'entrance' { [System.Drawing.Color]::Gold }
    'exit' { [System.Drawing.Color]::Red }
    default { [System.Drawing.Color]::White }
}

$brush = New-Object System.Drawing.SolidBrush($color)
$graphics.FillRectangle($brush, 0, 0, 32, 32)

# Add a border
$pen = New-Object System.Drawing.Pen([System.Drawing.Color]::Black, 1)
$graphics.DrawRectangle($pen, 0, 0, 31, 31)

$bitmap.Save('assets\tiles\%filename%', [System.Drawing.Imaging.ImageFormat]::Png)

$graphics.Dispose()
$brush.Dispose()
$pen.Dispose()
$bitmap.Dispose()
"

if exist "assets\tiles\%filename%" (
    echo ✅ Created: assets\tiles\%filename%
    echo 💡 You can now edit this file in your image editor
    echo 📏 Remember to keep it 32x32 pixels
) else (
    echo ❌ Failed to create tile image
)

echo.
goto :menu

:list_tiles
echo.
echo 📂 Existing tile images:
echo.
if exist "assets\*.png" (
    echo Main assets:
    dir /b "assets\*.png"
    echo.
)
if exist "assets\tiles\*.png" (
    echo Tile assets:
    dir /b "assets\tiles\*.png"
) else (
    echo No tile images found in assets\tiles\
)
echo.
goto :menu

:validate_tiles
echo.
echo 🔍 Validating tile assets...
echo.

set "required_assets=wall.png ground.png player.png enemy.png coin.png"
echo Main assets:
for %%f in (%required_assets%) do (
    if exist "assets\%%f" (
        echo ✅ Found: assets\%%f
    ) else (
        echo ❌ Missing: assets\%%f
    )
)

echo.
echo Tile assets:
set "tile_assets=dungeon_wall.png dungeon_floor.png city_house.png terrain_grass.png"
for %%f in (%tile_assets%) do (
    if exist "assets\tiles\%%f" (
        echo ✅ Found: assets\tiles\%%f
    ) else (
        echo ⚠️  Optional: assets\tiles\%%f
    )
)
echo.
goto :menu

:show_requirements
echo.
echo 📋 Tile Image Requirements:
echo.
echo 📏 Dimensions: 32x32 pixels exactly
echo 🎨 Format: PNG with transparency support
echo 📁 Location: assets\tiles\ folder
echo 🎯 Naming: descriptive names (e.g., 'brick_wall.png')
echo.
echo 🎨 Recommended themes:
echo   • dungeon_ - Medieval/fantasy dungeon tiles
echo   • city_ - Modern urban environment tiles  
echo   • terrain_ - Natural outdoor environment tiles
echo   • custom_ - Your unique artistic style
echo.
echo 💡 Tips:
echo   • Use consistent art style across all tiles
echo   • Consider using transparency for non-square objects
echo   • Test tiles in small generated maps first
echo   • Keep a backup of working tile sets
echo.
goto :menu

:end
echo.
echo Thanks for using the Tile Image Creation Helper!
pause
