# Procedural Map Persistence Fix

## Overview
Fixed the issue where procedural maps and entities were not visible when opening saved scenes. The problem was that procedural map data was not being serialized/deserialized with scene data.

## Problem
When a scene with a procedural map was saved and then loaded:
1. The procedural map data was lost during serialization
2. SceneWindow had no procedural map when the scene was reopened
3. No sprites or tiles were visible in the scene
4. Only regular entities (if any) would appear

## Root Cause
- Procedural maps were stored only in SceneWindow, not in Scene objects
- Scene serialization did not include procedural map data
- When loading scenes, no mechanism existed to restore procedural maps to SceneWindows

## Solution

### 1. Added Procedural Map Support to Scene Class
```cpp
// Scene.h - Added procedural map support
void setProceduralMap(std::shared_ptr<ProceduralMap> map);
std::shared_ptr<ProceduralMap> getProceduralMap() const;
bool hasProceduralMap() const;

private:
    std::shared_ptr<ProceduralMap> m_proceduralMap;
```

### 2. Enhanced SceneWindow to Store Maps in Scene
```cpp
// SceneWindow.cpp - Modified setProceduralMap
void SceneWindow::setProceduralMap(std::shared_ptr<ProceduralMap> map) {
    m_proceduralMap = map;
    if (m_tileRenderer) {
        m_tileRenderer->setMap(map);
    }
    
    // Store in scene for persistence
    if (m_scene) {
        m_scene->setProceduralMap(map);
    }
}
```

### 3. Automatic Procedural Map Restoration
```cpp
// SceneWindow constructor - Auto-restore from scene
SceneWindow::SceneWindow(...) {
    // ... existing code ...
    
    // Restore procedural map if scene has one
    if (scene && scene->hasProceduralMap()) {
        setProceduralMap(scene->getProceduralMap());
    }
}
```

### 4. Enhanced Scene Serialization

#### Saving Procedural Map Data:
```cpp
// Save procedural map data if present
if (scene->hasProceduralMap()) {
    auto proceduralMap = scene->getProceduralMap();
    json proceduralMapData;
    
    proceduralMapData["width"] = proceduralMap->getWidth();
    proceduralMapData["height"] = proceduralMap->getHeight();
    
    // Save tile data (only non-empty tiles for efficiency)
    json tilesArray = json::array();
    for (int y = 0; y < proceduralMap->getHeight(); ++y) {
        for (int x = 0; x < proceduralMap->getWidth(); ++x) {
            const auto& tile = proceduralMap->getTile(x, y);
            if (tile.type != TileType::Empty) {
                json tileData;
                tileData["x"] = x;
                tileData["y"] = y;
                tileData["type"] = static_cast<int>(tile.type);
                if (!tile.spriteName.empty()) {
                    tileData["sprite"] = tile.spriteName;
                }
                tilesArray.push_back(tileData);
            }
        }
    }
    proceduralMapData["tiles"] = tilesArray;
    
    sceneJsonData["proceduralMap"] = proceduralMapData;
}
```

#### Loading Procedural Map Data:
```cpp
// Load procedural map data if present
if (sceneJsonData.contains("proceduralMap")) {
    auto mapData = sceneJsonData["proceduralMap"];
    
    if (mapData.contains("width") && mapData.contains("height")) {
        int width = mapData["width"];
        int height = mapData["height"];
        
        // Create and populate procedural map
        auto proceduralMap = std::make_shared<ProceduralMap>(width, height);
        
        // Load tiles
        if (mapData.contains("tiles")) {
            for (const auto& tileData : mapData["tiles"]) {
                // Restore tile type and sprite name
            }
        }
        
        // Set sprite manager and assign to scene
        scene->setProceduralMap(proceduralMap);
    }
}
```

## Benefits

### 1. Complete Persistence
- Procedural maps are fully saved and restored with scenes
- All tile data, types, and sprite assignments are preserved
- No data loss when saving/loading scenes

### 2. Seamless User Experience
- Opening a saved scene with procedural content "just works"
- No need to regenerate procedural maps after loading
- Visual consistency between save and load

### 3. Efficient Storage
- Only non-empty tiles are saved (storage optimization)
- JSON format allows for easy debugging and modification
- Preserves sprite names and tile types

### 4. Automatic Restoration
- SceneWindow automatically gets procedural map when scene is opened
- TileRenderer is properly configured with the restored map
- Console logging provides feedback on restoration process

## Technical Details

### Files Modified:
- `Scene.h/cpp`: Added procedural map storage and methods
- `SceneWindow.h/cpp`: Enhanced procedural map handling and auto-restoration
- `SceneManager.cpp`: Added procedural map serialization/deserialization
- Added includes for `ProceduralGeneration.h` where needed

### Scene JSON Structure:
```json
{
  "metadata": { ... },
  "entities": [ ... ],
  "proceduralMap": {
    "width": 100,
    "height": 100,
    "tiles": [
      {
        "x": 5,
        "y": 10,
        "type": 1,
        "sprite": "wall_stone.png"
      }
    ],
    "hasSpriteManager": true
  }
}
```

### Console Output:
- "Restored procedural map to SceneWindow: [name]"
- "Loaded procedural map: WxH with N tiles"
- Provides confirmation of successful restoration

## Testing
Use `test_procedural_map_persistence.bat` to verify:
1. Generate procedural maps in new scenes
2. Save scenes with procedural content
3. Load saved scenes and verify procedural maps appear
4. Check console output for restoration messages

## Future Enhancements
- Save/restore sprite manager theme settings
- Compress tile data for large maps
- Support for procedural map metadata (generation seed, settings)
- Incremental updates for modified procedural maps
