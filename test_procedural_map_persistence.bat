@echo off
echo Testing procedural map persistence in saved scenes...
echo.

echo 1. Starting GameEditor...
cd /d "f:\Programming\game-engine\build\bin\Release"
start GameEditor.exe

echo.
echo TEST STEPS:
echo.
echo TEST 1: Generate and save a procedural map
echo 1. Wait for GameEditor to start
echo 2. Create a new untitled scene (New Untitled button)
echo 3. Generate a procedural map (Procedural menu -> Generate Dungeon)
echo 4. Verify the map appears in the scene window
echo 5. Save the scene using Scene Manager -> Save Selected
echo 6. Give it a name (e.g., "test_procedural.scene")
echo 7. Close the scene window
echo.
echo TEST 2: Load the saved scene and verify procedural map
echo 1. In Scene Manager, select the saved scene
echo 2. Double-click to open it
echo 3. Verify that:
echo    - The procedural map is visible
echo    - All sprites/tiles are rendered correctly
echo    - Entities are present if they were created
echo.
echo Expected behavior:
echo - Procedural maps should be saved with scene data
echo - When loading a scene, procedural maps should be restored
echo - SceneWindow should automatically get the procedural map
echo - Console should show "Restored procedural map to SceneWindow"
echo - Console should show "Loaded procedural map: WxH with N tiles"
echo.
echo Press any key to continue...
pause >nul
