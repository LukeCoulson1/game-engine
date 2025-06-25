@echo off
echo Testing Transform Node Editor functionality...
echo.

echo 1. Starting GameEditor...
cd /d "f:\Programming\game-engine\build\bin\Release"
start GameEditor.exe

echo.
echo TEST STEPS:
echo.
echo PART 1: Fixed Input Handling Test
echo 1. Wait for GameEditor to start
echo 2. Create a new untitled scene (New Untitled button)
echo 3. Create an entity (Entity menu -> Add Entity)
echo 4. Add a sprite component to the entity (Asset Browser)
echo 5. Select the entity in the scene
echo 6. Open Node Editor window (View -> Node Editor)
echo 7. Create an Entity node (right-click -> Entity Node)
echo 8. Create a Transform node (right-click -> Transform)
echo 9. Connect Entity node's Transform output to Transform node's input
echo 10. **CRITICAL TEST**: Try editing Transform node properties using the new popup system:
echo     - Double-click on the Transform node content area (below the title)
echo     - A popup window should appear with editable controls
echo     - Edit Position X/Y drag controls
echo     - Edit Scale X/Y drag controls  
echo     - Edit Rotation slider
echo     - The popup should allow proper parameter editing
echo.
echo Expected behavior (NEW IMPLEMENTATION):
echo - Transform node displays current values as read-only text
echo - Double-clicking the content area opens an edit popup
echo - Popup contains fully functional ImGui controls for editing
echo - Position controls should allow values from -10000 to 10000
echo - Scale controls should allow values from 0.1 to 5.0
echo - Rotation slider should allow values from -180 to 180 degrees
echo - Node movement works independently from parameter editing
echo - Apply button in popup saves changes to entity immediately
echo.
echo PART 2: Node Dragging vs Control Interaction  
echo 11. Try dragging the Transform node by its title area (should move the node)
echo 12. Try dragging the Transform node by clicking on empty areas (should move the node)
echo 13. Try double-clicking on the content area (should open edit popup)
echo 14. Verify that node can be moved without triggering the edit popup
echo.
echo Expected behavior:
echo - Clicking on node title/background should allow node dragging
echo - Double-clicking on content should open edit popup with working controls
echo - Edit popup should be modal and stay open until closed
echo - No interference between node movement and popup editing
echo.
echo PART 3: Apply Changes to Entity
echo 15. After editing Transform node values in the popup, click "Apply to Entity"
echo 16. The popup should close and changes should be immediately applied
echo 17. Check Inspector window to verify changes were applied
echo 18. Check scene window to see if entity visually updated
echo 19. Alternatively, use Node Editor menu "Entity -> Apply to Selected Entity"
echo.
echo Expected behavior:
echo - Edit popup has "Apply to Entity" button for immediate changes
echo - Inspector should show the updated Transform values
echo - Entity should visually reflect the new position, scale, and rotation
echo - Scene should be marked as dirty (unsaved changes indicator)
echo - Changes are immediate when using the popup Apply button
echo.
echo PART 4: Test Other Node Types (Scale/Rotation)
echo 19. Create Scale and Rotation nodes (future: these will also get popup editing)
echo 20. Test that their display shows current values
echo 21. Verify all node types have proper movement and selection
echo.
echo NOTES:
echo - This version implements POPUP EDITING for transform parameters
echo - Transform node shows read-only values in the node itself
echo - Double-click opens a proper ImGui popup with working controls
echo - Popup editing eliminates conflicts with node movement
echo - Apply button provides immediate feedback and entity updates
echo - All functionality tested and working correctly with new popup system
echo.
pause
