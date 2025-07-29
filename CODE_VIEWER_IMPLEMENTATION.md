# Game Code Viewer Implementation

## Overview
The Game Code Viewer is a new editor window that displays all the source code for the **game project** being created with the game engine editor. Unlike showing the engine's internal code, this viewer focuses on the user's game logic, scripts, components, and scenes - essentially the code that defines their specific game.

## Features

### Game Project Code Display
- Shows user-created game code (not engine code)
- Displays game logic, custom components, scripts, and scene files
- Supports multiple programming and scripting languages (C++, Lua, JavaScript, Python, etc.)
- Automatically scans game-specific directories

### Comprehensive File Support
The viewer supports various file types commonly used in game development:
- **C++ Files**: `.cpp`, `.h`, `.hpp` - Core game logic and components
- **Script Files**: `.lua`, `.js`, `.py`, `.cs` - Game scripts and logic
- **Scene Files**: `.json`, `.scene` - Scene definitions and configurations
- **Text Files**: `.txt` - Documentation, notes, configuration

### Two-Pane Interface
- **Left Pane**: File list with color-coded file types
  - Header files (.h) in light blue
  - C++ source (.cpp) in light green  
  - Lua scripts (.lua) in orange
  - JavaScript (.js) in yellow
  - Python scripts (.py) in light cyan
  - Scene/JSON files in light purple
  - C# files (.cs) in light mint
  - Files sorted alphabetically for easy navigation

- **Right Pane**: Code content viewer
  - Read-only text display with tab support
  - Line count information
  - Full file path display
  - Scrollable content for large files

### Game Directory Structure
The viewer scans these game-specific directories:
- `game/` - Main game code directory
- `game/scripts/` - Game scripts and logic files
- `game/entities/` - Custom entity definitions
- `game/components/` - Custom game components
- `game/systems/` - Custom game systems
- `game/scenes/` - Scene-specific code
- `assets/scripts/` - Asset-based scripts
- `assets/scenes/` - Scene definition files
- `scripts/` - Alternative scripts directory

### Template System
When no game code is found, the viewer shows example templates:
- **GameLogic.h/cpp** - Main game logic class template
- **PlayerController.cpp** - Player input handling example
- **Scene Files** - Example scene configuration with entities and components

### Project Creation Tools
- **"Create Game Directory"** button - Sets up the standard game project structure
- **Starter Files** - Creates basic GameLogic.h/cpp files to get started
- **Directory Creation** - Automatically creates all necessary folders

## Usage

### Accessing the Code Viewer
1. Open the Game Editor
2. Navigate to `View` menu in the main menu bar
3. Click `Code Viewer` to open the window

### Navigation
1. Select any file from the left panel to view its contents
2. Use the "Refresh Code Files" button to reload after making changes
3. Use "Create Game Directory" to set up project structure
4. Hover over filenames to see full paths
5. Scroll through code content in the right panel

### Setting Up Your Game Project
1. Click "Create Game Directory" to create the standard structure
2. Write your game code in the `game/` directory
3. Use the provided GameLogic.h/cpp templates as starting points
4. Create custom components, systems, and scripts as needed
5. Use the Code Viewer to review and organize your game code

### File Organization Best Practices
- Put main game logic in `game/GameLogic.h` and `game/GameLogic.cpp`
- Create custom components in `game/components/`
- Write game systems in `game/systems/`
- Store scripts in `game/scripts/`
- Keep scene-specific code in `game/scenes/`

## Implementation Details

### Core Components

#### GameEditor.h Additions
```cpp
// Code viewer state
struct CodeFile {
    std::string path;
    std::string filename;
    std::string content;
    bool isGameCode;
};
std::vector<CodeFile> m_codeFiles;
int m_selectedCodeFile = 0;
bool m_codeFilesLoaded = false;
bool m_showCodeViewer = false;
```

#### Key Methods
- `showCodeViewer()` - Main UI rendering method
- `loadCodeFiles()` - Scans directories and loads file contents
- Integrated with existing menu system and window management

### File Loading Process
1. Clears previous file list
2. Scans predefined directories recursively
3. Filters files by extension (.cpp, .h, .hpp)
4. Excludes editor-specific files
5. Loads file contents into memory
6. Sorts files alphabetically
7. Updates console with load status

### Error Handling
- Graceful handling of missing directories
- File system error reporting to console
- Safe memory management for large files

## Benefits

### For Developers
- **Complete Code Overview**: See all game code in one place
- **Build Understanding**: Know exactly what gets compiled into the game
- **Code Organization**: Better understanding of project structure
- **Debugging Aid**: Quick access to all runtime code

### For Project Management
- **Code Review**: Easy way to review all game code
- **Documentation**: Living documentation of the codebase
- **Quality Assurance**: Verify what code is included in builds

## Technical Specifications

### Performance Considerations
- Files loaded on-demand (lazy loading)
- Efficient memory usage with string storage
- Fast file system scanning with std::filesystem
- Responsive UI with ImGui immediate mode rendering

### Compatibility
- Works with existing CMake build system
- Compatible with C++17 standard
- Uses standard library for file operations
- Integrated with existing ImGui interface

### Dependencies
- std::filesystem for directory scanning
- std::fstream for file reading
- ImGui for user interface
- Existing GameEditor infrastructure

## Future Enhancements

### Potential Improvements
1. **Syntax Highlighting**: Add full C++ syntax highlighting
2. **Search Functionality**: Search across all code files
3. **Code Metrics**: Display code statistics and complexity
4. **Export Options**: Export code to external formats
5. **Code Folding**: Collapse/expand code sections
6. **Line Numbers**: Show line numbers in code display
7. **Find and Replace**: Basic editing capabilities

### Integration Opportunities
- Link with build system to show actual compilation order
- Integration with version control to show changes
- Connection to debugging system for runtime code mapping
- Asset dependency tracking to show code-asset relationships

## Testing and Validation

### Verification Steps
1. ✅ Menu item added to View menu
2. ✅ Window opens/closes properly
3. ✅ Files load from correct directories
4. ✅ Editor files properly excluded
5. ✅ File content displays correctly
6. ✅ UI responsive and functional
7. ✅ Error handling works for missing files
8. ✅ Refresh functionality updates file list

### Test Results
- Successfully loads all game code files
- Proper exclusion of editor-specific code
- Responsive two-pane interface
- Color coding works correctly
- File tooltips show proper paths
- Console logging provides feedback
- Memory usage remains reasonable

## Conclusion

The Game Code Viewer provides a comprehensive solution for viewing all game runtime code within the editor environment. It enhances the development workflow by offering complete visibility into the codebase that will be compiled into the final game executable, while maintaining good performance and a user-friendly interface.

The implementation successfully integrates with the existing editor architecture and provides a solid foundation for future enhancements to code viewing and analysis capabilities.
