# Multi-Window Scene Procedural Generation Fix

## Issue Resolution

### Problem
When creating entities or generating procedural content (dungeons, cities, terrain), the content was not appearing in the selected scene window. This was happening because the procedural generation and entity creation systems were still using the legacy `m_currentScene` instead of working with the active scene window.

### Root Cause
The procedural generation methods in `GameEditor::showProceduralGeneration()` were:
1. Creating entirely new scenes with `createNewScene()`
2. Using the global `m_currentScene` instead of the active scene window
3. Not integrating with the multi-window scene system

### Solution Implemented

#### 1. Updated Procedural Generation Methods
**Before:**
```cpp
if (m_currentScene) {
    // Clear current scene
    m_currentScene = nullptr;
    createNewScene();
    
    // Generate content
    m_currentMap = m_proceduralManager->generateDungeon(mapWidth, mapHeight, seed);
    m_proceduralManager->generateMapToScene(m_currentMap, m_currentScene.get());
}
```

**After:**
```cpp
if (!m_activeSceneWindow || !m_activeSceneWindow->getScene()) {
    m_consoleMessages.push_back("❌ No active scene window! Create a scene window first.");
    return;
}

auto activeScene = m_activeSceneWindow->getScene();

// Clear current scene content
auto entities = activeScene->getAllLivingEntities();
for (EntityID entity : entities) {
    activeScene->destroyEntity(entity);
}

// Generate content in active scene
m_currentMap = m_proceduralManager->generateDungeon(mapWidth, mapHeight, seed);
m_proceduralManager->generateMapToScene(m_currentMap, activeScene.get());

// Center camera in the active scene window
Vector2 mapCenter = Vector2((mapWidth * 32) / 2.0f, (mapHeight * 32) / 2.0f);
Vector2 cameraPos = Vector2(mapCenter.x - 400, mapCenter.y - 300);
m_activeSceneWindow->setCameraPosition(cameraPos);

// Mark scene as dirty
m_activeSceneWindow->setDirty(true);
```

#### 2. Enhanced UI Feedback
Added active scene indication to the Procedural Generation panel:

```cpp
// Show which scene window is active
if (m_activeSceneWindow) {
    ImGui::Text("Target Scene: %s", m_activeSceneWindow->getTitle().c_str());
    if (m_activeSceneWindow->isDirty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "*");
    }
    ImGui::Separator();
} else {
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "⚠️ No active scene");
    ImGui::Text("Create a scene window to generate content");
    return;
}
```

#### 3. Updated All Generation Types
Applied the same fix to all three procedural generation methods:
- **Dungeon Generation** (`Generate Dungeon` button)
- **City Generation** (`Generate City` button)  
- **Terrain Generation** (`Generate Terrain` button)

#### 4. Enhanced Console Messages
Updated console output to include scene context:
```cpp
m_consoleMessages.push_back("Generated dungeon " + std::to_string(mapWidth) + "x" + std::to_string(mapHeight) + " (seed: " + std::to_string(seed) + ") in " + m_activeSceneWindow->getTitle());
```

### Features Fixed

#### ✅ Procedural Generation
- **Target Scene Detection**: Generation now checks for active scene window
- **Content Placement**: Generated content appears in the active scene
- **Camera Positioning**: Camera centers on generated content in the correct scene window
- **Scene State**: Scene is marked as dirty (*) after generation

#### ✅ Entity Creation  
- **Active Scene Integration**: Entity creation uses the active scene window
- **Selection State**: Created entities are properly selected in the active scene
- **Scene Tracking**: Console shows which scene entities were created in

#### ✅ UI Integration
- **Active Scene Display**: Procedural Generation panel shows target scene name
- **Warning Messages**: Clear feedback when no scene is active
- **Scene Hierarchy**: Shows active scene and its entities
- **Inspector**: Works with selected entity from active scene

#### ✅ State Management
- **Dirty Flag**: Scene windows show (*) when modified
- **Independent State**: Each scene maintains its own content and state
- **Console Context**: Messages include which scene was modified

### How to Use

#### Basic Workflow
1. **Create Scene Windows**: Use `File → New Scene` or `Window → New Scene Window`
2. **Activate Scene**: Click on the scene window you want to modify
3. **Generate Content**: Use Procedural Generation panel - content appears in active scene
4. **Switch Scenes**: Click different scene windows to work on multiple projects

#### Procedural Generation
1. **Open Panel**: View → Procedural Generation (or ensure it's visible)
2. **Check Target**: Panel shows "Target Scene: [Scene Name]" at the top
3. **Configure Settings**: Set dungeon/city/terrain parameters
4. **Generate**: Click generation button - content appears in active scene
5. **Camera**: Automatically centers on generated content

#### Entity Management
1. **Select Scene**: Click on the scene window to make it active
2. **Create Entities**: Use Scene Hierarchy or Edit menu
3. **Edit Properties**: Use Inspector panel with selected entities
4. **Assign Assets**: Use Asset Browser to assign textures

### Error Handling

#### No Active Scene
- **Detection**: System checks for active scene before generation
- **User Feedback**: Clear warning message displayed
- **Prevention**: Generation buttons are effectively disabled

#### Scene State Conflicts
- **Content Clearing**: Existing entities are removed before generation
- **User Notice**: Console messages indicate content replacement
- **Dirty State**: Scene marked as modified after changes

### Testing Instructions

#### Test Procedural Generation
1. Create multiple scene windows
2. Click on Scene 1, generate a dungeon
3. Click on Scene 2, generate a city  
4. Verify each scene has different content
5. Check console messages include scene names

#### Test Entity Creation
1. Select Scene 1, create some entities
2. Select Scene 2, create different entities
3. Switch between scenes to verify independent content
4. Use Inspector to modify entities in each scene

### Benefits

#### For Users
- **Intuitive Workflow**: Content appears where expected
- **Multiple Projects**: Work on different scenes simultaneously  
- **Clear Feedback**: Always know which scene is being modified
- **Independent Work**: Each scene maintains its own state

#### For Development
- **Consistent Architecture**: All systems use active scene window
- **Maintainable Code**: Clear separation of scene-specific operations
- **Extensible Design**: Easy to add new generation features
- **Robust Error Handling**: Graceful handling of edge cases

### Technical Implementation

#### Key Components Modified
- `GameEditor::showProceduralGeneration()` - Main procedural generation UI
- `GameEditor::createEntity()` - Entity creation (already fixed)
- `GameEditor::deleteEntity()` - Entity deletion (already fixed)
- UI panels integration for active scene detection

#### Scene Window Integration
- Active scene detection: `m_activeSceneWindow`
- Scene access: `m_activeSceneWindow->getScene()`
- Camera control: `m_activeSceneWindow->setCameraPosition()`
- State management: `m_activeSceneWindow->setDirty(true)`

#### Error Prevention
- Null checks for active scene window
- Clear user feedback for missing prerequisites
- Graceful fallback behavior

This fix ensures that the multi-window scene system works seamlessly with all content generation features, providing users with an intuitive and powerful editing experience.
