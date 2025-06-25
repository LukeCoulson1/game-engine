@echo off
echo 🖼️ Testing Scene Window Size Persistence
echo ==========================================
echo.
echo This test verifies that scene windows retain their size when closed and reopened.
echo.

echo 📋 Test Steps:
echo 1. Open the Game Editor
echo 2. Create a new scene window (File → New Scene)
echo 3. Resize the scene window to a different size
echo 4. Close the scene window
echo 5. Open a new scene window again
echo 6. Verify the new window opens with the same size as step 3
echo.

echo 🎯 Expected Results:
echo - First scene window opens with default size (800x600)
echo - After resizing, the new size is saved to editor_config.json
echo - When opening a new scene window, it uses the saved size
echo - All scene windows share the same preferred size setting
echo.

echo 🔧 Implementation Details:
echo - ConfigManager stores scene window size in editor_config.json
echo - SceneWindow constructor calls restoreWindowSize() 
echo - Window size changes are detected and saved automatically
echo - Uses ImGui::SetNextWindowSize() with ImGuiCond_FirstUseEver
echo.

echo 🚀 Testing Tips:
echo - Try different window sizes to see persistence
echo - Check editor_config.json for "scene_window.width" and "scene_window.height"
echo - Close/reopen editor to test persistence across sessions
echo.

pause
.\build\bin\Release\GameEditor.exe
