# Mouse-Centered Zoom Feature

## Overview
Implemented intuitive mouse-centered zoom functionality that zooms towards the cursor position instead of the viewport center, providing a more natural and precise zooming experience.

## Feature Description

### Before (Center-Based Zoom)
- Mouse wheel zoom always centered on the viewport center
- User had to manually pan to the desired area before zooming
- Less intuitive workflow for detailed inspection of specific areas

### After (Mouse-Centered Zoom)
- Mouse wheel zoom centers on the cursor position
- The point under the cursor remains stationary during zoom operations
- Allows precise inspection of any area without manual panning

## Technical Implementation

### Algorithm
The mouse-centered zoom uses the following approach:

1. **Capture Mouse Position**: Get current mouse coordinates in screen space
2. **Convert to World Space**: Transform mouse position to world coordinates before zoom
3. **Apply Zoom**: Update the zoom level with min/max clamping
4. **Recalculate World Position**: Convert mouse position to world coordinates after zoom
5. **Adjust Camera**: Offset camera position to keep the mouse world position constant

### Code Implementation
```cpp
// Mouse wheel zoom towards cursor
if (io.MouseWheel != 0.0f) {
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    
    // Convert mouse position to world coordinates before zoom
    ImVec2 canvasCenter = ImVec2(canvasPos.x + canvasSize.x / 2, canvasPos.y + canvasSize.y / 2);
    Vector2 worldMousePos;
    worldMousePos.x = ((mousePos.x - canvasCenter.x) / m_zoomLevel) + m_cameraPosition.x;
    worldMousePos.y = ((mousePos.y - canvasCenter.y) / m_zoomLevel) + m_cameraPosition.y;
    
    // Apply zoom with limits
    float zoomFactor = 1.0f + (io.MouseWheel * 0.1f);
    float newZoomLevel = std::max(m_minZoom, std::min(m_maxZoom, m_zoomLevel * zoomFactor));
    
    // Convert mouse position to world coordinates after zoom
    Vector2 newWorldMousePos;
    newWorldMousePos.x = ((mousePos.x - canvasCenter.x) / newZoomLevel) + m_cameraPosition.x;
    newWorldMousePos.y = ((mousePos.y - canvasCenter.y) / newZoomLevel) + m_cameraPosition.y;
    
    // Adjust camera position to keep the mouse world position the same
    m_cameraPosition.x += (worldMousePos.x - newWorldMousePos.x);
    m_cameraPosition.y += (worldMousePos.y - newWorldMousePos.y);
    
    // Set the new zoom level
    m_zoomLevel = newZoomLevel;
}
```

## Coordinate System Integration

### Screen to World Conversion
The implementation uses the same coordinate transformation as the rest of the engine:
```cpp
worldPos.x = ((screenPos.x - canvasCenter.x) / zoomLevel) + cameraPosition.x;
worldPos.y = ((screenPos.y - canvasCenter.y) / zoomLevel) + cameraPosition.y;
```

### Camera Adjustment
The camera offset calculation ensures the world point under the cursor remains fixed:
```cpp
cameraOffset = worldPosBefore - worldPosAfter;
```

## User Experience Benefits

### 1. Intuitive Navigation
- **Natural Feel**: Matches behavior of modern image viewers, CAD software, and map applications
- **Precise Control**: Users can zoom directly to areas of interest
- **Reduced Steps**: No need to pan-then-zoom workflow

### 2. Workflow Improvements
- **Detail Inspection**: Easily zoom into specific entities or tile areas
- **Level Design**: Precise placement and editing of game elements
- **Large Maps**: Efficient navigation of procedurally generated content

### 3. Consistent Behavior
- **Zoom Limits**: Respects min/max zoom boundaries (0.1x to 10x)
- **Smooth Operation**: Gradual zoom with 10% increments per scroll step
- **Cross-Platform**: Works consistently across different input devices

## Use Cases

### 1. Entity Editing
- Hover mouse over an entity and zoom in for precise editing
- Quickly inspect sprite details and properties
- Navigate between entities without losing focus

### 2. Procedural Map Navigation
- Zoom into specific areas of generated dungeons, cities, or terrain
- Examine tile placement and patterns in detail
- Quick overview-to-detail transitions

### 3. Level Design
- Precise placement of game objects
- Detail work on specific scene areas
- Efficient navigation during large level creation

## Testing Scenarios

### Manual Testing
1. **Basic Functionality**
   - Move cursor to different scene areas
   - Use mouse wheel to zoom in/out
   - Verify point under cursor stays fixed

2. **Edge Cases**
   - Test at zoom limits (0.1x and 10x)
   - Test with cursor at viewport edges
   - Test rapid zoom changes

3. **Integration Testing**
   - Combine with camera panning
   - Test with entity selection
   - Test with procedural content

4. **Precision Testing**
   - Zoom into specific entities
   - Zoom into tile corners/edges
   - Verify accuracy at different zoom levels

## Performance Considerations

### Computational Efficiency
- **Minimal Overhead**: Only active during mouse wheel events
- **Single Frame**: All calculations completed in one frame
- **No Continuous Processing**: No ongoing performance impact

### Memory Usage
- **No Additional Storage**: Uses existing camera and zoom state
- **Temporary Variables**: Only local variables during calculation
- **No Memory Leaks**: All calculations are stack-based

## Files Modified
- `src/editor/SceneWindow.cpp`: Replaced simple zoom with mouse-centered zoom algorithm

## Future Enhancements
- **Configurable Zoom Speed**: Allow users to adjust zoom sensitivity
- **Smooth Zoom Animation**: Add interpolated zoom for smoother visual experience
- **Touch Gestures**: Extend to support pinch-to-zoom on touch devices
- **Zoom Presets**: Quick zoom levels (25%, 50%, 100%, 200%, etc.)

## Compatibility
- **Backward Compatible**: No breaking changes to existing functionality
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **Input Agnostic**: Compatible with different mouse types and scroll wheel settings
