# Coordinate System Fix v2 - Tile Center Alignment

## Problem Description
When clicking on procedurally generated tiles, the created entities appeared at a different location than where the tile was visually rendered. This created a mismatch between the visual representation and the actual entity position.

## Root Cause Analysis
The issue was in the coordinate system used by `ProceduralMap`:

1. **Original Problem**: `ProceduralMap::getWorldPosition(x, y)` returned the **top-left corner** of tiles (x*32, y*32)
2. **TileRenderer Behavior**: The `TileRenderer` draws tiles **centered** at the world position using `imageMin/imageMax` with +/- tileSize/2
3. **Entity Placement**: When converting tiles to entities, entities were placed at the top-left corner instead of the center
4. **Visual Mismatch**: Tiles appeared centered, but entities appeared at the top-left, creating a 16-pixel offset

## Solution Implemented

### 1. Updated ProceduralMap::getWorldPosition()
```cpp
// Before (top-left corner):
Vector2 ProceduralMap::getWorldPosition(int x, int y) const {
    return Vector2(x * 32.0f, y * 32.0f);
}

// After (tile center):
Vector2 ProceduralMap::getWorldPosition(int x, int y) const {
    return Vector2(x * 32.0f + 16.0f, y * 32.0f + 16.0f);
}
```

### 2. Updated ProceduralMap::getGridPosition()
```cpp
// Before (simple division):
Vector2 ProceduralMap::getGridPosition(const Vector2& worldPos) const {
    return Vector2(static_cast<int>(worldPos.x / 32.0f), 
                   static_cast<int>(worldPos.y / 32.0f));
}

// After (account for center offset):
Vector2 ProceduralMap::getGridPosition(const Vector2& worldPos) const {
    return Vector2(static_cast<int>((worldPos.x - 16.0f) / 32.0f + 0.5f), 
                   static_cast<int>((worldPos.y - 16.0f) / 32.0f + 0.5f));
}
```

## Coordinate System Consistency

Now all coordinate systems are aligned:

1. **TileRenderer**: Draws tiles centered at world positions
2. **Entity Rendering**: Renders entities centered at world positions  
3. **Mouse Click Detection**: Converts screen coordinates to world coordinates
4. **ProceduralMap**: Returns tile center positions as world coordinates
5. **Entity Creation**: Places entities at tile center positions

## Testing

### Manual Testing Steps
1. Create a scene window
2. Generate a procedural map (dungeon, city, or terrain)
3. Click on any generated tile
4. Verify the entity appears exactly where the tile is visually located
5. Test at different zoom levels (0.5x, 1x, 2x, 4x)
6. Test with camera panning
7. Test with different tile types

### Expected Results
- ✅ Perfect visual alignment between tiles and entities
- ✅ Accurate mouse click detection at all zoom levels
- ✅ No visual offset between procedural tiles and converted entities
- ✅ Consistent behavior across all procedural generation types

## Files Modified
- `src/generation/ProceduralGeneration.cpp`: Updated coordinate conversion methods

## Impact
- **Positive**: Perfect coordinate alignment, improved user experience
- **Neutral**: No performance impact, existing entities unaffected
- **Breaking**: None (coordinate system is now more intuitive)

## Future Considerations
This fix ensures that all coordinate systems in the engine are consistent and center-based, which is more intuitive for users and developers.
