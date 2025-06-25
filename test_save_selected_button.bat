@echo off
echo Testing Save Selected button functionality...
echo.

echo 1. Starting GameEditor...
cd /d "f:\Programming\game-engine\build\bin\Release"
start GameEditor.exe

echo.
echo TEST STEPS:
echo.
echo TEST 1: Save As for untitled scenes
echo 1. Wait for GameEditor to start
echo 2. Open Scene Manager (View menu)
echo 3. Click "New Untitled" button to create an untitled scene
echo 4. Select the untitled scene from the list
echo 5. Click "Save Selected" button - should open Save As dialog
echo 6. Enter a filename and click Save
echo 7. Verify the scene is saved and renamed
echo.
echo TEST 2: Regular save for saved scenes
echo 1. Select a scene that already has a filepath
echo 2. Make some changes to the scene (add/remove entities)
echo 3. Click "Save Selected" button - should save directly
echo 4. Verify the scene is saved (check console output)
echo.
echo Expected behavior:
echo - "Save Selected" button enabled when any loaded scene is selected
echo - For untitled scenes: Opens "Save As" dialog
echo - For saved scenes: Saves directly without dialog
echo - Console shows appropriate save messages
echo - Tooltips show helpful information when hovering over button
echo.
echo Press any key to continue...
pause >nul
