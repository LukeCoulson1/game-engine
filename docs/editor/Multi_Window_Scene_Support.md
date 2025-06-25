# Multi-Window Scene Support

## Overview

The Game Engine Editor now supports multiple scene windows that can be opened simultaneously, allowing you to edit multiple scenes at once. Each scene window is independently resizable and maintains its own state, camera position, and selection.

## Features

### 🪟 Window Management
- **Multiple Scene Windows**: Open multiple scenes simultaneously in separate windows
- **Independent Sizing**: Each scene window can be resized independently
- **Window Controls**: Scene windows can be moved, resized, and closed individually
- **Window Menu**: Access all open scene windows through the Window menu

### 🎬 Scene Operations
- **Independent Scenes**: Each window contains its own complete scene
- **Entity Management**: Create, edit, and delete entities in any scene window
- **Selection State**: Each scene maintains its own selected entity
- **Camera Position**: Independent camera controls per scene window

### 🎮 Viewport Controls
- **WASD Movement**: Use WASD or arrow keys for camera movement in the focused window
- **Mouse Panning**: Middle mouse or right mouse button for camera panning
- **Entity Selection**: Left-click to select entities in the scene viewport
- **Independent Controls**: Camera and selection work independently in each scene

### 🔧 UI Integration
- **Active Scene Tracking**: Scene Hierarchy shows which scene is currently active
- **Inspector Integration**: Inspector panel works with entities from the active scene
- **Asset Browser**: Assign textures and assets to entities in any scene
- **Console Messages**: Console shows which scene operations are performed on

### 💾 State Management
- **Dirty State Tracking**: Window titles show asterisk (*) for unsaved changes
- **Per-Scene State**: Each scene tracks its own modification state
- **Scene Persistence**: Window size/position settings are saved per session

## How to Use

### Opening Multiple Scenes

1. **Create New Scene Window**:
   - File → New Scene (creates a new scene in a new window)
   - Window → New Scene Window (creates another scene window)

2. **Open Existing Scene**:
   - File → Open Scene in New Window (loads a scene in a new window)

### Managing Scene Windows

1. **Switch Between Scenes**:
   - Click on any scene window to make it active
   - Use Window menu to see all open scenes and switch between them

2. **Close Scene Windows**:
   - Click the X button on the scene window
   - Use Window → Close Current Window
   - Use Ctrl+W keyboard shortcut

### Working with Multiple Scenes

1. **Entity Operations**:
   - Create entities: Click "Create Entity" or use Ctrl+E
   - Delete entities: Select entity and press Delete key
   - Rename entities: Use Inspector panel or right-click context menu

2. **Camera Controls**:
   - **WASD**: Move camera in the focused scene window
   - **Arrow Keys**: Alternative camera movement
   - **Middle Mouse**: Drag to pan camera
   - **Right Mouse**: Alternative camera panning

3. **Asset Management**:
   - Use Asset Browser to assign textures to entities in any scene
   - Changes affect the scene containing the selected entity

## Technical Implementation

### Architecture
- **SceneWindow Class**: Manages individual scene windows with independent state
- **GameEditor Integration**: Manages multiple SceneWindow instances
- **Active Scene Tracking**: UI panels work with the currently active scene window

### Scene Window Components
- **Independent Viewport**: Each window has its own rendering area
- **Menu Bar**: Scene-specific menu options (Save, Save As, Close)
- **Camera State**: Independent camera position and controls
- **Selection State**: Each scene tracks its own selected entity

### UI Panel Integration
- **Scene Hierarchy**: Shows entities from the active scene
- **Inspector**: Works with selected entity from active scene
- **Asset Browser**: Can assign assets to any scene window
- **Console**: Shows operation results with scene context

## Menu Reference

### File Menu
- **New Scene** (Ctrl+N): Creates a new scene in a new window
- **Open Scene in New Window** (Ctrl+Shift+N): Opens a scene file in a new window
- **Load Scene** (Ctrl+O): Loads a scene (TODO: File dialog)
- **Save Scene** (Ctrl+S): Saves the active scene (TODO: File dialog)

### Edit Menu
- **Create Entity** (Ctrl+E): Creates a new entity in the active scene
- **Delete Entity** (Delete): Deletes the selected entity from the active scene

### Window Menu
- **New Scene Window** (Ctrl+Shift+W): Creates a new empty scene window
- **[Scene List]**: Shows all open scene windows (click to activate)
- **Close Current Window** (Ctrl+W): Closes the active scene window

### Scene Window Menu Bar
Each scene window has its own menu bar with:
- **Scene Menu**: Save, Save As, Close options
- **Edit Menu**: Create Entity, Delete Entity options
- **View Menu**: Center Camera, Reset Zoom options

## Keyboard Shortcuts

### Global Shortcuts
- **Ctrl+N**: New scene in new window
- **Ctrl+Shift+N**: Open scene in new window
- **Ctrl+Shift+W**: New scene window
- **Ctrl+W**: Close current window
- **Ctrl+E**: Create entity in active scene
- **Delete**: Delete selected entity

### Viewport Controls (when scene window is focused)
- **WASD**: Camera movement
- **Arrow Keys**: Camera movement
- **Home**: Center camera on origin
- **Left Click**: Select entity
- **Middle Mouse Drag**: Pan camera
- **Right Mouse Drag**: Pan camera

## State Persistence

### Window State
- Window size and position are saved between sessions
- Panel visibility settings are preserved
- Camera position is maintained per scene window

### Scene State
- Each scene tracks its own dirty state (unsaved changes)
- Selection state is independent per scene
- Camera position is saved per scene window

## Tips and Best Practices

### Working with Multiple Scenes
1. **Scene Organization**: Use descriptive names for scene windows
2. **Active Scene**: Click on a scene window to make it active before using panels
3. **Asset Workflow**: Use Asset Browser to manage textures across all scenes
4. **Camera Management**: Each scene has independent camera controls

### Performance Considerations
1. **Resource Usage**: Multiple scenes use more memory and CPU
2. **Rendering**: Only visible scene windows are actively rendered
3. **Entity Limits**: Each scene can have its own set of entities

### Workflow Tips
1. **Level Design**: Use multiple scenes for different game levels
2. **Asset Testing**: Test assets in different scene contexts
3. **Comparison**: Compare different design iterations side by side
4. **Modular Development**: Work on different game areas simultaneously

## Troubleshooting

### Common Issues

1. **Scene Hierarchy Empty**:
   - Make sure a scene window is active (click on it)
   - Check that the scene window is open and visible

2. **Inspector Not Working**:
   - Ensure an entity is selected in the active scene
   - Click on a scene window to make it active

3. **Camera Controls Not Responding**:
   - Click on the scene viewport to focus it
   - Make sure the scene window is the active window

4. **Asset Assignment Issues**:
   - Select an entity in the active scene before using Asset Browser
   - Make sure the entity has a Sprite component

### Performance Issues
1. **Too Many Windows**: Close unused scene windows to improve performance
2. **Memory Usage**: Monitor memory usage with multiple large scenes
3. **Rendering Performance**: Only work with necessary scene windows open

## Future Enhancements

### Planned Features
- **Scene Saving/Loading**: Full file dialog support for scene management
- **Drag and Drop**: Move entities between scene windows
- **Scene Tabs**: Optional tabbed interface for scene management
- **Scene Templates**: Create new scenes from templates
- **Multi-Selection**: Select and edit multiple entities across scenes

### Advanced Features
- **Scene Linking**: Reference entities between scenes
- **Asset Synchronization**: Automatic asset updates across scenes
- **Scene Comparison**: Visual comparison tools for different scenes
- **Collaborative Editing**: Multiple users editing different scenes

## Related Documentation

- [Window State Persistence](Window_State_Persistence.md)
- [Procedural Generation](Procedural_Generation_Fix.md)
- [Nomenclature System](Nomenclature_System.md)
- [Quick Dungeon Test](../Quick_Dungeon_Test.md)

This multi-window scene system provides a powerful foundation for complex game development workflows while maintaining the simplicity and usability of the original editor.
