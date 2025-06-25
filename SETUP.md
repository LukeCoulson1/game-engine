# Setup Instructions for 2D Game Engine

## Prerequisites

### 1. Install Required Tools
- **Visual Studio 2022** (with C++ development tools)
- **CMake** (3.16 or higher)
- **vcpkg** (Microsoft C++ package manager)
- **Git**

### 2. Install vcpkg
```powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

### 3. Install Dependencies with vcpkg
```powershell
# Set environment variable
$env:VCPKG_ROOT = "C:\path\to\your\vcpkg"

# Install required packages
.\vcpkg install sdl2:x64-windows
.\vcpkg install sdl2-image:x64-windows  
.\vcpkg install sdl2-mixer:x64-windows
.\vcpkg install sdl2-ttf:x64-windows
.\vcpkg install glm:x64-windows
.\vcpkg install nlohmann-json:x64-windows
```

### 4. Get Dear ImGui
Since ImGui is typically included as source, download it manually:
```powershell
# Download ImGui release from https://github.com/ocornut/imgui
# Extract to external/imgui/ folder
```

## Building the Engine

### Option 1: Using the Build Script
```powershell
# Set VCPKG_ROOT environment variable first
$env:VCPKG_ROOT = "C:\path\to\your\vcpkg"

# Run the build script
.\build.bat
```

### Option 2: Manual Build
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

## Running the Engine

### Game Editor
```powershell
.\bin\Release\GameEditor.exe
```

### Game Runtime
```powershell
.\bin\Release\GameRuntime.exe
```

## Project Structure

```
game-engine/
├── src/
│   ├── core/           # Core engine systems
│   ├── graphics/       # Rendering system
│   ├── audio/          # Audio management
│   ├── input/          # Input handling
│   ├── scene/          # Scene management
│   ├── components/     # ECS components
│   ├── systems/        # ECS systems
│   ├── editor/         # GUI editor
│   └── utils/          # Utilities
├── external/           # External libraries
├── assets/             # Game assets
├── build/              # Build output
└── CMakeLists.txt      # Build configuration
```

## Features

### Core Engine
- Entity-Component-System architecture
- 2D sprite rendering
- Physics simulation
- Collision detection
- Audio system (sounds and music)
- Input handling (keyboard, mouse, controller)
- Scene management

### Editor Features
- Visual scene editor with docking layout
- Entity hierarchy view
- Component inspector
- Asset browser
- Real-time viewport
- Console output

### Zelda-like Game Features
- Top-down 2D camera system
- Tile-based world support
- Character controller components
- Collision system suitable for adventure games
- Resource management for sprites and audio

## Usage

### Creating a New Game
1. Launch the editor
2. Create entities using the hierarchy panel
3. Add components (Transform, Sprite, Collider, RigidBody)
4. Assign textures and configure properties
5. Test using the runtime

### Adding Custom Components
1. Define component in `src/components/Components.h`
2. Register in scene initialization
3. Create systems to process components
4. Add UI for component in editor inspector

## Troubleshooting

### Common Issues
- **CMake can't find packages**: Make sure VCPKG_ROOT is set correctly
- **Missing DLLs**: SDL2 DLLs should be copied automatically, check bin folder
- **ImGui compilation errors**: Ensure ImGui source files are in external/imgui/

### Performance Tips
- Use object pooling for frequently created/destroyed entities
- Batch sprite rendering by texture/layer
- Profile using built-in console timings
- Optimize collision detection with spatial partitioning (future enhancement)
