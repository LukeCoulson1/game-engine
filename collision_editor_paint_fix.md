# Collision Editor Paint Fix

## Issue Fixed
The collision editor's paintbrush tool was causing the window to be dragged around instead of painting the collision mask when left-clicking and dragging.

## Root Cause
The previous implementation tried to prevent window dragging by setting `ImGui::SetMouseCursor(ImGuiMouseCursor_None)`, but this was insufficient. The mouse input was still being processed by ImGui's window system for dragging operations.

## Solution Applied
Replaced the direct mouse input handling with `ImGui::InvisibleButton()` to properly capture mouse input over the sprite area:

### Key Changes in `handlePaintbrushTool()`:
1. **Proper Input Capture**: Used `ImGui::InvisibleButton()` to create an invisible interactive area exactly over the sprite bounds
2. **Active State Checking**: Only paint when `ImGui::IsItemActive()` and mouse is down, ensuring proper input capture
3. **Button Positioning**: Set the invisible button position to exactly match the sprite area using `ImGui::SetCursorScreenPos()`

### Code Flow:
```cpp
// Create invisible button over sprite area
ImGui::SetCursorScreenPos(spriteMin);
ImGui::InvisibleButton("##CollisionPaintArea", 
                      ImVec2(editorWidth, editorHeight), 
                      ImGuiButtonFlags_MouseButtonLeft);

// Only paint when button is active
if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
    // Paint collision mask...
}
```

## Testing Instructions
1. Launch GameEditor.exe
2. Create a new scene or load existing scene
3. Select an entity with a sprite component
4. Open Tools → Collision Editor
5. Click on the Paintbrush tool in the collision editor
6. Left-click and drag over the sprite area - should paint collision mask without moving the window
7. Use mouse wheel to zoom in/out within the collision editor
8. Test both paint mode and erase mode

## Result
- ✅ Left-click and drag now paints collision mask instead of moving window
- ✅ Mouse wheel zoom still works properly within collision editor
- ✅ Window can still be moved by dragging the title bar or other areas
- ✅ Invisible button prevents interference with normal window operations
