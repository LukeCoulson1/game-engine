@echo off
echo 🎨 Custom Tile Asset Replacer
echo ==============================
echo.

if "%1"=="mappings" goto :show_mappings
if "%1"=="replace" goto :replace_tile  
if "%1"=="validate" goto :validate
if "%1"=="show" goto :show_mappings

:show_mappings
echo 📋 Current Tile Mappings:
echo Default Theme (uses your existing assets):
echo   - Wall tiles → wall.png (your asset)
echo   - Floor tiles → ground.png (your asset)
echo   - Buildings → wall.png (your asset)  
echo   - Roads → ground.png (your asset)
echo.
echo Dungeon Theme:
echo   - Walls → wall.png (your asset)
echo   - Floors → ground.png (your asset)
echo   - Doors → dungeon_door.png (generated)
echo.
echo City Theme:
echo   - Roads → city_road.png (generated)
echo   - Houses → city_house.png (generated)
echo   - Shops → city_shop.png (generated)
echo.
echo Terrain Theme:
echo   - Grass → terrain_grass.png (generated)
echo   - Water → terrain_water.png (generated)
echo   - Stone → terrain_stone.png (generated)
echo   - Trees → terrain_tree.png (generated)
goto :end

:replace_tile
echo 📂 Available asset files:
if exist "assets\*.png" (
    dir /b assets\*.png
) else (
    echo No PNG files found in assets\
)
echo.
if exist "assets\tiles\*.png" (
    dir /b assets\tiles\*.png  
) else (
    echo No PNG files found in assets\tiles\
)
echo.
echo To replace a procedural tile with your own image:
echo 1. Copy your 32x32 PNG image to assets\tiles\
echo 2. Name it according to the tile type you want to replace
echo 3. Or update the sprite mappings in TileSpriteManager
echo.
echo Example commands:
echo   copy my_wall.png assets\tiles\dungeon_wall.png
echo   copy my_grass.png assets\tiles\terrain_grass.png  
echo   copy my_house.png assets\tiles\city_house.png
goto :end

:validate
echo 🔍 Validating tile assets...
if exist "assets\wall.png" (
    echo ✅ Found: assets\wall.png
) else (
    echo ❌ Not found: assets\wall.png
)

if exist "assets\ground.png" (
    echo ✅ Found: assets\ground.png
) else (
    echo ❌ Not found: assets\ground.png
)

if exist "assets\tiles\dungeon_door.png" (
    echo ✅ Found: assets\tiles\dungeon_door.png
) else (
    echo ❌ Not found: assets\tiles\dungeon_door.png
)

if exist "assets\tiles\city_house.png" (
    echo ✅ Found: assets\tiles\city_house.png
) else (
    echo ❌ Not found: assets\tiles\city_house.png
)

if exist "assets\tiles\terrain_grass.png" (
    echo ✅ Found: assets\tiles\terrain_grass.png
) else (
    echo ❌ Not found: assets\tiles\terrain_grass.png
)

echo.
echo 💡 Tip: Ensure images are 32x32 pixels for best results
goto :end

:help
echo Usage: %0 [mappings^|replace^|validate]
echo.
echo Commands:
echo   mappings - Show current tile-to-sprite mappings
echo   replace  - Instructions for replacing tiles with your images  
echo   validate - Check if tile assets exist
echo.
goto :show_mappings

:end
if "%1"=="" goto :help
pause
