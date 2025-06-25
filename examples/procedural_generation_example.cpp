// Example: How to use procedural generation in your game code

#include "generation/ProceduralGeneration.h"
#include "scene/Scene.h"

void runProceduralGeneration() {
    // Create a procedural generation manager
    ProceduralGenerationManager procManager;
    
    // Method A: Generate a dungeon
    {
        // Configure dungeon settings
        DungeonGenerator::DungeonSettings dungeonSettings;
        dungeonSettings.minRoomSize = 4;
        dungeonSettings.maxRoomSize = 12;
        dungeonSettings.numRooms = 8;
        dungeonSettings.wallDensity = 0.45f;
        dungeonSettings.useRoomApproach = true;
        dungeonSettings.generateTreasure = true;
        
        // Create dungeon generator
        auto dungeonGen = std::make_shared<DungeonGenerator>(dungeonSettings);
        procManager.setDungeonGenerator(dungeonGen);
        
        // Generate dungeon map (50x50 tiles, seed 12345)
        auto dungeonMap = procManager.generateDungeon(50, 50, 12345);
        
        // Apply to scene (assuming you have a scene)
        Scene* currentScene = /* your scene */;
        procManager.generateMapToScene(dungeonMap, currentScene, 32); // 32px tile size
        
        std::cout << "Generated dungeon: " << dungeonMap->getWidth() 
                  << "x" << dungeonMap->getHeight() << " tiles\n";
    }
    
    // Method B: Generate a city
    {
        CityGenerator::CitySettings citySettings;
        citySettings.blockSize = 8;
        citySettings.roadWidth = 2;
        citySettings.buildingDensity = 0.7f;
        citySettings.generateDistricts = true;
        citySettings.addLandmarks = true;
        
        auto cityGen = std::make_shared<CityGenerator>(citySettings);
        procManager.setCityGenerator(cityGen);
        
        auto cityMap = procManager.generateCity(60, 60, 54321);
        // Apply to scene...
    }
    
    // Method C: Generate terrain
    {
        TerrainGenerator::TerrainSettings terrainSettings;
        terrainSettings.waterLevel = 0.3f;
        terrainSettings.grassLevel = 0.6f;
        terrainSettings.stoneLevel = 0.8f;
        terrainSettings.addTrees = true;
        terrainSettings.treeDensity = 0.1f;
        
        auto terrainGen = std::make_shared<TerrainGenerator>(terrainSettings);
        procManager.setTerrainGenerator(terrainGen);
        
        auto terrainMap = procManager.generateTerrain(80, 80, 98765);
        // Apply to scene...
    }
    
    // Method D: Direct generator usage (lower level)
    {
        // Create map manually
        ProceduralMap map(30, 30);
        
        // Use generator directly
        DungeonGenerator generator;
        generator.setSeed(42);
        generator.generate(map);
        
        // Access individual tiles
        for (int y = 0; y < map.getHeight(); ++y) {
            for (int x = 0; x < map.getWidth(); ++x) {
                const Tile& tile = map.getTile(x, y);
                if (tile.type == TileType::Wall) {
                    std::cout << "#";
                } else if (tile.type == TileType::Floor) {
                    std::cout << ".";
                } else {
                    std::cout << " ";
                }
            }
            std::cout << "\n";
        }
    }
}
