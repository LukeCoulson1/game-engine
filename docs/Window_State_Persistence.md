# Window State Persistence Guide

The Game Engine Editor now supports automatic window state persistence, which means the editor will remember:

- Window size (width and height)
- Window position on screen
- Maximized state
- Panel visibility settings

## How It Works

### Automatic Saving
The editor automatically saves window state when you:
- Close the editor normally (click the X button)
- Exit through the File menu (if implemented)
- The application shuts down gracefully

### Automatic Loading
When you start the editor, it will:
- Load the previous window size and position
- Restore the maximized state if it was maximized
- Show/hide panels based on previous session

## Configuration File

Window state is saved in `editor_config.json` in the same directory as the editor executable.

Example config file:
```json
{
    "panels": {
        "AssetBrowser": true,
        "CameraControls": true,
        "Console": true,
        "Inspector": true,
        "ProceduralGeneration": false,
        "SceneHierarchy": true
    },
    "settings": null,
    "window": {
        "height": 720,
        "maximized": false,
        "width": 1280,
        "x": 100,
        "y": 100
    }
}
```

## Testing the Feature

1. **Run the test script**: Execute `test_window_persistence.bat` from the game engine root directory
2. **Manual testing**:
   - Start the editor
   - Resize the window
   - Move it to a different position
   - Optionally maximize it
   - Close the editor normally (click X)
   - Restart the editor - it should open at the same size and position

## Implementation Details

### Key Components
- **ConfigManager**: Handles loading/saving configuration data
- **GameEditor**: Manages window state during startup and shutdown
- **Renderer**: Provides access to SDL window for state queries

### Default Values
If no config file exists, the editor starts with:
- Width: 1280 pixels
- Height: 720 pixels
- Position: Centered on screen
- All panels visible except ProceduralGeneration

### Error Handling
- If the config file is corrupted, defaults are used
- If the saved position is off-screen, the window is centered
- Missing config sections use appropriate defaults

## File Locations

- Config file: `[editor directory]/editor_config.json`
- Source files:
  - `src/utils/ConfigManager.h/cpp`
  - `src/editor/GameEditor.h/cpp`
  - `src/graphics/Renderer.h`

## Future Enhancements

Potential improvements that could be added:
- Real-time saving (save on resize/move events)
- Multiple monitor support validation
- UI theme persistence
- Recent files list
- Custom keyboard shortcuts
- Editor layout presets

## Troubleshooting

**Window opens in wrong position:**
- Delete `editor_config.json` to reset to defaults
- Check if the saved position is valid for your current monitor setup

**Config file not being saved:**
- Ensure the editor directory is writable
- Check that you're closing the editor normally (not force-killing)
- Verify the config file permissions

**Editor crashes on startup:**
- Delete or rename `editor_config.json`
- Check the console for error messages
- Verify all required DLLs are present
