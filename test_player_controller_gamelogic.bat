@echo off
echo Testing Player Controller functionality in Game Logic Window...
echo.
echo Instructions:
echo 1. Open the Game Editor
echo 2. Create an entity in the Scene Window (right-click and "Create Entity")
echo 3. Select the entity and go to the Inspector window
echo 4. Click "Add Player Controller" button in the Inspector
echo 5. Open the Game Logic Window (should be one of the windows or in menu)
echo 6. Press F5 to start the game logic runtime
echo 7. Try using WASD keys to move the entity!
echo 8. Hold Shift while moving to run (entity should turn reddish)
echo 9. Press Space to jump (if physics components are added)
echo 10. Press Shift+F5 to stop the runtime
echo.
echo Expected behavior:
echo - Entity should move with WASD keys
echo - Entity should change color when running (Shift+WASD)
echo - Movement should be visible in real-time in the Scene Window
echo - Console in Game Logic Window should show when runtime starts/stops
echo.
echo Press any key to start the editor...
pause > nul

cd /d "f:\Programming\game-engine\build\bin\Release"
start GameEditor.exe

echo.
echo Test completed!
echo.
echo Notes:
echo - The PlayerSystem is now integrated into the GameLogicWindow
echo - Input handling works through SDL keyboard state
echo - The system automatically processes entities with PlayerController components
echo - You can add custom logic in the code editor while the runtime is active
echo.
pause
