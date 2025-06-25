@echo off
echo Testing transform scale and rotation in Inspector...
echo.

echo 1. Starting GameEditor...
cd /d "f:\Programming\game-engine\build\bin\Release"
start GameEditor.exe

echo.
echo TEST STEPS:
echo.
echo TEST 1: Create entity and test transform editing
echo 1. Wait for GameEditor to start
echo 2. Create a new untitled scene (New Untitled button)
echo 3. Create an entity (Entity menu -> Add Entity)
echo 4. Add a sprite component to the entity (Asset Browser)
echo 5. Select the entity in the scene
echo 6. Open Inspector window (View -> Inspector)
echo 7. In Transform section, try editing:
echo    - Scale values (default 1.0, 1.0)
echo    - Rotation value (default 0.0)
echo 8. Check console for debug output
echo.
echo Expected behavior:
echo - Modifying scale should resize the sprite visually
echo - Modifying rotation should rotate the sprite visually
echo - Console should show "Transform Scale updated: ..." when scale changes
echo - Console should show "Transform Rotation updated: ..." when rotation changes
echo - Console should show "Rendering Entity X - Scale: ... Rotation: ..." during rendering
echo.
echo If scale/rotation don't work:
echo - Check if debug messages appear in console
echo - Verify that rendering system receives the correct values
echo - Test if position changes work (as a control)
echo.
echo Press any key to continue...
pause >nul
