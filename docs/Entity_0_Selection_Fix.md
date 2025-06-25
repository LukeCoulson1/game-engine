# Entity Selection Fix - Entity 0 Issue Resolved

## Problem Description
When creating entities in the Scene Hierarchy, the first entity (Entity 0) would not appear in the Inspector panel when selected, but the second entity (Entity 1) would work correctly.

## Root Cause Analysis

### The Issue
The problem was a conflict between:

1. **Entity ID Generation**: `EntityManager` started entity IDs from 0
   ```cpp
   // In EntityManager.h - OLD CODE
   for (EntityID entity = 0; entity < MAX_ENTITIES; ++entity) {
       m_availableEntities.push(entity);
   }
   ```

2. **Selection Logic**: `SceneWindow::hasSelectedEntity()` treated 0 as "no selection"
   ```cpp
   // In SceneWindow.h
   bool hasSelectedEntity() const { return m_selectedEntity != 0; }
   ```

### The Conflict
- **Entity 0** was a valid, created entity
- But the UI system treated **Entity 0** as "no entity selected"
- This caused Entity 0 to never appear in the Inspector, even when clicked in the Scene Hierarchy

## Solution Implemented

### Changed Entity ID Generation
Modified `EntityManager` to start entity IDs from 1, reserving 0 as the null/invalid entity ID:

```cpp
// In EntityManager.h - NEW CODE
// Start from 1 since 0 is reserved as "null/invalid" entity
for (EntityID entity = 1; entity < MAX_ENTITIES; ++entity) {
    m_availableEntities.push(entity);
}
```

### Why This Fix Works
- **Entity 0** is now reserved as the "null" entity (never created)
- **First entity** created gets ID 1, which passes the `!= 0` check
- **All entities** now work correctly with the selection system
- **Follows common convention** where 0 = invalid/null entity

## Verification

### Before Fix:
- Create Entity → Gets ID 0 → `hasSelectedEntity()` returns false → Not shown in Inspector
- Create Entity → Gets ID 1 → `hasSelectedEntity()` returns true → Shows in Inspector

### After Fix:
- Create Entity → Gets ID 1 → `hasSelectedEntity()` returns true → Shows in Inspector
- Create Entity → Gets ID 2 → `hasSelectedEntity()` returns true → Shows in Inspector

## Files Modified
- `src/components/EntityManager.h` - Changed entity ID initialization to start from 1

## Testing
Run `test_entity_0_fix.bat` to verify:
1. Create multiple entities
2. Click on each entity in Scene Hierarchy
3. Verify all entities appear correctly in Inspector

## Result
✅ **Fixed**: All entities now properly appear in the Inspector when selected
✅ **Consistent**: Entity selection behavior is now predictable and reliable
✅ **Standard**: Follows ECS convention where Entity 0 = null/invalid

This fix resolves the core issue where "the second entity shows up in the inspector" but the first one doesn't.
