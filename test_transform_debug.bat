@echo off
echo Testing Transform Scale and Rotation Fix...
echo.

echo Starting GameEditor with debug output...
cd /d "f:\Programming\game-engine\build\bin\Release"
start GameEditor.exe

echo.
echo DEBUG TEST PROTOCOL:
echo.
echo 1. CREATE TEST ENTITY:
echo    - New Untitled Scene
echo    - Use procedural generation to create entities
echo    - Or manually create entity via Entity menu
echo.
echo 2. SELECT AND INSPECT:
echo    - Click on an entity in the scene
echo    - Open Inspector (View -> Inspector)
echo    - Verify Transform section is visible
echo.
echo 3. TEST SCALE (watch console):
echo    - Change Scale from 1.0, 1.0 to 2.0, 2.0
echo    - Look for: "DEBUG: Scale changed to 2.00, 2.00"
echo    - Look for: "RENDER DEBUG: Entity X - Scale: 2.00, 2.00..."
echo    - Entity should visually become larger
echo.
echo 4. TEST ROTATION (watch console):
echo    - Change Rotation from 0 to 45
echo    - Look for: "DEBUG: Rotation changed to 45.0"
echo    - Look for: "RENDER DEBUG: Entity X - Rotation: 45.0..."
echo    - Entity should visually rotate
echo.
echo 5. VERIFY VISUAL CHANGES:
echo    - Scale 2.0: Entity 2x larger
echo    - Scale 0.5: Entity half size
echo    - Rotation 45: Entity rotated 45 degrees
echo    - Rotation 90: Entity rotated 90 degrees
echo.
echo EXPECTED CONSOLE OUTPUT:
echo - "DEBUG: Scale changed to X.XX, Y.YY" (when changing scale)
echo - "DEBUG: Rotation changed to X.X" (when changing rotation)
echo - "RENDER DEBUG: Entity N - Scale: X.XX,Y.YY Rotation: Z.Z..."
echo - "  DstRect: X.X,Y.Y,W.W,H.H" (showing calculated destination rectangle)
echo.
echo If you see debug messages but no visual changes:
echo - The component system is working
echo - The issue is in SDL rendering or renderer implementation
echo.
echo If you don't see debug messages:
echo - The inspector UI is not properly updating components
echo - Check for ECS system issues
echo.
echo Press any key when ready to continue testing...
pause >nul
