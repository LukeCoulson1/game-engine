@echo off
echo Testing FIXED Player Controller functionality in Game Logic Window...
echo.
echo FIXES APPLIED:
echo - Removed automatic dance animation
echo - Added SDL_PumpEvents() for proper input handling
echo - Added debug logging for key presses
echo - Updated default code template
echo.
echo Instructions:
echo 1. Open the Game Editor
echo 2. Create an entity in the Scene Window (right-click and "Create Entity")
echo 3. Select the entity and go to the Inspector window
echo 4. Click "Add Player Controller" button in the Inspector
echo 5. Make sure the entity also has a Transform component
echo 6. Open the Game Logic Window
echo 7. Press F5 to start the game logic runtime
echo 8. Try using WASD keys to move the entity!
echo 9. Check the console in Game Logic Window for key press debug messages
echo 10. Hold Shift while moving to run (entity should turn reddish in demo template)
echo 11. Press Shift+F5 to stop the runtime
echo.
echo Expected behavior:
echo - NO automatic dance animation should occur
echo - Entity should move with WASD keys
echo - Console should show debug messages when keys are pressed
echo - Movement should be visible in real-time in the Scene Window
echo - Entity should change color when running (if using demo template)
echo.
echo Press any key to start the editor...
pause > nul

cd /d "f:\Programming\game-engine\build\bin\Release"
start GameEditor.exe

echo.
echo Test completed!
echo.
echo Troubleshooting:
echo - If entity doesn't move, check console for key press debug messages
echo - Make sure entity has both Transform AND PlayerController components
echo - Try clicking in Scene Window to ensure it has focus for input
echo - Check that F5 was pressed and runtime is actually started
echo.
pause
