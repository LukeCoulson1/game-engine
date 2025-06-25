# 2D Game Engine

A complete 2D game engine built in C++ designed for creating Zelda-like top-down adventure games with an intuitive GUI editor.

![Engine Screenshot](docs/editor-screenshot.png)

## 🚀 Features

### 🎮 Core Engine
- **Entity-Component-System (ECS)** architecture for flexible game object management
- **High-performance 2D rendering** with SDL2 and hardware acceleration
- **Physics simulation** with collision detection and response
- **Multi-layer sprite rendering** with sorting and culling
- **Audio system** supporting sound effects and background music
- **Input management** for keyboard, mouse, and game controllers
- **Resource management** with automatic loading and caching
- **Scene management** for organizing game content

### 🛠️ Visual Editor
- **Dockable GUI** built with Dear ImGui for professional workflow
- **Scene hierarchy** with drag-and-drop entity management
- **Component inspector** for real-time property editing
- **Asset browser** for easy resource management
- **Real-time viewport** with immediate feedback
- **Console output** for debugging and logging
- **Undo/redo system** for safe editing

### 🏰 Zelda-like Game Features
- **Top-down 2D camera** system perfect for adventure games
- **Tile-based world** system for efficient level design
- **Character controller** with smooth movement and collision
- **Inventory and item** management systems
- **Dialogue system** for NPC interactions
- **Save/load functionality** for game progression
- **Room transition** system for connected areas

## 📋 Prerequisites

- **Windows 10/11** (Linux/Mac support planned)
- **Visual Studio 2022** with C++ development tools
- **CMake 3.16+**
- **vcpkg** package manager
- **Git**

## 🔧 Quick Setup

### 1. Install vcpkg
```powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

### 2. Install Dependencies
```powershell
vcpkg install sdl2:x64-windows sdl2-image:x64-windows sdl2-mixer:x64-windows sdl2-ttf:x64-windows glm:x64-windows nlohmann-json:x64-windows
```

### 3. Get Dear ImGui
Download ImGui from [GitHub](https://github.com/ocornut/imgui) and extract to `external/imgui/`

### 4. Build the Engine
```powershell
$env:VCPKG_ROOT = "C:\path\to\your\vcpkg"
.\build.bat
```

### 5. Run!
```powershell
# Launch the editor
.\bin\Release\GameEditor.exe

# Or run a game directly
.\bin\Release\GameRuntime.exe
```

## 🎯 Quick Start Tutorial

### Creating Your First Entity
1. Launch the **Game Editor**
2. Click **"Create Entity"** in the Scene Hierarchy
3. Select the entity and add components in the **Inspector**:
   - **Transform** (position, scale, rotation)
   - **Sprite** (visual representation)
   - **Collider** (physics collision)
   - **RigidBody** (physics movement)

### Building a Simple Scene
```cpp
// Example: Creating a player character
EntityID player = scene->createEntity();
scene->addComponent<Transform>(player, Transform(100, 100));
scene->addComponent<Sprite>(player, Sprite(playerTexture));
scene->addComponent<Collider>(player, Collider(32, 32));
scene->addComponent<PlayerController>(player, PlayerController());
```

### Adding Game Logic
```cpp
// Custom system for player movement
class PlayerSystem : public System {
public:
    void update(float deltaTime) override {
        for (auto entity : entities) {
            auto& transform = scene->getComponent<Transform>(entity);
            auto& controller = scene->getComponent<PlayerController>(entity);
            
            // Handle input and update position
            handlePlayerInput(transform, controller, deltaTime);
        }
    }
};
```

## 📁 Project Structure

```
game-engine/
├── 📂 src/
│   ├── 📂 core/           # Engine core (Engine.h, main loop)
│   ├── 📂 graphics/       # Rendering (Renderer.h, Texture.h)
│   ├── 📂 audio/          # Audio (AudioManager.h, Sound.h)
│   ├── 📂 input/          # Input (InputManager.h)
│   ├── 📂 scene/          # Scenes (Scene.h, SceneManager.h)
│   ├── 📂 components/     # ECS Components (Transform, Sprite, etc.)
│   ├── 📂 systems/        # ECS Systems (Render, Physics, etc.)
│   ├── 📂 editor/         # GUI Editor (GameEditor.h)
│   └── 📂 utils/          # Utilities (ResourceManager.h)
├── 📂 assets/             # Game assets (sprites, audio, maps)
├── 📂 examples/           # Example games and tutorials
├── 📂 external/           # Third-party libraries (ImGui)
├── 📂 build/              # Build output directory
├── 🔧 CMakeLists.txt      # Build configuration
├── 📋 requirements.txt    # Dependency list
└── 📖 SETUP.md           # Detailed setup instructions
```

## 🎮 Example Games

### Zelda-like Adventure
```cpp
#include "examples/zelda/ZeldaGame.h"

int main() {
    auto game = std::make_shared<ZeldaGame>();
    game->initialize();
    
    Engine::getInstance().setActiveScene(game);
    Engine::getInstance().run();
}
```

**Features:**
- Top-down exploration
- Sword combat system
- Collectible items and hearts
- Multiple interconnected rooms
- Save/load progression

### Platformer Demo
- Side-scrolling camera
- Jump and run mechanics
- Enemy AI patterns
- Collectible coins and power-ups

## 🔧 Advanced Features

### Custom Components
```cpp
// Define new component
class HealthComponent : public Component {
public:
    int maxHealth = 100;
    int currentHealth = 100;
    bool invulnerable = false;
};

// Register in scene
scene->registerComponent<HealthComponent>();
```

### Custom Systems
```cpp
class HealthSystem : public System {
public:
    void update(float deltaTime) override {
        for (auto entity : entities) {
            auto& health = scene->getComponent<HealthComponent>(entity);
            // Process health logic
        }
    }
};
```

### Asset Pipeline
- **Automatic resource loading** with caching
- **Texture atlas support** for efficient rendering  
- **Audio streaming** for large music files
- **JSON-based scene serialization**

## 🎨 Editor Features

### Scene Editing
- **Drag-and-drop** entity positioning
- **Multi-selection** for batch operations
- **Copy/paste** entities with all components
- **Prefab system** for reusable objects

### Asset Management
- **Automatic asset detection** in folders
- **Preview thumbnails** for images and audio
- **Drag-and-drop** asset assignment
- **Asset dependency tracking**

### Debugging Tools
- **Real-time component editing** while game runs
- **Performance profiler** with frame timing
- **Memory usage tracking**
- **Collision shape visualization**

## 🚀 Performance

- **60+ FPS** on modest hardware
- **Efficient batched rendering** reduces draw calls
- **Spatial partitioning** for collision optimization
- **Memory pooling** for frequent allocations
- **Asset streaming** for large worlds

## 🤝 Contributing

We welcome contributions! Areas where help is needed:

- **Platform support** (Linux, macOS)
- **Additional component types** (Animation, Particle Systems)
- **Editor improvements** (Visual scripting, Better asset pipeline)
- **Documentation and tutorials**
- **Example games and demos**

## 📚 Documentation

- [📖 Setup Guide](SETUP.md) - Detailed installation instructions
- [🎮 Game Examples](examples/) - Complete example projects
- [📝 API Reference](docs/api/) - Complete API documentation
- [🎥 Video Tutorials](docs/tutorials/) - Step-by-step video guides

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **SDL2** - Cross-platform multimedia library
- **Dear ImGui** - Immediate mode GUI framework
- **GLM** - OpenGL Mathematics library
- **nlohmann/json** - Modern C++ JSON library

---

**Ready to create your next 2D adventure game? [Get started now!](SETUP.md)**
