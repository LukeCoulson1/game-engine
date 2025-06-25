@echo off
echo Testing sprite component texture updates in node editor...
echo.
echo Instructions:
echo 1. Create an entity in the scene
echo 2. Select the entity
echo 3. In the node editor, create a Sprite Component node
echo 4. Connect the Sprite Component to the Entity node
echo 5. In the Sprite Component node, click "Select Texture" and choose an image
echo 6. Check that the texture appears in the scene window immediately
echo 7. Check that the inspector shows the sprite component with the correct texture
echo.
echo Press any key to start the editor...
pause > nul

cd /d "f:\Programming\game-engine\build\bin\Release"
start GameEditor.exe

echo.
echo Test completed!
echo.
echo Expected behavior:
echo - When you select a texture in the sprite node, it should immediately appear in the scene
echo - The inspector should show the sprite component with the texture path
echo - The scene should refresh automatically without needing manual refresh
echo.
pause
