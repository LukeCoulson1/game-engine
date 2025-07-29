# Entity Drag Functionality Implementation

## Overview
The Scene Window now supports clicking and dragging entities around with the mouse. This feature provides an intuitive way to position entities in the scene while maintaining clear distinction between entity manipulation and camera panning.

## Features

### Entity Dragging
- **Left Click**: Select an entity
- **Left Click + Drag**: Move the selected entity smoothly
- **Visual Feedback**: Dragged entities show a green border instead of the normal orange selection border
- **Drag Threshold**: Small mouse movements (< 5 pixels) won't trigger dragging, preventing accidental entity movement

### Camera Panning (Unchanged)
- **Right Click + Drag**: Pan the camera
- **Middle Click + Drag**: Pan the camera
- **Mouse Wheel**: Zoom in/out with mouse-centered zooming

## Implementation Details

### New State Variables (SceneWindow.h)
```cpp
// Entity dragging state
bool m_isDraggingEntity = false;
EntityID m_draggedEntity = 0;
Vector2 m_entityDragOffset{0, 0}; // Offset from entity center to mouse position
```

### Mouse Input Handling (SceneWindow.cpp)
The `handleInput()` method now:

1. **On Left Mouse Click**: 
   - Finds the entity under the mouse cursor
   - Calculates the offset from entity center to mouse position
   - Prepares for potential dragging

2. **During Mouse Movement**:
   - Checks if mouse has moved beyond drag threshold
   - Starts entity dragging if an entity was clicked
   - Falls back to camera panning if no entity was clicked

3. **While Dragging Entity**:
   - Updates entity position in real-time
   - Maintains the original click offset for smooth dragging
   - Marks the scene as dirty for proper save state

4. **On Mouse Release**:
   - Completes entity selection only if no dragging occurred
   - Resets all drag states

### Visual Feedback
The `renderSceneContent()` method provides visual feedback:

- **Selected Entity**: Orange border (unchanged)
- **Dragged Entity**: Green border with thicker line
- **Entity Dots**: Entities without sprites show larger green dots when being dragged

### Entity Detection
The system supports accurate entity detection for:
- **Sprite Entities**: Uses actual texture dimensions with proper scaling and rotation
- **Rotated Sprites**: Performs proper rotated rectangle collision detection
- **Non-Sprite Entities**: Uses small circular hit areas (8-pixel radius)

## Usage Instructions

### For Users
1. **Select an Entity**: Left-click on any entity in the Scene Window
2. **Drag an Entity**: Left-click and drag to move the entity smoothly
3. **Pan Camera**: Use right-click or middle-click to pan the camera view
4. **Zoom**: Use mouse wheel to zoom in/out

### Visual Cues
- **Orange Border**: Entity is selected
- **Green Border**: Entity is being dragged
- **Thicker Border**: More prominent feedback during dragging
- **Larger Dots**: Non-sprite entities appear larger when being dragged

## Technical Benefits

1. **Smooth Interaction**: Real-time position updates during dragging
2. **Precise Control**: Maintains click offset for accurate positioning
3. **Clear Distinction**: Separate handling for entity dragging vs camera panning
4. **Robust Detection**: Handles entities with and without sprites, rotated sprites
5. **Proper State Management**: Marks scene as dirty when entities are moved

## Code Files Modified

- `src/editor/SceneWindow.h`: Added entity dragging state variables
- `src/editor/SceneWindow.cpp`: 
  - Enhanced `handleInput()` with entity dragging logic
  - Updated `renderSceneContent()` with drag visual feedback

## Testing

Use the `test_entity_drag.bat` script to validate the functionality:
1. Creates/opens a scene with entities
2. Tests entity selection and dragging
3. Verifies camera panning still works
4. Confirms proper visual feedback

The implementation maintains full compatibility with existing camera controls while adding intuitive entity manipulation capabilities.
