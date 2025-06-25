#pragma once

#include "../generation/ProceduralGeneration.h"
#include "../editor/SceneWindow.h"

// Optimized procedural generation that uses efficient tile rendering
// instead of creating thousands of entities
class OptimizedProceduralGeneration {
public:
    // Generate map and assign to scene window for efficient rendering
    static void generateDungeonToSceneWindow(SceneWindow* sceneWindow, int width, int height, 
                                           unsigned int seed, const DungeonGenerator::DungeonSettings& settings);
    
    static void generateCityToSceneWindow(SceneWindow* sceneWindow, int width, int height, 
                                        unsigned int seed, const CityGenerator::CitySettings& settings);
    
    static void generateTerrainToSceneWindow(SceneWindow* sceneWindow, int width, int height, 
                                           unsigned int seed, const TerrainGenerator::TerrainSettings& settings);
    
    // For creating interactive objects (players, enemies, etc.) as actual entities
    static void addGameplayEntities(SceneWindow* sceneWindow, std::shared_ptr<ProceduralMap> map);
    
private:
    static std::shared_ptr<TileSpriteManager> createSpriteManager(GenerationTheme theme);
};
