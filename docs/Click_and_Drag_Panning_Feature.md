# Click and Drag Camera Panning Feature

## Overview
Added intuitive left mouse button click and drag functionality to pan the camera in scene windows, while preserving entity selection capabilities.

## Features Implemented

### 1. Left Mouse Button Dual Function
- **Click**: Select entities or convert procedural tiles to entities
- **Click and Drag**: Pan the camera through the scene

### 2. Intelligent Drag Detection
- **Drag Threshold**: 5 pixels minimum movement before drag mode activates
- **Prevents Accidental Panning**: Small mouse movements during selection don't trigger panning
- **State Tracking**: Clean separation between selection and panning modes

### 3. Zoom-Aware Panning
- **Scaled Movement**: Panning speed adjusts based on current zoom level
- **Consistent Feel**: Camera movement feels natural at all zoom levels
- **Formula**: `cameraPosition -= mouseDelta / zoomLevel`

### 4. Multiple Panning Methods
Users can now pan the camera using any of these methods:
- **Left Mouse Drag**: Primary intuitive method
- **Middle Mouse Drag**: Traditional CAD-style panning
- **Right Mouse Drag**: Alternative panning method
- **WASD Keys**: Keyboard-based camera movement
- **Arrow Keys**: Alternative keyboard movement

## Technical Implementation

### Member Variables Added
```cpp
// SceneWindow.h
bool m_isDragging = false;           // Tracks if currently in drag mode
ImVec2 m_dragStartPos{0, 0};        // Starting position of potential drag
const float m_dragThreshold = 5.0f; // Minimum pixels to start dragging
```

### Input Handling Logic
```cpp
// SceneWindow.cpp - handleInput()
if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        // Start tracking for potential drag
        m_isDragging = false;
        m_dragStartPos = mousePos;
    } else if (!m_isDragging) {
        // Check if we've moved far enough to start dragging
        float dragDistance = sqrt(dragDelta.x * dragDelta.x + dragDelta.y * dragDelta.y);
        if (dragDistance > m_dragThreshold) {
            m_isDragging = true;
        }
    }
    
    if (m_isDragging) {
        // Pan the camera based on mouse movement
        m_cameraPosition.x -= delta.x / m_zoomLevel;
        m_cameraPosition.y -= delta.y / m_zoomLevel;
    }
}

// Entity selection only occurs on mouse release if not dragging
if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !m_isDragging) {
    // Perform entity selection...
}
```

## User Experience Improvements

### 1. Intuitive Controls
- **Natural Feel**: Left mouse drag is the most intuitive way to pan in modern applications
- **No Learning Curve**: Users immediately understand the interaction
- **Consistent**: Matches behavior of image viewers, CAD software, and map applications

### 2. Precise Control
- **Zoom Scaling**: Panning sensitivity automatically adjusts with zoom level
- **Threshold Protection**: Prevents accidental panning during precision clicking
- **State Feedback**: Clear separation between selection and panning modes

### 3. Fallback Options
- **Multiple Methods**: Users can choose their preferred panning method
- **Keyboard Support**: WASD/Arrow keys for users who prefer keyboard navigation
- **Traditional Options**: Middle/right mouse still available for power users

## Testing

### Manual Test Cases
1. **Basic Panning**
   - Left click and drag to pan around the scene
   - Verify smooth camera movement
   - Test at different zoom levels

2. **Selection Preservation**
   - Single click on entities to select them
   - Single click on procedural tiles to convert them
   - Verify no interference between selection and panning

3. **Drag Threshold**
   - Make very small mouse movements during clicks
   - Verify entity selection still works
   - Test the 5-pixel threshold boundary

4. **Multi-Method Panning**
   - Test left mouse drag panning
   - Test middle mouse drag panning
   - Test right mouse drag panning
   - Test WASD keyboard panning

5. **Zoom Integration**
   - Pan at 0.5x zoom (slower movement)
   - Pan at 1x zoom (normal movement)
   - Pan at 4x zoom (faster movement)
   - Verify consistent feel across zoom levels

## Benefits

### For Users
- **Improved Workflow**: Faster navigation around large scenes
- **Reduced Friction**: No need to use middle mouse or keyboard for basic panning
- **Better Precision**: Zoom-aware panning provides appropriate sensitivity
- **Familiar Interface**: Matches standard application behaviors

### For Developers
- **Clean Implementation**: Well-separated selection and panning logic
- **Extensible**: Easy to modify drag threshold or add new panning modes
- **Maintainable**: Clear state management and event handling

## Files Modified
- `src/editor/SceneWindow.h`: Added drag state member variables and ImGui include
- `src/editor/SceneWindow.cpp`: Implemented dual-mode left mouse button handling

## Future Enhancements
- **Configurable Threshold**: Allow users to adjust the drag threshold in settings
- **Momentum Panning**: Add inertia to camera movement for smoother feel
- **Touch Support**: Extend to support touch gestures on compatible devices
- **Multi-Touch**: Support pinch-to-zoom and two-finger panning
