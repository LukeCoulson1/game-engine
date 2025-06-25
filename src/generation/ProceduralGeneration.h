#pragma once

#include <vector>
#include <random>
#include <memory>
#include <unordered_map>
#include <string>
#include "../graphics/Renderer.h"
#include "../components/Components.h"

// Forward declarations
class Scene;

// Generation themes for different sprite sets
enum class GenerationTheme {
    Default,
    Dungeon,
    City,
    Terrain,
    Fantasy,
    Modern
};

// Tile types for procedural generation
enum class TileType {
    Empty = 0,
    Wall,
    Floor,
    Door,
    Water,
    Grass,
    Stone,
    Tree,
    Building,
    Road,
    House,
    Shop,
    Entrance,
    Exit
};

// Tile sprite mapping manager
class TileSpriteManager {
public:
    TileSpriteManager();
    
    // Theme management
    void setTheme(GenerationTheme theme);
    GenerationTheme getCurrentTheme() const { return m_currentTheme; }
    
    // Sprite mapping
    std::string getSprite(TileType type) const;
    void setSprite(TileType type, const std::string& spritePath);
    void setThemeSprite(GenerationTheme theme, TileType type, const std::string& spritePath);
    
    // Batch sprite loading
    void loadDefaultSprites();
    void loadDungeonSprites();
    void loadCitySprites();
    void loadTerrainSprites();
      // Dynamic management for editor
    std::vector<std::string> getAvailableImages() const;
    void refreshAvailableImages();
    bool imageExists(const std::string& imagePath) const;
    void saveThemeMapping(GenerationTheme theme) const;
    void loadThemeMapping(GenerationTheme theme);
    
    // Helper for asset paths
    std::string getAssetPath(const std::string& relativePath) const;
    
    // Get all tile types for iteration
    static std::vector<TileType> getAllTileTypes();
    static std::string getTileTypeName(TileType type);
      // Validation
    bool hasSpriteForType(TileType type) const;
    std::vector<std::string> getMissingSprites() const;
    
    // Nomenclature-based automatic assignment
    void autoAssignByNomenclature();
    void autoAssignThemeByNomenclature(GenerationTheme theme);
    bool assignSpriteByNaming(TileType type, const std::string& basePath = "assets/tiles/");
    std::vector<std::string> findSpritesByPattern(const std::string& pattern, const std::string& basePath = "assets/tiles/") const;
    
    // Nomenclature patterns
    std::vector<std::string> getTileTypePatterns(TileType type) const;
    static std::string getThemePrefixName(GenerationTheme theme);
    
private:
    GenerationTheme m_currentTheme;
    std::unordered_map<TileType, std::string> m_currentMapping;
    std::unordered_map<GenerationTheme, std::unordered_map<TileType, std::string>> m_themeMapping;
    
    void updateCurrentMapping();
};

// Tile data structure
struct Tile {
    TileType type;
    Vector2 position;
    bool walkable;
    std::string spriteName;
    
    Tile(TileType t = TileType::Empty, Vector2 pos = Vector2(0, 0)) 
        : type(t), position(pos), walkable(true), spriteName("") {
        // spriteName will be set by TileSpriteManager
    }
    
    void updateProperties(const TileSpriteManager& spriteManager);
};

// Grid-based map structure
class ProceduralMap {
public:
    ProceduralMap(int width, int height);
    ~ProceduralMap() = default;
    
    // Map access
    Tile& getTile(int x, int y);
    const Tile& getTile(int x, int y) const;
    void setTile(int x, int y, TileType type);
    bool isValidPosition(int x, int y) const;
    
    // Map properties
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    Vector2 getWorldPosition(int x, int y) const;
    Vector2 getWorldPosition(int x, int y, int tileSize) const;
    Vector2 getGridPosition(const Vector2& worldPos) const;
    
    // Sprite management
    void setSpriteManager(std::shared_ptr<TileSpriteManager> spriteManager);
    std::shared_ptr<TileSpriteManager> getSpriteManager() const { return m_spriteManager; }
    void updateAllTileSprites();
    
    // Generation
    void clear(TileType fillType = TileType::Empty);
    void generateToScene(Scene* scene, int tileSize = 32);
    
private:
    int m_width, m_height;
    std::vector<std::vector<Tile>> m_tiles;
    std::shared_ptr<TileSpriteManager> m_spriteManager;
};

// Base class for procedural generators
class ProceduralGenerator {
public:
    ProceduralGenerator(unsigned int seed = 0);
    virtual ~ProceduralGenerator() = default;
    
    virtual void generate(ProceduralMap& map) = 0;
    
    void setSeed(unsigned int seed);
    unsigned int getSeed() const { return m_seed; }
    
protected:
    std::mt19937 m_rng;
    unsigned int m_seed;
    
    // Utility functions
    int randomInt(int min, int max);
    float randomFloat(float min = 0.0f, float max = 1.0f);
    bool randomBool(float probability = 0.5f);
    Vector2 randomPosition(int minX, int minY, int maxX, int maxY);
};

// Dungeon generator using cellular automata and room-corridor approach
class DungeonGenerator : public ProceduralGenerator {
public:
    struct DungeonSettings {
        int minRoomSize = 4;
        int maxRoomSize = 12;
        int numRooms = 8;
        float wallDensity = 0.45f;
        int smoothingIterations = 4;
        bool useRoomApproach = true;
        bool generateTreasure = true;
        int corridorWidth = 1;
    };
    
    DungeonGenerator(const DungeonSettings& settings = DungeonSettings());
    void generate(ProceduralMap& map) override;
    
    void setSettings(const DungeonSettings& settings) { m_settings = settings; }
    const DungeonSettings& getSettings() const { return m_settings; }

private:
    DungeonSettings m_settings;
    
    struct Room {
        int x, y, width, height;
        Vector2 center() const { return Vector2(x + width/2, y + height/2); }
    };
    
    void generateRooms(ProceduralMap& map);
    void generateCellularAutomata(ProceduralMap& map);
    void smoothMap(ProceduralMap& map);
    void connectRooms(ProceduralMap& map, const std::vector<Room>& rooms);
    void createCorridor(ProceduralMap& map, Vector2 start, Vector2 end);
    void addDetails(ProceduralMap& map);
    bool isRoomValid(const Room& room, const std::vector<Room>& existingRooms, int mapWidth, int mapHeight);
};

// City generator with roads, buildings, and districts
class CityGenerator : public ProceduralGenerator {
public:
    struct CitySettings {
        int blockSize = 8;
        int roadWidth = 2;
        float buildingDensity = 0.7f;
        int minBuildingSize = 2;
        int maxBuildingSize = 6;
        bool generateDistricts = true;
        bool addLandmarks = true;
        int numLandmarks = 3;
    };
    
    CityGenerator(const CitySettings& settings = CitySettings());
    void generate(ProceduralMap& map) override;
    
    void setSettings(const CitySettings& settings) { m_settings = settings; }
    const CitySettings& getSettings() const { return m_settings; }

private:
    CitySettings m_settings;
    
    void generateRoadNetwork(ProceduralMap& map);
    void generateBuildings(ProceduralMap& map);
    void generateDistricts(ProceduralMap& map);
    void addLandmarks(ProceduralMap& map);
    void placeBuildingInBlock(ProceduralMap& map, int blockX, int blockY);
};

// Terrain generator for outdoor environments
class TerrainGenerator : public ProceduralGenerator {
public:
    struct TerrainSettings {
        float waterLevel = 0.3f;
        float grassLevel = 0.6f;
        float stoneLevel = 0.8f;
        int octaves = 4;
        float persistence = 0.5f;
        float scale = 0.1f;
        bool addTrees = true;
        float treeDensity = 0.1f;
    };
    
    TerrainGenerator(const TerrainSettings& settings = TerrainSettings());
    void generate(ProceduralMap& map) override;
    
    void setSettings(const TerrainSettings& settings) { m_settings = settings; }
    const TerrainSettings& getSettings() const { return m_settings; }

private:
    TerrainSettings m_settings;
    
    float generateNoise(float x, float y);
    float perlinNoise(float x, float y);
    TileType getTerrainType(float noiseValue);
    void addVegetation(ProceduralMap& map);
};

// Manager class for procedural generation
class ProceduralGenerationManager {
public:
    ProceduralGenerationManager();
    ~ProceduralGenerationManager() = default;
    
    // Generator management
    void setDungeonGenerator(std::shared_ptr<DungeonGenerator> generator);
    void setCityGenerator(std::shared_ptr<CityGenerator> generator);
    void setTerrainGenerator(std::shared_ptr<TerrainGenerator> generator);
    
    // Sprite theme management
    void setSpriteManager(std::shared_ptr<TileSpriteManager> spriteManager);
    std::shared_ptr<TileSpriteManager> getSpriteManager() const { return m_spriteManager; }
    void setGenerationTheme(GenerationTheme theme);
    GenerationTheme getCurrentTheme() const;
    
    // Generation methods with automatic theme application
    std::shared_ptr<ProceduralMap> generateDungeon(int width, int height, unsigned int seed = 0);
    std::shared_ptr<ProceduralMap> generateCity(int width, int height, unsigned int seed = 0);
    std::shared_ptr<ProceduralMap> generateTerrain(int width, int height, unsigned int seed = 0);
    
    // Utility
    void generateMapToScene(std::shared_ptr<ProceduralMap> map, Scene* scene, int tileSize = 32);
    
private:
    std::shared_ptr<DungeonGenerator> m_dungeonGenerator;
    std::shared_ptr<CityGenerator> m_cityGenerator;
    std::shared_ptr<TerrainGenerator> m_terrainGenerator;
    std::shared_ptr<TileSpriteManager> m_spriteManager;
};
