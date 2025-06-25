# Asset Browser Folder Configuration

## Overview
The Asset Browser now supports configurable asset folder locations, allowing you to organize your project assets in any folder structure you prefer.

## Features Added

### 1. Configurable Asset Folder Path
- **Default**: `assets` (maintains backward compatibility)
- **Editable**: Can be changed through the Asset Browser UI
- **Persistent**: Settings are saved to `editor_config.json`

### 2. Asset Browser Enhancements

#### Folder Management
- **Text Input Field**: Edit the asset folder path directly
- **Browse Button**: For future folder dialog integration
- **Open Folder**: Opens the current asset folder in file explorer
- **Create Folder**: Creates the asset folder if it doesn't exist

#### Enhanced File Display
- **Subdirectory Support**: Shows and scans subdirectories for assets
- **Better Categorization**: Images, Audio, and Other files with icons
- **Improved Tooltips**: Shows full file paths and information
- **Case-Insensitive**: File extension matching works regardless of case

### 3. Inspector Integration
- **Automatic Updates**: Texture assignment in Inspector uses the configured folder
- **Dynamic Paths**: All asset references update when folder changes
- **Subfolder Support**: Automatically scans for `tiles/` subfolder

### 4. Configuration Management
- **ConfigManager**: New asset folder methods added
  - `setAssetFolder(const std::string& folder)`
  - `getAssetFolder() const`
- **Auto-Save**: Configuration automatically saved when folder changes

## Usage

### Changing Asset Folder
1. Open the **Asset Browser** window
2. Edit the path in the text field at the top
3. Press **Enter** to confirm the change
4. The new path is automatically saved to configuration

### Supported Workflows
- **Project Organization**: Use different folders for different projects
- **External Assets**: Point to shared asset libraries
- **Version Control**: Use relative paths for team collaboration

### File Organization
The system supports the following structure:
```
your-asset-folder/
├── images/           (general images)
├── tiles/           (automatically scanned)
├── audio/           (sound files)
└── other-files/     (any other assets)
```

## Technical Implementation

### ConfigManager Extensions
```cpp
// New methods in ConfigManager
void setAssetFolder(const std::string& folder);
std::string getAssetFolder() const;

// Default configuration
m_stringValues["assets.folder"] = "assets";
```

### GameEditor Changes
```cpp
// New member variable
std::string m_currentAssetFolder;

// Initialization from config
m_currentAssetFolder = config.getAssetFolder();
```

### Enhanced Asset Browser
- Dynamic folder path display
- Real-time folder validation
- Improved error handling
- Better user feedback

## Backward Compatibility
- **Default Path**: Still uses `assets` folder by default
- **Existing Projects**: No migration needed
- **Configuration**: Old configs work without modification

## Future Enhancements
- Native folder browser dialog
- Recent folders list
- Asset folder bookmarks
- Project-specific asset folder sets

## Error Handling
- **Invalid Paths**: Clear error messages with suggestions
- **Missing Folders**: Option to create folders automatically
- **Permission Issues**: Graceful degradation with helpful feedback
- **File Access**: Robust exception handling throughout

This enhancement makes the game engine more flexible for different project structures and asset organization preferences while maintaining full backward compatibility.
