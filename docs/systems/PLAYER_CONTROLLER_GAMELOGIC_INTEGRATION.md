# Player Controller Integration in Game Logic Window

## Overview
Added PlayerSystem integration to the GameLogicWindow to enable real-time testing of player controller logic during game development.

## Changes Made

### 1. GameLogicWindow.h
- Added `#include "../systems/PlayerSystem.h"`
- Added `std::unique_ptr<PlayerSystem> m_playerSystem = nullptr;` member variable

### 2. GameLogicWindow.cpp
- Added `#include <SDL2/SDL.h>` for keyboard state access
- Modified constructor to initialize PlayerSystem:
  ```cpp
  m_playerSystem = std::make_unique<PlayerSystem>();
  ```

### 3. Enhanced updateRuntime() Method
- Added player input handling and PlayerSystem update:
  ```cpp
  // Get keyboard state and handle player input
  const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
  m_playerSystem->handleInput(scene, keyboardState, deltaTime);
  m_playerSystem->update(scene, deltaTime);
  ```

### 4. Updated Default Code Template
- Changed default user code to focus on player controller testing
- Added clear instructions for setting up player controller components
- Included information about WASD controls and running mode

### 5. Added Player Controller Code Template
- Created "Player Controller Demo" template with:
  - Instructions for setup
  - Example code for detecting player controller entities
  - Custom behavior examples (color change when running)
  - Comments about available controls

## How It Works

### Input Processing Flow
1. GameLogicWindow runtime calls `updateRuntime()` every frame
2. Gets SDL keyboard state via `SDL_GetKeyboardState()`
3. Passes keyboard state to `PlayerSystem::handleInput()`
4. PlayerSystem processes input for all entities with PlayerController components
5. PlayerSystem updates physics, movement, and state for player entities
6. Changes are immediately visible in the Scene Window

### Supported Controls
- **WASD**: Movement (top-down style)
- **Shift**: Run (increases speed, changes entity color in demo)
- **Space**: Jump (if physics components are present)
- **1-4**: Use abilities (if PlayerAbilities component is present)

### Component Requirements
For an entity to be controlled by the player system, it needs:
- **Transform**: Position, scale, rotation
- **PlayerController**: Control scheme, movement settings, input state
- **Optional Components**:
  - **Sprite**: For visual representation and color changes
  - **PlayerPhysics**: For advanced movement and jumping
  - **Collider**: For collision detection
  - **PlayerAbilities**: For special abilities

## Usage Instructions

1. **Setup Entity**:
   - Create entity in Scene Window
   - Select entity in Inspector
   - Add PlayerController component

2. **Test Player Controls**:
   - Open Game Logic Window
   - Press F5 to start runtime
   - Use WASD to move entity
   - Watch real-time movement in Scene Window

3. **Customize Behavior**:
   - Use "Player Controller Demo" template for examples
   - Add custom logic in `updateGame()` function
   - Test changes in real-time without stopping runtime

## Benefits

### Real-Time Testing
- Immediate feedback on player controller changes
- No need to build separate runtime executable
- Live debugging of movement logic

### Integrated Development
- Edit component properties in Inspector
- Test immediately in Game Logic Window
- See results in Scene Window simultaneously

### Flexible Controls
- Easy to modify control schemes in Inspector
- Support for different movement types (top-down, platformer, etc.)
- Extensible for custom input handling

## Testing
Use `test_player_controller_gamelogic.bat` to verify:
1. Player controller component addition
2. Real-time WASD movement
3. Running mode (Shift+WASD)
4. Scene Window updates
5. Game Logic Window integration

## Future Enhancements
- Gamepad support integration
- Visual debugging of input state
- Recording/playback of player input
- Multiplayer testing support
