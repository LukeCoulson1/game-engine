# Scene Window Zoom Functionality

## Overview
Added comprehensive zoom functionality to scene windows, allowing users to zoom in and out using mouse wheel and keyboard controls. All visual elements scale appropriately with the zoom level.

## Features Implemented

### 1. Mouse Wheel Zoom
- **Control**: Mouse wheel scroll
- **Behavior**: Scroll up to zoom in, scroll down to zoom out
- **Zoom Factor**: 10% increment per scroll step
- **Implementation**: Real-time zoom level adjustment in `handleInput()`

### 2. Keyboard Shortcuts
- **Zoom In**: `+` or `Numpad +`
- **Zoom Out**: `-` or `Numpad -`
- **Reset Zoom**: `1` key (returns to 1.0x)
- **Context**: Only active when scene viewport is focused

### 3. Menu Controls
- **Location**: View menu in scene window menu bar
- **Options**: 
  - Zoom In
  - Zoom Out
  - Reset Zoom
- **Shortcuts Displayed**: Menu shows corresponding keyboard shortcuts

### 4. Visual Feedback
- **Zoom Display**: Current zoom level shown in scene window header
  - Format: "Camera: (x, y) | Zoom: 1.5x"
- **Quick Reset**: "1:1" button next to camera controls
- **Range Indicator**: Zoom level updates in real-time

### 5. Smart Scaling
All visual elements scale proportionally with zoom:

#### Sprites/Textures
- Base tile size: 32x32 pixels
- Zoom scaling: `tileSize = 32.0f * m_zoomLevel`
- Maintains aspect ratio during scaling

#### Entity Dots
- Base radius: 3.0 pixels
- Zoom scaling: `radius = 3.0f * m_zoomLevel`
- Used for entities without sprites or invisible sprites

#### Grid Overlay
- Base grid size: 32x32 pixels
- Zoom scaling: `gridSize = 32.0f * m_zoomLevel`
- **Smart hiding**: Grid disappears when zoom < 0.5x (prevents visual clutter)

#### Selection Borders
- Scales with entity size
- Maintains 2-pixel border thickness regardless of zoom

### 6. Zoom Constraints
- **Minimum Zoom**: 0.1x (10% of original size)
- **Maximum Zoom**: 10.0x (1000% of original size)
- **Default Zoom**: 1.0x (100% - original size)
- **Bounds Enforcement**: `setZoomLevel()` clamps values to valid range

### 7. Coordinate System Integration
#### Screen to World Conversion
- Zoom affects entity positioning relative to canvas center
- Mouse click detection accounts for zoom level
- Camera panning works correctly at all zoom levels

#### World to Screen Conversion
```cpp
// Apply zoom scaling to screen position relative to canvas center
ImVec2 canvasCenter = ImVec2(canvasPos.x + canvasSize.x / 2, canvasPos.y + canvasSize.y / 2);
screenPos.x = canvasCenter.x + (screenPos.x - canvasSize.x / 2) * m_zoomLevel;
screenPos.y = canvasCenter.y + (screenPos.y - canvasSize.y / 2) * m_zoomLevel;
```

## Implementation Details

### SceneWindow Class Changes
**Header (`SceneWindow.h`)**:
```cpp
// Zoom controls
float getZoomLevel() const { return m_zoomLevel; }
void setZoomLevel(float zoom);
void zoomIn() { setZoomLevel(m_zoomLevel * 1.2f); }
void zoomOut() { setZoomLevel(m_zoomLevel / 1.2f); }
void resetZoom() { setZoomLevel(1.0f); }

// Private members
float m_zoomLevel = 1.0f;
const float m_minZoom = 0.1f;
const float m_maxZoom = 10.0f;
```

**Implementation (`SceneWindow.cpp`)**:
- `setZoomLevel()`: Clamps zoom to valid range
- `handleInput()`: Mouse wheel and keyboard zoom handling
- `renderSceneContent()`: Applies zoom to all rendering
- Menu integration for zoom controls

### User Experience Improvements
1. **Intuitive Controls**: Standard zoom controls (mouse wheel, +/- keys)
2. **Visual Feedback**: Real-time zoom level display
3. **Smooth Operation**: Incremental zoom steps for fine control
4. **Reasonable Limits**: Prevents unusably small/large zoom levels
5. **Context Awareness**: Grid hiding at small zoom levels

## Testing

### Manual Testing Steps
1. Open GameEditor and create a scene window
2. Add some entities with procedural generation
3. Test mouse wheel zoom in both directions
4. Test keyboard shortcuts (+, -, 1)
5. Test View menu zoom options
6. Verify zoom level display updates
7. Verify all elements scale appropriately
8. Test at extreme zoom levels (0.1x, 10.0x)

### Test Script
Run `test_zoom_functionality.bat` for guided testing.

## Future Enhancements
Potential improvements:
- Zoom to fit functionality (auto-zoom to show all entities)
- Zoom to selection (center and zoom on selected entity)
- Smooth zoom animation transitions
- Configurable zoom sensitivity
- Zoom history/bookmarks

## Files Modified
- `src/editor/SceneWindow.h`: Added zoom interface and member variables
- `src/editor/SceneWindow.cpp`: Implemented zoom functionality
- `docs/Scene_Window_Isolation_Summary.md`: Updated documentation

The zoom functionality is now fully integrated and provides a smooth, professional zooming experience in scene windows.
