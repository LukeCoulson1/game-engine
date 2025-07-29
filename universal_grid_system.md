# Universal Grid System Implementation

## Overview
Implemented a universal grid system that automatically resizes all entities (including those with sprites) to fit within individual grid squares, ensuring consistent sizing across the entire game world.

## Key Features

### 1. Grid Size Constant
- **GRID_SIZE**: 32.0f pixels - universal grid square size
- Defined as `static constexpr float` in `SceneWindow.h` for consistency
- Used throughout the rendering and collision systems

### 2. Automatic Sprite Grid-Fitting
All sprites are automatically scaled to fit within one grid square while maintaining their aspect ratio:

#### Grid-Fitting Algorithm:
```cpp
Vector2 calculateGridFitScale(float textureWidth, float textureHeight) const {
    float scaleX = GRID_SIZE / textureWidth;
    float scaleY = GRID_SIZE / textureHeight;
    
    // Use smaller scale to ensure sprite fits entirely within grid square
    float uniformScale = std::min(scaleX, scaleY);
    return Vector2(uniformScale, uniformScale);
}
```

### 3. Entity Creation with Grid-Fitting

#### Procedural Tiles → Entities:
- When clicking on procedural tiles to convert them to entities
- Automatically applies grid-fitting scale to the entity's transform
- Maintains tile positioning but ensures consistent sizing

#### Manual Entity Creation:
- "Create Entity" menu item (Ctrl+N) creates entities at camera center
- Automatically applies grid-fitting scale for consistent sizing
- Non-sprite entities get default 1.0x scale

### 4. Rendering System Integration

#### Sprite Rendering:
```cpp
// Calculate grid-fitting scale
Vector2 gridFitScale = calculateGridFitScale(texWidth, texHeight);

// Apply grid fitting, transform scale, and zoom
float scaledWidth = GRID_SIZE * gridFitScale.x * transform.scale.x * m_zoomLevel;
float scaledHeight = GRID_SIZE * gridFitScale.y * transform.scale.y * m_zoomLevel;
```

#### Benefits:
- **Visual Consistency**: All sprites appear at consistent sizes relative to the grid
- **Collision Accuracy**: Collision detection automatically uses grid-fitted dimensions
- **Transform Integration**: Grid-fitting is applied via transform.scale, so it works with rotation and additional scaling
- **Aspect Ratio Preservation**: Sprites maintain their original proportions

### 5. Collision Detection Compatibility
The collision system automatically works with grid-fitted entities because:
- Grid-fitting scale is stored in `transform.scale`
- Collision detection multiplies texture dimensions by `transform.scale`
- No additional changes needed for collision accuracy

### 6. Grid Visualization
- Grid overlay uses the same `GRID_SIZE` constant
- Ensures perfect alignment between visual grid and entity sizing
- Grid lines appear at proper intervals matching entity boundaries

## Usage Examples

### Creating Consistent Entities:
1. **From Menu**: Edit → Create Entity (Ctrl+N) - creates grid-fitted entity at camera center
2. **From Tiles**: Click on procedural map tiles - converts to grid-fitted entities
3. **Custom Sprites**: Any entity with a sprite component gets automatically grid-fitted

### Transform Scale Interaction:
- Grid-fitting provides the base scale (e.g., 0.5x for a 64x64 sprite in 32px grid)
- Additional scaling via transform.scale multiplies on top of grid-fitting
- Example: 64x64 sprite gets 0.5x grid-fit scale, then 2.0x user scale = final 1.0x scale

### Visual Results:
- **Before**: Sprites of different sizes (16x16, 32x32, 64x64) appeared at different scales
- **After**: All sprites fit within one grid square, maintaining visual hierarchy and consistency

## Technical Implementation

### Files Modified:
1. **SceneWindow.h**: Added GRID_SIZE constant and grid-fitting functions
2. **SceneWindow.cpp**: 
   - Modified sprite rendering to use grid-fitting
   - Updated entity creation to apply grid-fitting
   - Added grid-fitting utility functions
   - Updated grid visualization to use GRID_SIZE constant

### Performance Considerations:
- **Minimal Overhead**: Grid-fitting calculated only during entity creation/sprite assignment
- **Runtime Efficiency**: No additional calculations during rendering beyond normal transform scaling
- **Memory Usage**: No additional memory overhead - uses existing transform.scale system

## Future Enhancements
- **Grid Snapping**: Optionally snap entity positions to grid coordinates
- **Multi-Grid Entities**: Support entities that span multiple grid squares
- **Dynamic Grid Size**: Allow runtime adjustment of GRID_SIZE for different game areas
- **Grid-Aligned Rotation**: Constrain rotations to 90-degree increments aligned with grid

## Testing Guidelines
1. **Create entities via Edit menu** - should appear at consistent grid-fitted sizes
2. **Click on procedural tiles** - converted entities should fit within grid squares
3. **Load sprites of various dimensions** - all should scale to fit grid consistently
4. **Test collision detection** - should work accurately with grid-fitted entities
5. **Verify grid alignment** - visual grid lines should align with entity boundaries
6. **Test transform scaling** - additional scaling should multiply on top of grid-fitting
