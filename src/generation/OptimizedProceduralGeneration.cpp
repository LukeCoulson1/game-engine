#include "OptimizedProceduralGeneration.h"
#include "../core/Engine.h"
#include "../utils/ResourceManager.h"
#include <iostream>

void OptimizedProceduralGeneration::generateDungeonToSceneWindow(SceneWindow* sceneWindow, int width, int height, 
                                                               unsigned int seed, const DungeonGenerator::DungeonSettings& settings) {
    if (!sceneWindow) return;
    
    // Create map
    auto map = std::make_shared<ProceduralMap>(width, height);
    
    // Set up sprite manager
    auto spriteManager = createSpriteManager(GenerationTheme::Dungeon);
    map->setSpriteManager(spriteManager);
      // Generate dungeon
    auto dungeonGen = std::make_shared<DungeonGenerator>(settings);
    dungeonGen->setSeed(seed);
    dungeonGen->generate(*map);
    
    // Debug: Log generation results
    int nonEmptyTiles = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Tile& tile = map->getTile(x, y);
            if (tile.type != TileType::Empty) {
                nonEmptyTiles++;
            }
        }
    }
    std::cout << "DEBUG: Generated " << nonEmptyTiles << " non-empty tiles out of " << (width * height) << " total tiles" << std::endl;
    
    // Assign to scene window for efficient rendering
    sceneWindow->setProceduralMap(map);
    
    // Only create entities for interactive objects (optional)
    addGameplayEntities(sceneWindow, map);
    
    // Center camera on generated content
    Vector2 mapCenter = Vector2((width * 32) / 2.0f, (height * 32) / 2.0f);
    Vector2 cameraPos = Vector2(mapCenter.x - 400, mapCenter.y - 300);
    sceneWindow->setCameraPosition(cameraPos);
    sceneWindow->setDirty(true);
    
    // Force viewport refresh to ensure map is rendered
    sceneWindow->setZoomLevel(sceneWindow->getZoomLevel()); // Trigger refresh
}

void OptimizedProceduralGeneration::generateCityToSceneWindow(SceneWindow* sceneWindow, int width, int height, 
                                                            unsigned int seed, const CityGenerator::CitySettings& settings) {
    if (!sceneWindow) return;
    
    // Create map
    auto map = std::make_shared<ProceduralMap>(width, height);
    
    // Set up sprite manager
    auto spriteManager = createSpriteManager(GenerationTheme::City);
    map->setSpriteManager(spriteManager);
    
    // Generate city
    auto cityGen = std::make_shared<CityGenerator>(settings);
    cityGen->setSeed(seed);
    cityGen->generate(*map);
    
    // Assign to scene window for efficient rendering
    sceneWindow->setProceduralMap(map);
    
    // Only create entities for interactive objects (optional)
    addGameplayEntities(sceneWindow, map);
    
    // Center camera on generated content
    Vector2 mapCenter = Vector2((width * 32) / 2.0f, (height * 32) / 2.0f);
    Vector2 cameraPos = Vector2(mapCenter.x - 400, mapCenter.y - 300);
    sceneWindow->setCameraPosition(cameraPos);
    sceneWindow->setDirty(true);
    
    // Force viewport refresh to ensure map is rendered
    sceneWindow->setZoomLevel(sceneWindow->getZoomLevel()); // Trigger refresh
}

void OptimizedProceduralGeneration::generateTerrainToSceneWindow(SceneWindow* sceneWindow, int width, int height, 
                                                               unsigned int seed, const TerrainGenerator::TerrainSettings& settings) {
    if (!sceneWindow) return;
    
    // Create map
    auto map = std::make_shared<ProceduralMap>(width, height);
    
    // Set up sprite manager
    auto spriteManager = createSpriteManager(GenerationTheme::Terrain);
    map->setSpriteManager(spriteManager);
    
    // Generate terrain
    auto terrainGen = std::make_shared<TerrainGenerator>(settings);
    terrainGen->setSeed(seed);
    terrainGen->generate(*map);
    
    // Assign to scene window for efficient rendering
    sceneWindow->setProceduralMap(map);
    
    // Only create entities for interactive objects (optional)
    addGameplayEntities(sceneWindow, map);
    
    // Center camera on generated content
    Vector2 mapCenter = Vector2((width * 32) / 2.0f, (height * 32) / 2.0f);
    Vector2 cameraPos = Vector2(mapCenter.x - 400, mapCenter.y - 300);
    sceneWindow->setCameraPosition(cameraPos);
    sceneWindow->setDirty(true);
    
    // Force viewport refresh to ensure map is rendered
    sceneWindow->setZoomLevel(sceneWindow->getZoomLevel()); // Trigger refresh
}

void OptimizedProceduralGeneration::addGameplayEntities(SceneWindow* sceneWindow, std::shared_ptr<ProceduralMap> map) {
    if (!sceneWindow || !map) return;
    
    auto scene = sceneWindow->getScene();
    if (!scene) return;
    
    // Only create entities for special tiles (entrance, exit, interactive objects)
    // This dramatically reduces entity count from thousands to just a few
    for (int y = 0; y < map->getHeight(); ++y) {
        for (int x = 0; x < map->getWidth(); ++x) {
            const Tile& tile = map->getTile(x, y);
            
            // Only create entities for interactive/special tiles
            if (tile.type == TileType::Entrance || tile.type == TileType::Exit) {
                EntityID entity = scene->createEntity();
                
                // Add transform
                Vector2 worldPos = map->getWorldPosition(x, y);
                scene->addComponent<Transform>(entity, Transform(worldPos));
                
                // Mark as procedurally generated (gameplay element)
                scene->addComponent<ProceduralGenerated>(entity, 
                    ProceduralGenerated(ProceduralGenerated::GenerationType::GameplayElement, x, y));
                
                // Add sprite (these special tiles will be rendered as entities, not tiles)
                auto& engine = Engine::getInstance();
                auto resourceManager = engine.getResourceManager();
                if (resourceManager && !tile.spriteName.empty()) {
                    Sprite sprite;
                    auto texture = resourceManager->loadTexture(tile.spriteName);
                    if (texture) {
                        sprite.texture = texture;
                        sprite.sourceRect = Rect(0, 0, texture->getWidth(), texture->getHeight());
                    }
                    scene->addComponent<Sprite>(entity, sprite);
                }
                
                // Set entity name
                std::string entityName = (tile.type == TileType::Entrance) ? "Entrance" : "Exit";
                entityName += "_" + std::to_string(x) + "_" + std::to_string(y);
                scene->setEntityName(entity, entityName);
            }
        }
    }
}

std::shared_ptr<TileSpriteManager> OptimizedProceduralGeneration::createSpriteManager(GenerationTheme theme) {
    auto spriteManager = std::make_shared<TileSpriteManager>();
    spriteManager->setTheme(theme);
    std::cout << "DEBUG: Created TileSpriteManager with theme " << static_cast<int>(theme) << std::endl;
    return spriteManager;
}
