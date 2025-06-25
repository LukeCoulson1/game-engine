# Sprite Component Texture Update Fix

## Problem
When attaching a sprite component to an entity in the node editor and selecting a texture, the scene window would not update to show the texture. The inspector would correctly show the sprite component was added, but the texture itself wasn't being applied to the entity immediately.

## Root Cause
The issue was that when a texture was selected in a sprite node that was already connected to an entity, there was no mechanism to automatically update the entity with the new texture data. The texture selection only updated the node's internal component data, but didn't trigger a re-application of that component to the connected entity.

## Solution Implemented

### 1. Added Callback Mechanism to Node Class
- Added `std::function<void(Node*)> onComponentDataChanged` callback to the Node struct in `NodeEditor.h`
- Added `#include <functional>` to support std::function

### 2. Set Up Callback in NodeEditorWindow
- Modified `createNode()` method to set up the callback for all new nodes:
  ```cpp
  node->onComponentDataChanged = [this](Node* changedNode) {
      this->onNodeComponentDataChanged(changedNode);
  };
  ```

### 3. Implemented Callback Handler
- Added `onNodeComponentDataChanged()` method to NodeEditorWindow class
- This method:
  - Checks if the node is connected to an entity
  - Finds the connected entity
  - Automatically calls `applyComponentToEntity()` to update the entity with the new component data

### 4. Triggered Callback on Texture Selection
- Modified the texture selection code in `Node::drawTextureSelectionPopup()` to call the callback:
  ```cpp
  if (onComponentDataChanged) {
      onComponentDataChanged(this);
  }
  ```

### 5. Fixed Existing Bug
- Fixed typo in `applyComponentToEntity()` for PlayerAbilities case:
  - Changed `componentData` to `componentNode->componentData` for consistency

## Files Modified
1. `src/editor/NodeEditor.h` - Added callback mechanism and method declaration
2. `src/editor/NodeEditor.cpp` - Implemented callback system and fixed bug
3. `test_sprite_texture_update.bat` - Created test script

## How It Works Now
1. User creates a sprite component node and connects it to an entity
2. User selects a texture in the sprite node popup
3. The texture is applied to the node's component data
4. The callback is triggered, notifying the NodeEditorWindow
5. NodeEditorWindow automatically calls `applyComponentToEntity()` 
6. The entity's sprite component is updated with the new texture
7. Scene is marked as dirty (`setDirty(true)`) to trigger visual refresh
8. Scene window immediately shows the updated texture

## Benefits
- Immediate visual feedback when selecting textures
- No need for manual "Apply" buttons or entity reconnection
- Consistent behavior across all component types
- Automatic scene refresh
- Works for both newly created nodes and nodes loaded from existing entities

## Testing
Use the `test_sprite_texture_update.bat` script to verify:
1. Create entity and select it
2. Create sprite component node in node editor
3. Connect sprite node to entity node
4. Select texture in sprite node
5. Verify texture appears immediately in scene window
6. Verify inspector shows correct sprite component data
