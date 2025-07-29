#include "ProceduralGeneration.h"
#include "../scene/Scene.h"
#include "../core/Engine.h"
#include "../utils/ResourceManager.h"
#include "../utils/ConfigManager.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <filesystem>
#include <iostream>

// TileSpriteManager Implementation
TileSpriteManager::TileSpriteManager() : m_currentTheme(GenerationTheme::Default) {
    loadDefaultSprites();
    loadDungeonSprites();
    loadCitySprites();
    loadTerrainSprites();
    
    // Auto-assign sprites by nomenclature for all themes
    autoAssignThemeByNomenclature(GenerationTheme::Dungeon);
    autoAssignThemeByNomenclature(GenerationTheme::City);
    autoAssignThemeByNomenclature(GenerationTheme::Terrain);
    
    updateCurrentMapping();
}

void TileSpriteManager::setTheme(GenerationTheme theme) {
    m_currentTheme = theme;
    updateCurrentMapping();
}

std::string TileSpriteManager::getSprite(TileType type) const {
    auto it = m_currentMapping.find(type);
    return (it != m_currentMapping.end()) ? it->second : getAssetPath("ground.png"); // Fallback with proper asset path
}

void TileSpriteManager::setSprite(TileType type, const std::string& spritePath) {
    m_currentMapping[type] = spritePath;
}

void TileSpriteManager::setThemeSprite(GenerationTheme theme, TileType type, const std::string& spritePath) {
    m_themeMapping[theme][type] = spritePath;
    if (theme == m_currentTheme) {
        m_currentMapping[type] = spritePath;
    }
}

void TileSpriteManager::loadDefaultSprites() {
    auto& defaultMap = m_themeMapping[GenerationTheme::Default];
    defaultMap[TileType::Empty] = "";
    defaultMap[TileType::Wall] = getAssetPath("wall.png");        // Uses your existing wall.png
    defaultMap[TileType::Floor] = getAssetPath("ground.png");     // Uses your existing ground.png
    defaultMap[TileType::Door] = getAssetPath("ground.png");
    defaultMap[TileType::Water] = getAssetPath("ground.png");     
    defaultMap[TileType::Grass] = getAssetPath("ground.png");
    defaultMap[TileType::Stone] = getAssetPath("wall.png");       // Uses your existing wall.png
    defaultMap[TileType::Tree] = getAssetPath("wall.png");
    defaultMap[TileType::Building] = getAssetPath("wall.png");    // Uses your existing wall.png
    defaultMap[TileType::Road] = getAssetPath("ground.png");      // Uses your existing ground.png
    defaultMap[TileType::House] = getAssetPath("wall.png");
    defaultMap[TileType::Shop] = getAssetPath("wall.png");
    defaultMap[TileType::Entrance] = getAssetPath("ground.png");
    defaultMap[TileType::Exit] = getAssetPath("ground.png");
}

void TileSpriteManager::loadDungeonSprites() {
    auto& dungeonMap = m_themeMapping[GenerationTheme::Dungeon];
    dungeonMap[TileType::Empty] = "";
    dungeonMap[TileType::Wall] = getAssetPath("tiles/dungeon_wall.png");     // Uses generated dungeon wall
    dungeonMap[TileType::Floor] = getAssetPath("tiles/dungeon_floor.png");   // Uses generated dungeon floor  
    dungeonMap[TileType::Door] = getAssetPath("tiles/dungeon_door.png");     // Uses generated door
    dungeonMap[TileType::Water] = getAssetPath("tiles/terrain_water.png");   // Uses generated water
    dungeonMap[TileType::Grass] = getAssetPath("tiles/dungeon_floor.png");   // Uses dungeon floor as grass
    dungeonMap[TileType::Stone] = getAssetPath("tiles/dungeon_wall.png");    // Uses dungeon wall as stone
    dungeonMap[TileType::Tree] = getAssetPath("tiles/dungeon_wall.png");     // Uses dungeon wall as tree
    dungeonMap[TileType::Building] = getAssetPath("tiles/dungeon_wall.png"); // Uses dungeon wall as building
    dungeonMap[TileType::Road] = getAssetPath("tiles/dungeon_floor.png");    // Uses dungeon floor as road
    dungeonMap[TileType::House] = getAssetPath("tiles/dungeon_wall.png");    // Uses dungeon wall as house
    dungeonMap[TileType::Shop] = getAssetPath("tiles/dungeon_wall.png");     // Uses dungeon wall as shop
    dungeonMap[TileType::Entrance] = getAssetPath("tiles/tile_entrance.png");
    dungeonMap[TileType::Exit] = getAssetPath("tiles/tile_exit.png");
}

void TileSpriteManager::loadCitySprites() {
    auto& cityMap = m_themeMapping[GenerationTheme::City];
    cityMap[TileType::Empty] = getAssetPath("tiles/tile_empty.png");
    cityMap[TileType::Wall] = getAssetPath("tiles/city_building.png");
    cityMap[TileType::Floor] = getAssetPath("tiles/city_road.png");
    cityMap[TileType::Door] = getAssetPath("tiles/dungeon_door.png");
    cityMap[TileType::Water] = getAssetPath("tiles/terrain_water.png");
    cityMap[TileType::Grass] = getAssetPath("tiles/terrain_grass.png");
    cityMap[TileType::Stone] = getAssetPath("tiles/terrain_stone.png");
    cityMap[TileType::Tree] = getAssetPath("tiles/terrain_tree.png");
    cityMap[TileType::Building] = getAssetPath("tiles/city_building.png");
    cityMap[TileType::Road] = getAssetPath("tiles/city_road.png");
    cityMap[TileType::House] = getAssetPath("tiles/city_house.png");
    cityMap[TileType::Shop] = getAssetPath("tiles/city_shop.png");
    cityMap[TileType::Entrance] = getAssetPath("tiles/tile_entrance.png");
    cityMap[TileType::Exit] = getAssetPath("tiles/tile_exit.png");
}

void TileSpriteManager::loadTerrainSprites() {
    auto& terrainMap = m_themeMapping[GenerationTheme::Terrain];
    terrainMap[TileType::Empty] = getAssetPath("tiles/tile_empty.png");
    terrainMap[TileType::Wall] = getAssetPath("tiles/terrain_stone.png");
    terrainMap[TileType::Floor] = getAssetPath("tiles/terrain_grass.png");
    terrainMap[TileType::Door] = getAssetPath("tiles/terrain_grass.png");
    terrainMap[TileType::Water] = getAssetPath("tiles/terrain_water.png");
    terrainMap[TileType::Grass] = getAssetPath("tiles/terrain_grass.png");
    terrainMap[TileType::Stone] = getAssetPath("tiles/terrain_stone.png");
    terrainMap[TileType::Tree] = getAssetPath("tiles/terrain_tree.png");
    terrainMap[TileType::Building] = getAssetPath("tiles/terrain_stone.png");
    terrainMap[TileType::Road] = getAssetPath("tiles/terrain_grass.png");
    terrainMap[TileType::House] = getAssetPath("tiles/terrain_stone.png");
    terrainMap[TileType::Shop] = getAssetPath("tiles/terrain_stone.png");
    terrainMap[TileType::Entrance] = getAssetPath("tiles/tile_entrance.png");
    terrainMap[TileType::Exit] = getAssetPath("tiles/tile_exit.png");
}

bool TileSpriteManager::hasSpriteForType(TileType type) const {
    auto it = m_currentMapping.find(type);
    return it != m_currentMapping.end() && !it->second.empty();
}

std::vector<std::string> TileSpriteManager::getMissingSprites() const {
    std::vector<std::string> missing;
    for (const auto& pair : m_currentMapping) {
        if (pair.second.empty()) {
            missing.push_back("TileType::" + std::to_string(static_cast<int>(pair.first)));
        }
    }
    return missing;
}

std::vector<std::string> TileSpriteManager::getAvailableImages() const {
    std::vector<std::string> images;
    
    // Add main asset images
    const char* mainAssets[] = { "wall.png", "ground.png", "player.png", "enemy.png", "coin.png", "tileset.png" };
    for (const auto& asset : mainAssets) {
        images.push_back(std::string("assets/") + asset);
    }
    
    // Add tile assets  
    const char* tileAssets[] = {
        "dungeon_wall.png", "dungeon_floor.png", "dungeon_door.png",
        "city_road.png", "city_house.png", "city_shop.png", "city_building.png",
        "terrain_grass.png", "terrain_water.png", "terrain_stone.png", "terrain_tree.png",
        "tile_empty.png", "tile_entrance.png", "tile_exit.png"
    };
    for (const auto& asset : tileAssets) {
        images.push_back(std::string("assets/tiles/") + asset);
    }
    
    return images;
}

void TileSpriteManager::refreshAvailableImages() {
    // This could scan the filesystem in the future
    // For now, we use the hardcoded list
}

bool TileSpriteManager::imageExists(const std::string& imagePath) const {
    // Check if file exists using filesystem
    try {
        return std::filesystem::exists(imagePath);
    } catch (const std::exception&) {
        // If filesystem check fails, fall back to available images list
        auto availableImages = getAvailableImages();
        return std::find(availableImages.begin(), availableImages.end(), imagePath) != availableImages.end();
    }
}

void TileSpriteManager::saveThemeMapping(GenerationTheme theme) const {
    // This could save to a JSON file or similar in the future
    // For now, we just update the theme mapping in memory
}

void TileSpriteManager::loadThemeMapping(GenerationTheme theme) {
    setTheme(theme);
}

std::vector<TileType> TileSpriteManager::getAllTileTypes() {
    return {
        TileType::Empty, TileType::Wall, TileType::Floor, TileType::Door,
        TileType::Water, TileType::Grass, TileType::Stone, TileType::Tree,
        TileType::Building, TileType::Road, TileType::House, TileType::Shop,
        TileType::Entrance, TileType::Exit
    };
}

std::string TileSpriteManager::getTileTypeName(TileType type) {
    const char* names[] = {
        "Empty", "Wall", "Floor", "Door", "Water", "Grass",
        "Stone", "Tree", "Building", "Road", "House", "Shop", "Entrance", "Exit"
    };
    int index = static_cast<int>(type);
    return (index >= 0 && index < 14) ? names[index] : "Unknown";
}

std::string TileSpriteManager::getAssetPath(const std::string& relativePath) const {
    if (relativePath.empty()) return "";
    
    // Get the configurable asset folder
    std::string assetFolder = ConfigManager::getInstance().getAssetFolder();
    
    // Remove "assets/" prefix if present in relativePath
    std::string cleanPath = relativePath;
    if (cleanPath.length() >= 7 && cleanPath.substr(0, 7) == "assets/") {
        cleanPath = cleanPath.substr(7);
    }
    
    // Ensure asset folder ends with /
    if (!assetFolder.empty() && assetFolder.back() != '/' && assetFolder.back() != '\\') {
        assetFolder += '/';
    }
    
    return assetFolder + cleanPath;
}

void TileSpriteManager::updateCurrentMapping() {
    auto themeIt = m_themeMapping.find(m_currentTheme);
    if (themeIt != m_themeMapping.end()) {
        m_currentMapping = themeIt->second;
    }
}

// Tile Implementation
void Tile::updateProperties(const TileSpriteManager& spriteManager) {
    // Set walkability based on tile type
    switch (type) {
        case TileType::Empty:
        case TileType::Floor:
        case TileType::Door:
        case TileType::Grass:
        case TileType::Road:
        case TileType::Entrance:
        case TileType::Exit:
            walkable = true;
            break;
        case TileType::Wall:
        case TileType::Water:
        case TileType::Stone:
        case TileType::Tree:
        case TileType::Building:
        case TileType::House:
        case TileType::Shop:
            walkable = false;
            break;
    }
    
    // Get sprite from sprite manager
    spriteName = spriteManager.getSprite(type);
    
    // Debug output to track sprite assignment
    if (type != TileType::Empty) {
        std::cout << "DEBUG: Tile(" << static_cast<int>(type) << ") assigned sprite: " << spriteName << std::endl;
    }
}

// Nomenclature-based automatic assignment implementation
void TileSpriteManager::autoAssignByNomenclature() {
    autoAssignThemeByNomenclature(m_currentTheme);
}

void TileSpriteManager::autoAssignThemeByNomenclature(GenerationTheme theme) {
    std::string themePrefix = getThemePrefixName(theme);
    auto& themeMap = m_themeMapping[theme];
    
    // Try to auto-assign each tile type
    for (TileType type : getAllTileTypes()) {
        if (type == TileType::Empty) continue; // Skip empty tiles
        
        // First try theme-specific naming
        if (assignSpriteByNaming(type, "assets/tiles/")) {
            // Success with tiles folder
            continue;
        }
        
        // Try main assets folder for basic types
        if (assignSpriteByNaming(type, "assets/")) {
            // Success with main folder
            continue;
        }
    }
    
    updateCurrentMapping();
}

bool TileSpriteManager::assignSpriteByNaming(TileType type, const std::string& basePath) {
    std::vector<std::string> patterns = getTileTypePatterns(type);
    std::string themePrefix = getThemePrefixName(m_currentTheme);
    
    // Try each pattern for this tile type
    for (const std::string& pattern : patterns) {
        // Try theme-specific first (e.g., "city_wall.png")
        std::string themePattern = themePrefix + "_" + pattern;
        auto candidates = findSpritesByPattern(themePattern, basePath);
        
        if (!candidates.empty()) {
            // Found theme-specific sprite
            auto& themeMap = m_themeMapping[m_currentTheme];
            themeMap[type] = candidates[0]; // Use first match
            return true;
        }
        
        // Try generic pattern (e.g., "wall.png")
        candidates = findSpritesByPattern(pattern, basePath);
        if (!candidates.empty()) {
            // Found generic sprite
            auto& themeMap = m_themeMapping[m_currentTheme];
            themeMap[type] = candidates[0]; // Use first match
            return true;
        }
    }
    
    return false;
}

std::vector<std::string> TileSpriteManager::findSpritesByPattern(const std::string& pattern, const std::string& basePath) const {
    std::vector<std::string> matches;
    
    // Common image extensions to check
    std::vector<std::string> extensions = {".png", ".jpg", ".jpeg", ".bmp", ".tga"};
    
    for (const std::string& ext : extensions) {
        std::string fullPath = basePath + pattern + ext;
        if (imageExists(fullPath)) {
            matches.push_back(fullPath);
        }
    }
    
    return matches;
}

std::vector<std::string> TileSpriteManager::getTileTypePatterns(TileType type) const {
    switch (type) {
        case TileType::Wall:
            return {"wall", "barrier", "block"};
        case TileType::Floor:
            return {"floor", "ground", "tile"};
        case TileType::Door:
            return {"door", "entrance", "gate"};
        case TileType::Water:
            return {"water", "sea", "ocean", "lake"};
        case TileType::Grass:
            return {"grass", "lawn", "field"};
        case TileType::Stone:
            return {"stone", "rock", "boulder"};
        case TileType::Tree:
            return {"tree", "forest", "wood"};
        case TileType::Building:
            return {"building", "structure", "house"};
        case TileType::Road:
            return {"road", "path", "street"};
        case TileType::House:
            return {"house", "home", "dwelling"};
        case TileType::Shop:
            return {"shop", "store", "market"};
        case TileType::Entrance:
            return {"entrance", "entry", "start"};
        case TileType::Exit:
            return {"exit", "end", "finish"};
        default:
            return {"tile"};
    }
}

std::string TileSpriteManager::getThemePrefixName(GenerationTheme theme) {
    switch (theme) {
        case GenerationTheme::Dungeon:
            return "dungeon";
        case GenerationTheme::City:
            return "city";
        case GenerationTheme::Terrain:
            return "terrain";
        case GenerationTheme::Fantasy:
            return "fantasy";
        case GenerationTheme::Modern:
            return "modern";
        default:
            return "tile";
    }
}

// ProceduralMap Implementation
ProceduralMap::ProceduralMap(int width, int height) : m_width(width), m_height(height) {
    m_tiles.resize(height);
    for (int y = 0; y < height; ++y) {
        m_tiles[y].resize(width);
        for (int x = 0; x < width; ++x) {
            m_tiles[y][x] = Tile(TileType::Empty, Vector2(x, y));
        }
    }
    
    // Create default sprite manager
    m_spriteManager = std::make_shared<TileSpriteManager>();
}

Tile& ProceduralMap::getTile(int x, int y) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        static Tile invalidTile(TileType::Wall);
        return invalidTile;
    }
    return m_tiles[y][x];
}

const Tile& ProceduralMap::getTile(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        static Tile invalidTile(TileType::Wall);
        return invalidTile;
    }
    return m_tiles[y][x];
}

void ProceduralMap::setTile(int x, int y, TileType type) {
    if (isValidPosition(x, y)) {
        m_tiles[y][x] = Tile(type, Vector2(x, y));
        if (m_spriteManager) {
            m_tiles[y][x].updateProperties(*m_spriteManager);
        }
    }
}

bool ProceduralMap::isValidPosition(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

void ProceduralMap::setSpriteManager(std::shared_ptr<TileSpriteManager> spriteManager) {
    m_spriteManager = spriteManager;
    updateAllTileSprites();
}

void ProceduralMap::updateAllTileSprites() {
    if (!m_spriteManager) {
        std::cout << "DEBUG: ProceduralMap::updateAllTileSprites - no sprite manager!" << std::endl;
        return;
    }
    
    std::cout << "DEBUG: ProceduralMap::updateAllTileSprites - updating " << (m_width * m_height) << " tiles" << std::endl;
    
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            m_tiles[y][x].updateProperties(*m_spriteManager);
        }
    }
    
    std::cout << "DEBUG: ProceduralMap::updateAllTileSprites - completed" << std::endl;
}

Vector2 ProceduralMap::getWorldPosition(int x, int y) const {
    // Return center position of tile to match TileRenderer rendering
    return Vector2(x * 32.0f + 16.0f, y * 32.0f + 16.0f); // 32 pixel tiles, centered
}

Vector2 ProceduralMap::getWorldPosition(int x, int y, int tileSize) const {
    // Return center position of tile to match TileRenderer rendering
    return Vector2(x * tileSize + tileSize/2.0f, y * tileSize + tileSize/2.0f);
}

Vector2 ProceduralMap::getGridPosition(const Vector2& worldPos) const {
    // Convert from center-based world position to grid coordinates
    return Vector2(static_cast<int>((worldPos.x - 16.0f) / 32.0f + 0.5f), 
                   static_cast<int>((worldPos.y - 16.0f) / 32.0f + 0.5f));
}

void ProceduralMap::clear(TileType fillType) {
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            setTile(x, y, fillType);
        }
    }
}

void ProceduralMap::generateToScene(Scene* scene, int tileSize) {
    if (!scene) return;
    
    // Get resource manager to load textures
    auto& engine = Engine::getInstance();
    auto resourceManager = engine.getResourceManager();
    if (!resourceManager) return;
    
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            const Tile& tile = getTile(x, y);
            if (tile.type != TileType::Empty && !tile.spriteName.empty()) {
                // Create entity for this tile
                EntityID entity = scene->createEntity();
                
                // Add transform
                Vector2 worldPos = getWorldPosition(x, y, tileSize);
                scene->addComponent<Transform>(entity, Transform(worldPos));
                
                // Add sprite with loaded texture
                Sprite sprite;
                auto texture = resourceManager->loadTexture(tile.spriteName);
                if (texture) {
                    sprite.texture = texture;
                    sprite.sourceRect = Rect(0, 0, texture->getWidth(), texture->getHeight());
                }
                scene->addComponent<Sprite>(entity, sprite);
                
                // Set entity name
                std::string tileName = "Tile_" + std::to_string(x) + "_" + std::to_string(y);
                scene->setEntityName(entity, tileName);
            }
        }
    }
}

// ProceduralGenerator Implementation
ProceduralGenerator::ProceduralGenerator(unsigned int seed) : m_seed(seed) {
    if (seed == 0) {
        m_seed = std::random_device{}();
    }
    m_rng.seed(m_seed);
}

void ProceduralGenerator::setSeed(unsigned int seed) {
    m_seed = seed;
    m_rng.seed(seed);
}

int ProceduralGenerator::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_rng);
}

float ProceduralGenerator::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_rng);
}

bool ProceduralGenerator::randomBool(float probability) {
    return randomFloat() < probability;
}

Vector2 ProceduralGenerator::randomPosition(int minX, int minY, int maxX, int maxY) {
    return Vector2(randomInt(minX, maxX), randomInt(minY, maxY));
}

// DungeonGenerator Implementation
DungeonGenerator::DungeonGenerator(const DungeonSettings& settings) 
    : ProceduralGenerator(), m_settings(settings) {
}

void DungeonGenerator::generate(ProceduralMap& map) {
    // Fill with walls initially
    map.clear(TileType::Wall);
    
    if (m_settings.useRoomApproach) {
        generateRooms(map);
    } else {
        generateCellularAutomata(map);
    }
    
    addDetails(map);
}

void DungeonGenerator::generateRooms(ProceduralMap& map) {
    std::vector<Room> rooms;
    
    // Generate rooms
    for (int i = 0; i < m_settings.numRooms; ++i) {
        for (int attempts = 0; attempts < 50; ++attempts) {
            Room room;
            room.width = randomInt(m_settings.minRoomSize, m_settings.maxRoomSize);
            room.height = randomInt(m_settings.minRoomSize, m_settings.maxRoomSize);
            room.x = randomInt(1, map.getWidth() - room.width - 1);
            room.y = randomInt(1, map.getHeight() - room.height - 1);
            
            if (isRoomValid(room, rooms, map.getWidth(), map.getHeight())) {
                rooms.push_back(room);
                
                // Carve out the room
                for (int y = room.y; y < room.y + room.height; ++y) {
                    for (int x = room.x; x < room.x + room.width; ++x) {
                        map.setTile(x, y, TileType::Floor);
                    }
                }
                break;
            }
        }
    }
    
    // Connect rooms
    connectRooms(map, rooms);
}

void DungeonGenerator::generateCellularAutomata(ProceduralMap& map) {
    // Initialize with random walls and floors
    for (int y = 1; y < map.getHeight() - 1; ++y) {
        for (int x = 1; x < map.getWidth() - 1; ++x) {
            if (randomFloat() < m_settings.wallDensity) {
                map.setTile(x, y, TileType::Wall);
            } else {
                map.setTile(x, y, TileType::Floor);
            }
        }
    }
    
    // Smooth the map
    for (int i = 0; i < m_settings.smoothingIterations; ++i) {
        smoothMap(map);
    }
}

void DungeonGenerator::smoothMap(ProceduralMap& map) {
    std::vector<std::vector<TileType>> newTiles(map.getHeight(), std::vector<TileType>(map.getWidth()));
    
    for (int y = 0; y < map.getHeight(); ++y) {
        for (int x = 0; x < map.getWidth(); ++x) {
            int wallCount = 0;
            
            // Count neighboring walls
            for (int ny = y - 1; ny <= y + 1; ++ny) {
                for (int nx = x - 1; nx <= x + 1; ++nx) {
                    if (nx == x && ny == y) continue;
                    if (!map.isValidPosition(nx, ny) || map.getTile(nx, ny).type == TileType::Wall) {
                        wallCount++;
                    }
                }
            }
            
            // Apply smoothing rule
            if (wallCount > 4) {
                newTiles[y][x] = TileType::Wall;
            } else {
                newTiles[y][x] = TileType::Floor;
            }
        }
    }
    
    // Apply changes
    for (int y = 0; y < map.getHeight(); ++y) {
        for (int x = 0; x < map.getWidth(); ++x) {
            map.setTile(x, y, newTiles[y][x]);
        }
    }
}

void DungeonGenerator::connectRooms(ProceduralMap& map, const std::vector<Room>& rooms) {
    for (size_t i = 1; i < rooms.size(); ++i) {
        Vector2 start = rooms[i-1].center();
        Vector2 end = rooms[i].center();
        createCorridor(map, start, end);
    }
}

void DungeonGenerator::createCorridor(ProceduralMap& map, Vector2 start, Vector2 end) {
    Vector2 current = start;
    
    // Horizontal first
    while (current.x != end.x) {
        for (int w = 0; w < m_settings.corridorWidth; ++w) {
            map.setTile(current.x, current.y + w, TileType::Floor);
        }
        current.x += (current.x < end.x) ? 1 : -1;
    }
    
    // Then vertical
    while (current.y != end.y) {
        for (int w = 0; w < m_settings.corridorWidth; ++w) {
            map.setTile(current.x + w, current.y, TileType::Floor);
        }
        current.y += (current.y < end.y) ? 1 : -1;
    }
}

void DungeonGenerator::addDetails(ProceduralMap& map) {
    // Add entrance and exit
    // Find floor tiles on edges for entrance/exit
    std::vector<Vector2> edgeFloors;
    
    for (int x = 0; x < map.getWidth(); ++x) {
        if (map.getTile(x, 1).type == TileType::Floor) {
            edgeFloors.push_back(Vector2(x, 0));
        }
        if (map.getTile(x, map.getHeight()-2).type == TileType::Floor) {
            edgeFloors.push_back(Vector2(x, map.getHeight()-1));
        }
    }
    
    if (!edgeFloors.empty()) {
        Vector2 entrance = edgeFloors[randomInt(0, edgeFloors.size()-1)];
        map.setTile(entrance.x, entrance.y, TileType::Entrance);
        
        if (edgeFloors.size() > 1) {
            Vector2 exit;
            do {
                exit = edgeFloors[randomInt(0, edgeFloors.size()-1)];
            } while (exit.x == entrance.x && exit.y == entrance.y);
            map.setTile(exit.x, exit.y, TileType::Exit);
        }
    }
}

bool DungeonGenerator::isRoomValid(const Room& room, const std::vector<Room>& existingRooms, int mapWidth, int mapHeight) {
    // Check bounds
    if (room.x + room.width >= mapWidth || room.y + room.height >= mapHeight) {
        return false;
    }
    
    // Check overlap with existing rooms
    for (const Room& existing : existingRooms) {
        if (room.x < existing.x + existing.width + 1 &&
            room.x + room.width + 1 > existing.x &&
            room.y < existing.y + existing.height + 1 &&
            room.y + room.height + 1 > existing.y) {
            return false;
        }
    }
    
    return true;
}

// CityGenerator Implementation
CityGenerator::CityGenerator(const CitySettings& settings) 
    : ProceduralGenerator(), m_settings(settings) {
}

void CityGenerator::generate(ProceduralMap& map) {
    map.clear(TileType::Grass);
    
    generateRoadNetwork(map);
    generateBuildings(map);
    
    if (m_settings.generateDistricts) {
        generateDistricts(map);
    }
    
    if (m_settings.addLandmarks) {
        addLandmarks(map);
    }
}

void CityGenerator::generateRoadNetwork(ProceduralMap& map) {
    // Generate main roads (grid pattern)
    int blockSizeWithRoad = m_settings.blockSize + m_settings.roadWidth;
    
    // Vertical roads
    for (int x = 0; x < map.getWidth(); x += blockSizeWithRoad) {
        for (int roadX = x; roadX < x + m_settings.roadWidth && roadX < map.getWidth(); ++roadX) {
            for (int y = 0; y < map.getHeight(); ++y) {
                map.setTile(roadX, y, TileType::Road);
            }
        }
    }
    
    // Horizontal roads
    for (int y = 0; y < map.getHeight(); y += blockSizeWithRoad) {
        for (int roadY = y; roadY < y + m_settings.roadWidth && roadY < map.getHeight(); ++roadY) {
            for (int x = 0; x < map.getWidth(); ++x) {
                map.setTile(x, roadY, TileType::Road);
            }
        }
    }
}

void CityGenerator::generateBuildings(ProceduralMap& map) {
    int blockSizeWithRoad = m_settings.blockSize + m_settings.roadWidth;
    
    for (int blockY = m_settings.roadWidth; blockY < map.getHeight(); blockY += blockSizeWithRoad) {
        for (int blockX = m_settings.roadWidth; blockX < map.getWidth(); blockX += blockSizeWithRoad) {
            if (randomFloat() < m_settings.buildingDensity) {
                placeBuildingInBlock(map, blockX, blockY);
            }
        }
    }
}

void CityGenerator::placeBuildingInBlock(ProceduralMap& map, int blockX, int blockY) {
    int buildingWidth = randomInt(m_settings.minBuildingSize, 
                                std::min(m_settings.maxBuildingSize, m_settings.blockSize));
    int buildingHeight = randomInt(m_settings.minBuildingSize, 
                                 std::min(m_settings.maxBuildingSize, m_settings.blockSize));
    
    int startX = blockX + randomInt(0, m_settings.blockSize - buildingWidth);
    int startY = blockY + randomInt(0, m_settings.blockSize - buildingHeight);
    
    TileType buildingType = randomBool(0.3f) ? TileType::Shop : TileType::House;
    
    for (int y = startY; y < startY + buildingHeight && y < map.getHeight(); ++y) {
        for (int x = startX; x < startX + buildingWidth && x < map.getWidth(); ++x) {
            map.setTile(x, y, buildingType);
        }
    }
}

void CityGenerator::generateDistricts(ProceduralMap& map) {
    // Different areas could have different building types
    // This is a placeholder for more complex district generation
}

void CityGenerator::addLandmarks(ProceduralMap& map) {
    // Add special large buildings or landmarks
    // This is a placeholder for landmark generation
}

// TerrainGenerator Implementation  
TerrainGenerator::TerrainGenerator(const TerrainSettings& settings)
    : ProceduralGenerator(), m_settings(settings) {
}

void TerrainGenerator::generate(ProceduralMap& map) {
    // Generate base terrain using noise
    for (int y = 0; y < map.getHeight(); ++y) {
        for (int x = 0; x < map.getWidth(); ++x) {
            float noiseValue = generateNoise(x, y);
            TileType terrainType = getTerrainType(noiseValue);
            map.setTile(x, y, terrainType);
        }
    }
    
    if (m_settings.addTrees) {
        addVegetation(map);
    }
}

float TerrainGenerator::generateNoise(float x, float y) {
    return perlinNoise(x * m_settings.scale, y * m_settings.scale);
}

float TerrainGenerator::perlinNoise(float x, float y) {
    // Simplified Perlin noise implementation
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < m_settings.octaves; ++i) {
        total += sin(x * frequency) * sin(y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= m_settings.persistence;
        frequency *= 2.0f;
    }
    
    return total / maxValue;
}

TileType TerrainGenerator::getTerrainType(float noiseValue) {
    // Normalize to 0-1 range
    noiseValue = (noiseValue + 1.0f) / 2.0f;
    
    if (noiseValue < m_settings.waterLevel) {
        return TileType::Water;
    } else if (noiseValue < m_settings.grassLevel) {
        return TileType::Grass;
    } else if (noiseValue < m_settings.stoneLevel) {
        return TileType::Stone;
    } else {
        return TileType::Stone;
    }
}

void TerrainGenerator::addVegetation(ProceduralMap& map) {
    for (int y = 0; y < map.getHeight(); ++y) {
        for (int x = 0; x < map.getWidth(); ++x) {
            if (map.getTile(x, y).type == TileType::Grass && randomFloat() < m_settings.treeDensity) {
                map.setTile(x, y, TileType::Tree);
            }
        }
    }
}

// ProceduralGenerationManager Implementation
ProceduralGenerationManager::ProceduralGenerationManager() {
    // Initialize with default generators
    m_dungeonGenerator = std::make_shared<DungeonGenerator>();
    m_cityGenerator = std::make_shared<CityGenerator>();
    m_terrainGenerator = std::make_shared<TerrainGenerator>();
    
    // Initialize sprite manager
    m_spriteManager = std::make_shared<TileSpriteManager>();
}

void ProceduralGenerationManager::setDungeonGenerator(std::shared_ptr<DungeonGenerator> generator) {
    m_dungeonGenerator = generator;
}

void ProceduralGenerationManager::setCityGenerator(std::shared_ptr<CityGenerator> generator) {
    m_cityGenerator = generator;
}

void ProceduralGenerationManager::setTerrainGenerator(std::shared_ptr<TerrainGenerator> generator) {
    m_terrainGenerator = generator;
}

void ProceduralGenerationManager::setSpriteManager(std::shared_ptr<TileSpriteManager> spriteManager) {
    m_spriteManager = spriteManager;
}

void ProceduralGenerationManager::setGenerationTheme(GenerationTheme theme) {
    if (m_spriteManager) {
        m_spriteManager->setTheme(theme);
    }
}

GenerationTheme ProceduralGenerationManager::getCurrentTheme() const {
    return m_spriteManager ? m_spriteManager->getCurrentTheme() : GenerationTheme::Default;
}

std::shared_ptr<ProceduralMap> ProceduralGenerationManager::generateDungeon(int width, int height, unsigned int seed) {
    auto map = std::make_shared<ProceduralMap>(width, height);
    
    // Set sprite manager and theme
    if (m_spriteManager) {
        m_spriteManager->setTheme(GenerationTheme::Dungeon);
        map->setSpriteManager(m_spriteManager);
    }
    
    // Generate dungeon
    if (m_dungeonGenerator) {
        m_dungeonGenerator->setSeed(seed);
        m_dungeonGenerator->generate(*map);
    }
    
    return map;
}

std::shared_ptr<ProceduralMap> ProceduralGenerationManager::generateCity(int width, int height, unsigned int seed) {
    auto map = std::make_shared<ProceduralMap>(width, height);
    
    // Set sprite manager and theme
    if (m_spriteManager) {
        m_spriteManager->setTheme(GenerationTheme::City);
        map->setSpriteManager(m_spriteManager);
    }
    
    // Generate city
    if (m_cityGenerator) {
        m_cityGenerator->setSeed(seed);
        m_cityGenerator->generate(*map);
    }
    return map;
}

std::shared_ptr<ProceduralMap> ProceduralGenerationManager::generateTerrain(int width, int height, unsigned int seed) {
    auto map = std::make_shared<ProceduralMap>(width, height);
    
    // Set sprite manager and theme
    if (m_spriteManager) {
        m_spriteManager->setTheme(GenerationTheme::Terrain);
        map->setSpriteManager(m_spriteManager);
    }
    
    // Generate terrain
    if (m_terrainGenerator) {
        m_terrainGenerator->setSeed(seed);
        m_terrainGenerator->generate(*map);
    }
    
    return map;
}

void ProceduralGenerationManager::generateMapToScene(std::shared_ptr<ProceduralMap> map, Scene* scene, int tileSize) {
    if (map && scene) {
        map->generateToScene(scene, tileSize);
    }
}
