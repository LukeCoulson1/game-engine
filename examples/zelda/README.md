# Example Zelda-like Game

This folder contains an example implementation of a simple Zelda-like game using the engine.

## Features Implemented

### Player Character
- Top-down movement with WASD keys
- Sprite animation (walking cycles)
- Collision detection with environment

### World
- Tile-based map system
- Static obstacles and walls
- Interactive objects (chests, doors, NPCs)

### Game Systems
- Simple inventory system
- Health/hearts system
- Basic combat mechanics
- Sound effects and music

### Level Design
- Multiple connected screens/rooms
- Transition system between areas
- Save/load game state

## Files

- `ZeldaGameScene.h/cpp` - Main game scene implementation
- `PlayerController.h/cpp` - Player movement and input handling
- `InventorySystem.h/cpp` - Item collection and management
- `GameComponents.h` - Game-specific components
- `TileMap.h/cpp` - Tile-based world system

## Assets Used

- `sprites/player.png` - Player character sprite sheet
- `sprites/tileset.png` - Environment tiles
- `sprites/items.png` - Collectible items
- `audio/bgm.ogg` - Background music
- `audio/pickup.wav` - Item pickup sound
- `maps/overworld.json` - Level data

## Controls

- **WASD** - Move player
- **E** - Interact with objects
- **I** - Open inventory
- **ESC** - Pause menu

## Building

This example game is built automatically with the main engine. Run with:

```
GameRuntime.exe --scene=examples/zelda/overworld.json
```

Or load the scene through the editor.
